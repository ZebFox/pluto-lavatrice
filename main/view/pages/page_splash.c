#include <stdlib.h>
#include <assert.h>
#include "lvgl/lvgl.h"
#include "view/view.h"
#include "view/images/legacy.h"
#include "view/view_types.h"
#include "view/widgets/custom_lv_img.h"
#include "gel/pagemanager/page_manager.h"
#include "peripherals/keyboard.h"


enum {
    TIMEOUT_TIMER_ID,
};


struct page_data {
    lv_task_t *task;
};


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->task             = view_register_periodic_task(4000UL, LV_TASK_PRIO_OFF, TIMEOUT_TIMER_ID);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    (void)pdata;
    lv_task_set_prio(pdata->task, LV_TASK_PRIO_MID);

    lv_obj_t *logo = custom_lv_img_create(lv_scr_act(), NULL);
    custom_lv_img_set_src(logo, &legacy_img_logo_ciao);
    lv_obj_align(logo, NULL, LV_ALIGN_CENTER, 0, 0);
}


static view_message_t process_page_event(model_t *pmodel, void *arg, view_event_t event) {
    view_message_t    msg  = VIEW_EMPTY_MSG;
    struct page_data *data = arg;
    (void)data;

    switch (event.code) {
        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                switch (event.key_event.code) {
                    case BUTTON_LANA: {
                        break;
                    }
                    default:
                        break;
                }
            }
        }
        break;

        case VIEW_EVENT_CODE_TIMER:
            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_DIGOUT_TURNOFF;
            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE;
            msg.vmsg.page = &page_main;
            break;

        default:
            break;
    }
    return msg;
}


static void close_page(void *arg) {
    struct page_data *data = arg;
    lv_task_set_prio(data->task, LV_TASK_PRIO_OFF);
    lv_obj_clean(lv_scr_act());
}


static void destroy_page(void *arg, void *extra) {
    struct page_data *data = arg;
    lv_task_del(data->task);
    free(data);
    free(extra);
}


const pman_page_t page_splash = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};