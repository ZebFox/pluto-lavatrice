#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"

#include "model/model.h"
#include "view/view.h"
#include "controller/controller.h"
#include "controller/gui.h"
#include "peripherals/display/NT7534.h"
#include "peripherals/keyboard.h"
#include "peripherals/system.h"
#include "peripherals/heartbit.h"


static const char *TAG = "Main";


void app_main(void) {
    model_t model;

    system_i2c_init();
    nt7534_init();
    //heartbit_init();

    model_init(&model);
    view_init(&model, nt7534_flush, nt7534_rounder, nt7534_set_px, keyboard_reset);
    controller_init(&model);

    ESP_LOGI(TAG, "Begin main loop");
    for (;;) {
        controller_gui_process(&model);
        vTaskDelay(pdMS_TO_TICKS(4));
    }
}
