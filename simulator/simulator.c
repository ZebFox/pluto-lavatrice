#include "FreeRTOS.h"
#include "task.h"
#include "esp_log.h"

#include "model/model.h"
#include "controller/controller.h"


static const char *TAG = "Main";


void app_main(void *arg) {
    model_t model;
    (void)arg;

    model_init(&model);
    // view_init(&model);
    controller_init(&model);

    ESP_LOGI(TAG, "Begin main loop");
    for (;;) {
        ESP_LOGI(TAG, "Hello simulated world!");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vTaskDelete(NULL);
}