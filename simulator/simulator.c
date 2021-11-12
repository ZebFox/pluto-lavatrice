#include "FreeRTOS.h"
#include "task.h"
#include "esp_log.h"

#include "model/model.h"
#include "controller/controller.h"
#include "controller/gui.h"
#include "display/monitor.h"
#include "indev/mouse.h"
#include "peripherals/keyboard.h"
#include "view/view.h"


static const char *TAG = "Main";


void app_main(void *arg) {
    model_t model;
    (void)arg;

    monitor_init();
    keyboard_init();

    model_init(&model);
    view_init(&model, monitor_flush, NULL, NULL, keyboard_reset);
    controller_init(&model);

    ESP_LOGI(TAG, "Begin main loop");
    for (;;) {
        controller_gui_process(&model);
        vTaskDelay(pdMS_TO_TICKS(4));
    }

    vTaskDelete(NULL);
}