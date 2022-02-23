#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "lvgl/lvgl.h"
#include "src/lv_core/lv_obj.h"
#include "src/lv_misc/lv_task.h"
#include "src/lv_objx/lv_label.h"
#include "view/styles.h"
#include "view/view.h"
#include "view/images/legacy.h"
#include "view/view_types.h"
#include "view/widgets/custom_lv_img.h"
#include "gel/pagemanager/page_manager.h"
#include "peripherals/keyboard.h"
#include "view/common.h"
#include "view/intl/intl.h"
#include "utils/utils.h"
#include "esp_log.h"


enum {
    TIMEOUT_TIMER_ID,
};


struct page_data {
    int        flag;
    lv_task_t *task;
};


static void update_page(model_t *pmodel, struct page_data *pdata);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->task             = view_register_periodic_task(500UL, LV_TASK_PRIO_OFF, TIMEOUT_TIMER_ID);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *data = args;

    view_common_title(lv_scr_act(), view_intl_get_string(pmodel, STRINGS_PREZZO));

    lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(cont, 110, 20);
    lv_obj_align(cont, NULL, LV_ALIGN_IN_TOP_MID, 0, 1);
    lv_obj_set_style(cont, &style_container_bordered);

    update_page(pmodel, data);
}


static view_message_t process_page_event(model_t *pmodel, void *arg, view_event_t event) {
    view_message_t    msg  = VIEW_EMPTY_MSG;
    struct page_data *data = arg;
    (void)data;

    switch (event.code) {
        case VIEW_EVENT_CODE_PROGRAM_SAVED:
            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
            break;

        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                switch (event.key_event.code) {
                    case BUTTON_MENU:
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE;
                        msg.vmsg.page = (void *)&page_program_type;
                        break;

                    case BUTTON_STOP: {
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_UPDATE_PROGRAM;
                        break;
                    }

                    case BUTTON_DESTRA: {
                    }

                    case BUTTON_SINISTRA: {
                        break;
                    }

                    case BUTTON_MENO: {
                        break;
                    }

                    case BUTTON_PIU: {
                        break;
                    }

                    case BUTTON_LANA:
                        break;

                    case BUTTON_LINGUA: {
                        break;
                    }
                }
            }
            break;
        }

        case VIEW_EVENT_CODE_TIMER: {
            lv_task_reset(data->task);
            data->flag = !data->flag;
            update_page(pmodel, data);
        }

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


static void update_page(model_t *pmodel, struct page_data *pdata) {}


const pman_page_t page_program_price = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};