#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"

#include "keypad/keypad.h"
#include "model/model.h"
#include "view/view.h"
#include "controller/controller.h"
#include "controller/gui.h"
#include "peripherals/display/NT7534.h"
#include "peripherals/keyboard.h"
#include "peripherals/system.h"
#include "peripherals/heartbit.h"
#include "utils/utils.h"
#include "peripherals/buzzer.h"
#include "peripherals/hardwareprofile.h"
#include "peripherals/gettoniera.h"
#include "controller/modbus.h"
#include "view/view_types.h"


static const char *TAG = "Main";


void app_main(void) {
    model_t         model;
    keypad_update_t event;

    system_i2c_init();
    nt7534_init();
    keyboard_init();
    buzzer_init();
    gettoniera_init();
    modbus_init();
    // heartbit_init();

    model_init(&model);
    view_init(&model, nt7534_flush, nt7534_rounder, nt7534_set_px, keyboard_reset);
    controller_init(&model);

    modbus_read_input_register();
    ESP_LOGI(TAG, "Begin main loop");
    for (;;) {
        controller_gui_process(&model);
        event = keyboard_manage(get_millis());
        if (event.event != KEY_NOTHING) {
            view_event((view_event_t) {.code = VIEW_EVENT_KEYPAD, .key_event = event});
        }
        vTaskDelay(pdMS_TO_TICKS(4));
    }
}
