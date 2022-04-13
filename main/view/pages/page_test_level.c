#include <stdio.h>
#include "view/view.h"
#include "gel/pagemanager/page_manager.h"
#include "model/model.h"
#include "peripherals/keyboard.h"
#include "lvgl/lvgl.h"
#include "view/fonts/legacy_fonts.h"
#include "view/common.h"
#include "view/styles.h"
#include "view/intl/intl.h"
#include "esp_log.h"


struct page_data {
    lv_obj_t *lbl_liters;
    lv_obj_t *lbl_centimeters;

    lv_task_t *timer;

    size_t index;
};


static const char *TAG = "PageTestLevel";


static void update_page(model_t *pmodel, struct page_data *pdata) {
    lv_label_set_text_fmt(pdata->lbl_centimeters, "%c%i cm (%i + %i)", pdata->index == 0 ? '>' : ' ',
                          pmodel->run.macchina.livello, pmodel->test.adc_press, pmodel->test.offset_press);
    lv_label_set_text_fmt(pdata->lbl_liters, "%c%i lt", pdata->index == 1 ? '>' : ' ',
                          pmodel->run.macchina.livello_litri);
}


static void *create_page(model_t *pmodel, void *extra) {
    (void)TAG;
    (void)pmodel;
    (void)extra;
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->index            = 0;
    pdata->timer            = view_register_periodic_task(500UL, LV_TASK_PRIO_OFF, 0);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;

    lv_task_set_prio(pdata->timer, LV_TASK_PRIO_MID);

    view_common_title(lv_scr_act(), view_intl_get_string(pmodel, STRINGS_TEST_LIVELLO));

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_body_draw(lbl, 1);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 20);
    pdata->lbl_centimeters = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_body_draw(lbl, 1);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 28);
    pdata->lbl_liters = lbl;

    update_page(pmodel, pdata);
}


static view_message_t process_page_event(model_t *pmodel, void *args, pman_event_t event) {
    view_message_t    msg   = VIEW_EMPTY_MSG;
    struct page_data *pdata = args;

    switch (event.code) {
        case VIEW_EVENT_CODE_MODEL_UPDATE:
            update_page(pmodel, pdata);
            break;

        case VIEW_EVENT_CODE_OPEN:
            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_TEST;
            msg.cmsg.test = 1;
            break;

        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                switch (event.key_event.code) {
                    case BUTTON_SINISTRA: {
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE;
                        msg.vmsg.page = &page_test_digout;
                        break;
                    }

                    case BUTTON_DESTRA: {
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE;
                        msg.vmsg.page = &page_test_temperature;
                        break;
                    }

                    case BUTTON_MENO: {
                        if (pdata->index > 0) {
                            pdata->index--;
                        } else {
                            pdata->index = 1;
                        }
                        update_page(pmodel, pdata);
                        break;
                    }

                    case BUTTON_PIU: {
                        pdata->index = (pdata->index + 1) % 2;
                        update_page(pmodel, pdata);
                        break;
                    }

                    case BUTTON_STOP: {
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_TEST;
                        msg.cmsg.test = 0;
                        break;
                    }

                    case BUTTON_LINGUA: {
                        if (pdata->index == 0) {
                            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_OFFSET_PRESSIONE;
                        } else {
                            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_AZZERA_LITRI;
                        }
                        break;
                    }

                    default:
                        break;
                }
            }
            break;
        }

        case VIEW_EVENT_CODE_TIMER:
            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_TEST_REFRESH;
            break;

        default:
            break;
    }

    return msg;
}


static void close_page(void *arg) {
    struct page_data *pdata = arg;
    lv_task_set_prio(pdata->timer, LV_TASK_PRIO_OFF);
    lv_obj_clean(lv_scr_act());
}


static void destroy_page(void *arg, void *extra) {
    struct page_data *pdata = arg;
    lv_task_del(pdata->timer);
    free(pdata);
    free(extra);
}


const pman_page_t page_test_level = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};