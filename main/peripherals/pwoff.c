#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "utils/utils.h"


// #include "i2c_devices/eeprom/24LC16/24LC16.h"
#include "gel/timer/timecheck.h"

#include "peripherals/hardwareprofile.h"
// #include "i2c_devices.h"

static const char *TAG = "PWoff";

static QueueHandle_t queue;

static void (*callback)(void *data) = NULL;
static void *callback_data          = NULL;


static void IRAM_ATTR pwoff_handler(void *arg) {
    (void)arg;

    int msg = 1;
    xQueueSendFromISR(queue, &msg, NULL);
}


void invioprova() {
    int msg = 1;
    xQueueSend(queue, &msg, portMAX_DELAY);
}


static void pwoff_task(void *arg) {
    (void)arg;

    unsigned long lastts = 0;

    for (;;) {
        int res;
        if (xQueueReceive(queue, &res, portMAX_DELAY) == pdTRUE) {
            if (is_expired(lastts, get_millis(), 1000UL)) {
                if (callback)
                    callback(callback_data);
                lastts = get_millis();
            }
            ESP_LOGI(TAG, "Power off fired!");
        }
    }

    vTaskDelete(NULL);
}


void power_off_init(void) {
    queue = xQueueCreate(1, sizeof(int));

    xTaskCreate(pwoff_task, "PWoff task", 1024, NULL, 10, NULL);

    ESP_LOGI(TAG, "Power off init!");

    gpio_config_t io_conf = {0};
    // interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    // bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = BIT64(HAL_PWOFF);
    // set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    gpio_config(&io_conf);

    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(HAL_PWOFF, pwoff_handler, NULL);
}


void power_off_register_callback(void (*cb)(void *), void *data) {
    callback      = cb;
    callback_data = data;

    ESP_LOGI(TAG, "Power off register cb!");

}