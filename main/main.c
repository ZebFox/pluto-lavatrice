#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"

static const char *TAG = "Main";

void app_main() {

    ESP_LOGI(TAG, "Begin main loop");
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
