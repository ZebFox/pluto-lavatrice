#include "model/model.h"
#include "gel/timer/timecheck.h"
#include "view/view_types.h"
#include "lvgl/lvgl.h"
#include "utils/utils.h"
#include "view/view.h"
#include "model/model.h"


void controller_gui_process(model_t *pmodel) {
    static unsigned long last_invoked = 0;

    view_message_t umsg;
    view_event_t   event;

    lv_obj_invalidate(lv_scr_act());
    lv_task_handler();
    // lv_mem_monitor_t mem_monitor;
    // lv_mem_monitor(&mem_monitor);
    // model_mem_data(model, &mem_monitor);

    if (last_invoked != get_millis()) {
        lv_tick_inc(time_interval(last_invoked, get_millis()));
        last_invoked = get_millis();
    }

    while (view_get_next_msg(pmodel, &umsg, &event)) {
        // controller_process_msg(&umsg.cmsg, model);
        view_process_msg(umsg.vmsg, pmodel);

        if (event.code == VIEW_EVENT_KEYPAD && event.key_event.event == KEY_CLICK) {
            // digout_buzzer_bip(1, 100, 0);
        }
    }
}