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
#include "peripherals/machine_serial.h"
#include "peripherals/storage.h"
#include "peripherals/fs_storage.h"
#include "view/view_types.h"
#include "gel/timer/timecheck.h"

#include "freertos/queue.h"

static const char *TAG = "Main";
static model_t     model;

void invioprova();


void app_main(void) {
    keypad_update_t event;
    unsigned long   t = 0;

    vTaskDelay(pdMS_TO_TICKS(1000));

    system_i2c_init();
    nt7534_init();

    keyboard_init();
    buzzer_init();

    gettoniera_init();
    machine_serial_init();
    storage_init();
    fs_storage_mount_littlefs();
    // heartbit_init();

    model_init(&model);
    view_init(&model, nt7534_flush, nt7534_rounder, nt7534_set_px, keyboard_reset);
    controller_init(&model);


    ESP_LOGI(TAG, "Begin main loop");

    for (;;) {
        controller_gui_process(&model);
        controller_manage(&model);
        event = keyboard_manage(get_millis());
        if (event.event != KEY_NOTHING) {
            view_event((view_event_t){.code = VIEW_EVENT_CODE_KEYPAD, .key_event = event});
        }
        if (is_expired(t, get_millis(), 5000)) {
            // invioprova();
            t = get_millis();
        }

        vTaskDelay(pdMS_TO_TICKS(4));
    }
}
