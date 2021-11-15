#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "lightmodbus/master.h"
#include "peripherals/hardwareprofile.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "config/app_config.h"
#include "driver/gpio.h"
#include "config/app_config.h"
#include <sys/types.h>
#include "lightmodbus/lightmodbus.h"
#include "lightmodbus/master_func.h"
#include "gel/timer/timecheck.h"
#include "utils/utils.h"
#include "modbus.h"


#define MB_PORTNUM 1
// Timeout threshold for UART = number of symbols (~10 tics) with unchanged state on receive pin
#define ECHO_READ_TOUT (3)     // 3.5T * 8 = 28 ticks, TOUT=3 -> ~24..33 ticks

#define MODBUS_RESPONSE_03_LEN(data_len) (5 + data_len * 2)

#define BASE_TASK_SIZE       1024
#define SLAVE_DEVICE_ADDRESS 2

#define MODBUS_MESSAGE_QUEUE_SIZE     512
#define MODBUS_QUERY_INTERVAL         500
#define MODBUS_TIMEOUT                50
#define MODBUS_MAX_PACKET_SIZE        256
#define MODBUS_COMMUNICATION_ATTEMPTS 3

#define MODBUS_HOLDING_REGISTER_ADDRESS 0

#define MODBUS_AUTO_COMMISSIONING_DONE_BIT 0x01

typedef enum {
    TASK_MESSAGE_CODE_READ_INPUT_REGISTER,
    TASK_MESSAGE_CODE_SET_DEVICE_OUTPUT,
} task_message_code_t;


struct __attribute__((packed)) task_message {
    task_message_code_t code;
    uint8_t             address;
    union {
        struct {
            int value;
            int index;
        };
    };
};

static LIGHTMODBUS_RET_ERROR build_custom_request(ModbusMaster *status, uint8_t function, uint8_t *data, size_t len);
static void                  modbus_task(void *args);
static int write_holding_register(ModbusMaster *master, uint8_t address, uint16_t index, uint16_t data);
static int write_coil(ModbusMaster *master, uint8_t address, uint16_t index, int value);
static int read_input_register(ModbusMaster *master, uint8_t address, uint16_t register);

static const char *  TAG       = "Modbus";
static QueueHandle_t messageq  = NULL;
static QueueHandle_t responseq = NULL;
static TaskHandle_t  task      = NULL;

static ModbusMasterFunctionHandler custom_functions[] = {
#if defined(LIGHTMODBUS_F01M) || defined(LIGHTMODBUS_MASTER_FULL)
    {1, modbusParseResponse01020304},
#endif

#if defined(LIGHTMODBUS_F02M) || defined(LIGHTMODBUS_MASTER_FULL)
    {2, modbusParseResponse01020304},
#endif

#if defined(LIGHTMODBUS_F03M) || defined(LIGHTMODBUS_MASTER_FULL)
    {3, modbusParseResponse01020304},
#endif

#if defined(LIGHTMODBUS_F04M) || defined(LIGHTMODBUS_MASTER_FULL)
    {4, modbusParseResponse01020304},
#endif

#if defined(LIGHTMODBUS_F05M) || defined(LIGHTMODBUS_MASTER_FULL)
    {5, modbusParseResponse0506},
#endif

#if defined(LIGHTMODBUS_F06M) || defined(LIGHTMODBUS_MASTER_FULL)
    {6, modbusParseResponse0506},
#endif

#if defined(LIGHTMODBUS_F15M) || defined(LIGHTMODBUS_MASTER_FULL)
    {15, modbusParseResponse1516},
#endif

#if defined(LIGHTMODBUS_F16M) || defined(LIGHTMODBUS_MASTER_FULL)
    {16, modbusParseResponse1516},
#endif

#if defined(LIGHTMODBUS_F22M) || defined(LIGHTMODBUS_MASTER_FULL)
    {22, modbusParseResponse22},
#endif
    // Guard - prevents 0 size array
    {0, NULL},
};


void modbus_init(void) {
    uart_config_t uart_config = {
        .baud_rate           = 115200,
        .data_bits           = UART_DATA_8_BITS,
        .parity              = UART_PARITY_DISABLE,
        .stop_bits           = UART_STOP_BITS_1,
        .flow_ctrl           = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };

    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(MB_PORTNUM, &uart_config));

    uart_set_pin(MB_PORTNUM, HAL_MB_UART_TXD, HAL_MB_UART_RXD, -1, -1);
    ESP_ERROR_CHECK(uart_driver_install(MB_PORTNUM, 512, 512, 10, NULL, 0));
    ESP_ERROR_CHECK(uart_set_mode(MB_PORTNUM, UART_MODE_UART));
    ESP_ERROR_CHECK(uart_set_rx_timeout(MB_PORTNUM, ECHO_READ_TOUT));

    static StaticQueue_t static_queue1;
    static uint8_t       queue_buffer1[MODBUS_MESSAGE_QUEUE_SIZE * sizeof(struct task_message)] = {0};
    messageq =
        xQueueCreateStatic(MODBUS_MESSAGE_QUEUE_SIZE, sizeof(struct task_message), queue_buffer1, &static_queue1);

    static StaticQueue_t static_queue2;
    static uint8_t       queue_buffer2[MODBUS_MESSAGE_QUEUE_SIZE * sizeof(modbus_response_t)] = {0};
    responseq = xQueueCreateStatic(MODBUS_MESSAGE_QUEUE_SIZE, sizeof(modbus_response_t), queue_buffer2, &static_queue2);

    static uint8_t      task_stack[BASE_TASK_SIZE * 4] = {0};
    static StaticTask_t static_task;
    task = xTaskCreateStatic(modbus_task, TAG, sizeof(task_stack), NULL, 5, task_stack, &static_task);
}

void modbus_read_input_register(void) {
    struct task_message message = {.code = TASK_MESSAGE_CODE_READ_INPUT_REGISTER, .address = SLAVE_DEVICE_ADDRESS};
    xQueueSend(messageq, &message, portMAX_DELAY);
}

void modbus_set_device_output(uint8_t index, uint8_t value) {
    struct task_message message = {
        .code = TASK_MESSAGE_CODE_SET_DEVICE_OUTPUT, .index = index, .address = SLAVE_DEVICE_ADDRESS, .value = value};
    xQueueSend(messageq, &message, portMAX_DELAY);
}

void modbus_stop_current_operation(void) {
    xTaskNotifyGive(task);
}

static ModbusError dataCallback(const ModbusMaster *master, const ModbusDataCallbackArgs *args) {
    modbus_response_t *response = modbusMasterGetUserPointer(master);
    printf("Received data from %d, reg: %d, value: %d\n", args->address, args->index, args->value);
    if (response != NULL) {
        switch (response->code) {
            case MODBUS_RESPONSE_ERROR: {
            } break;
            default:
                break;
        }
    }
    return MODBUS_OK;
}

static ModbusError masterExceptionCallback(const ModbusMaster *master, uint8_t address, uint8_t function,
                                           ModbusExceptionCode code) {
    printf("Received exception (function %d) from slave %d code %d\n", function, address, code);
    return MODBUS_OK;
}


static void modbus_task(void *args) {
    (void)args;
    ModbusMaster    master;
    ModbusErrorInfo err = modbusMasterInit(&master,
                                           dataCallback,                // Callback for handling incoming data
                                           masterExceptionCallback,     // Exception callback (optional)
                                           modbusDefaultAllocator,      // Memory allocator used to allocate request
                                           custom_functions,            // Set of supported functions
                                           modbusMasterDefaultFunctionCount     // Number of supported functions
    );

    // Check for errors
    assert(modbusIsOk(err) && "modbusMasterInit() failed");
    struct task_message message    = {0};
    modbus_response_t   error_resp = {.code = MODBUS_RESPONSE_ERROR};

    ESP_LOGI(TAG, "Task starting");
    for (;;) {
        xTaskNotifyStateClear(task);
        uart_flush_input(MB_PORTNUM);

        if (xQueueReceive(messageq, &message, pdMS_TO_TICKS(100))) {
            error_resp.address = message.address;
            modbusMasterSetUserPointer(&master, NULL);

            switch (message.code) {
                case TASK_MESSAGE_CODE_READ_INPUT_REGISTER: {
                    read_input_register(&master, message.address, 0);
                    break;
                }
                case TASK_MESSAGE_CODE_SET_DEVICE_OUTPUT: {
                    write_coil(&master, message.address, message.index, message.value);
                    break;
                }
                default:
                    break;
            }
            vTaskDelay(pdMS_TO_TICKS(MODBUS_TIMEOUT));
        }
    }
    vTaskDelete(NULL);
}

static int read_input_register(ModbusMaster *master, uint8_t address, uint16_t reg) {
    ModbusErrorInfo err;
    uint8_t         buffer[MODBUS_RESPONSE_03_LEN(2)] = {0};
    int             res                               = 0;
    size_t          counter                           = 0;

    do {
        res = 0;
        err = modbusBuildRequest04RTU(master, address, reg, 1);
        assert(modbusIsOk(err));
        uart_write_bytes(MB_PORTNUM, modbusMasterGetRequest(master), modbusMasterGetRequestLength(master));

        int len = uart_read_bytes(MB_PORTNUM, buffer, sizeof(buffer), pdMS_TO_TICKS(MODBUS_TIMEOUT));
        err     = modbusParseResponseRTU(master, modbusMasterGetRequest(master), modbusMasterGetRequestLength(master),
                                     buffer, len);

        if (!modbusIsOk(err)) {
            res = 1;
        }
    } while (res && counter++ < MODBUS_COMMUNICATION_ATTEMPTS);

    return res;
}


static LIGHTMODBUS_RET_ERROR build_custom_request(ModbusMaster *status, uint8_t function, uint8_t *data, size_t len) {
    if (modbusMasterAllocateRequest(status, len + 1)) {
        return MODBUS_GENERAL_ERROR(ALLOC);
    }

    status->request.pdu[0] = function;
    if (data != NULL) {
        for (size_t i = 0; i < len; i++) {
            status->request.pdu[1 + i] = data[i];
        }
    }

    return MODBUS_NO_ERROR();
}

static int write_holding_registers(ModbusMaster *master, uint8_t address, uint16_t starting_address, uint16_t *data,
                                   size_t num) {
    uint8_t buffer[MODBUS_MAX_PACKET_SIZE] = {0};
    int     res                            = 0;
    size_t  counter                        = 0;

    do {
        res                 = 0;
        ModbusErrorInfo err = modbusBuildRequest16RTU(master, address, starting_address, num, data);
        assert(modbusIsOk(err));
        uart_write_bytes(MB_PORTNUM, modbusMasterGetRequest(master), modbusMasterGetRequestLength(master));

        int len = uart_read_bytes(MB_PORTNUM, buffer, sizeof(buffer), pdMS_TO_TICKS(MODBUS_TIMEOUT));
        err     = modbusParseResponseRTU(master, modbusMasterGetRequest(master), modbusMasterGetRequestLength(master),
                                     buffer, len);

        if (!modbusIsOk(err)) {
            ESP_LOGW(TAG, "Write holding registers for %i error: %i %i", address, err.source, err.error);
            res = 1;
            vTaskDelay(pdMS_TO_TICKS(MODBUS_TIMEOUT));
        }
    } while (res && counter++ < MODBUS_COMMUNICATION_ATTEMPTS);

    return res;
}

static int write_coil(ModbusMaster *master, uint8_t address, uint16_t index, int value) {
    uint8_t buffer[MODBUS_MAX_PACKET_SIZE] = {0};
    int     res                            = 0;
    size_t  counter                        = 0;

    do {
        res = 0;
        ModbusErrorInfo err = modbusBuildRequest05RTU(master, address, index, value);
        assert(modbusIsOk(err));
        uart_write_bytes(MB_PORTNUM, modbusMasterGetRequest(master), modbusMasterGetRequestLength(master));

        int len = uart_read_bytes(MB_PORTNUM, buffer, sizeof(buffer), pdMS_TO_TICKS(MODBUS_TIMEOUT));
        err     = modbusParseResponseRTU(master, modbusMasterGetRequest(master), modbusMasterGetRequestLength(master),
                                     buffer, len);

            ESP_LOG_BUFFER_HEX(TAG, buffer, len);
//02 05 00 01 ff 00 dd c9
//02 05 00 01 00 00 9c 39
        if (!modbusIsOk(err)) {
            ESP_LOGW(TAG, "Write coil for %i error: %i %i", address, err.source, err.error);
            res = 1;
            vTaskDelay(pdMS_TO_TICKS(MODBUS_TIMEOUT));
        } else {
            break;
        }
    } while (res && counter++ < MODBUS_COMMUNICATION_ATTEMPTS);

    return res;
}