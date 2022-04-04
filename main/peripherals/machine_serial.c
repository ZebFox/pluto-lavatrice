#include "driver/uart.h"
#include "hardwareprofile.h"


#define UART_PORTNUM   1
#define ECHO_READ_TOUT (3)     // 3.5T * 8 = 28 ticks, TOUT=3 -> ~24..33 ticks
#define TIMEOUT_MS 50


static const char *TAG = "Machine serial";


void machine_serial_init(void) {
    (void)TAG;
    uart_config_t uart_config = {
        .baud_rate           = 230400,
        .data_bits           = UART_DATA_8_BITS,
        .parity              = UART_PARITY_DISABLE,
        .stop_bits           = UART_STOP_BITS_1,
        .flow_ctrl           = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };

    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(UART_PORTNUM, &uart_config));

    uart_set_pin(UART_PORTNUM, HAL_MB_UART_TXD, HAL_MB_UART_RXD, -1, -1);
    ESP_ERROR_CHECK(uart_driver_install(UART_PORTNUM, 512, 512, 10, NULL, 0));
    ESP_ERROR_CHECK(uart_set_mode(UART_PORTNUM, UART_MODE_UART));
    ESP_ERROR_CHECK(uart_set_rx_timeout(UART_PORTNUM, ECHO_READ_TOUT));
}


void machine_serial_flush(void) {
    uart_flush(UART_PORTNUM);
}


int machine_serial_read(uint8_t *buffer, size_t len) {
    return uart_read_bytes(UART_PORTNUM, buffer, len, pdMS_TO_TICKS(TIMEOUT_MS));
}


int machine_serial_write(uint8_t *buffer, size_t len) {
    return uart_write_bytes(UART_PORTNUM, buffer, len);
}