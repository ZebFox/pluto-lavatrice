#include <stdio.h>
#include "view/view.h"
#include "gel/pagemanager/page_manager.h"
#include "model/model.h"
#include "peripherals/keyboard.h"
#include "lvgl/lvgl.h"
#include "view/fonts/legacy_fonts.h"
#include "view/common.h"
#include "view/intl/intl.h"


struct page_data {
    lv_obj_t  *lbl_celsius;
    lv_obj_t  *lbl_analog;
    lv_task_t *task;
};


static void update_page(model_t *pmodel, struct page_data *pdata) {
    lv_label_set_text_fmt(pdata->lbl_celsius, "Celsius : %4iC", pmodel->run.macchina.temperatura);
    lv_label_set_text_fmt(pdata->lbl_analog, "ADC     : %4i", pmodel->test.adc_temp);
}


static void *create_page(model_t *model, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->task             = view_register_periodic_task(500UL, LV_TASK_PRIO_OFF, 0);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;

    lv_task_set_prio(pdata->task, LV_TASK_PRIO_MID);

    view_common_title(lv_scr_act(), view_intl_get_string(pmodel, STRINGS_TEST_TEMPERATURE));

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 24);
    pdata->lbl_celsius = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 36);
    pdata->lbl_analog = lbl;

    update_page(pmodel, pdata);
}


static view_message_t process_page_event(model_t *model, void *args, pman_event_t event) {
    view_message_t    msg   = VIEW_EMPTY_MSG;
    struct page_data *pdata = args;

    switch (event.code) {
        case VIEW_EVENT_CODE_OPEN:
            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_TEST;
            msg.cmsg.test = 1;
            break;

        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                switch (event.key_event.code) {
                    case BUTTON_DESTRA: {
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE;
                        msg.vmsg.page = &page_test_digout;
                        break;
                    }
                    case BUTTON_SINISTRA: {
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE;
                        msg.vmsg.page = &page_test_level;
                        break;
                    }
                    case BUTTON_MENO: {
                        break;
                    }
                    case BUTTON_PIU: {
                        break;
                    }
                    case BUTTON_STOP: {
                        break;
                    }
                    case BUTTON_LINGUA: {
                        break;
                    }

                    default:
                        break;
                }
                break;
            }

            default:
                break;
        }

        case VIEW_EVENT_CODE_MODEL_UPDATE:
            break;

        case VIEW_EVENT_CODE_TIMER: {
            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_TEST_REFRESH;
            update_page(model, pdata);
            break;
        }
    }

    return msg;
}


static void view_close(void *args) {
    struct page_data *pdata = args;
    lv_obj_clean(lv_scr_act());
    lv_task_set_prio(pdata->task, LV_TASK_PRIO_OFF);
}

static void page_destroy(void *args, void *extra) {
    struct page_data *pdata = args;
    lv_task_del(pdata->task);
}


const pman_page_t page_test_temperature = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = view_close,
    .destroy       = page_destroy,
};