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
    lv_obj_t *lbl_color;
    lv_obj_t *lbl_mode;
    size_t    index;
    int       mode;

    lv_task_t *timer;
};


static const char *TAG = "PageTestLED";

static const char *colori[] = {"SPENTO", "ROSSO", "VERDE", "BLU", "GIALLO", "VIOLETTO", "TURCHESE", "BIANCO"};


static void update_page(model_t *pmodel, struct page_data *pdata) {
    lv_label_set_text_fmt(pdata->lbl_color, "Colore   : %8s", colori[pdata->index]);
    lv_label_set_text_fmt(pdata->lbl_mode, "Modalita': %4s", pdata->mode ? "AUTO" : "MAN");
}


static void *create_page(model_t *pmodel, void *extra) {
    (void)TAG;
    (void)pmodel;
    (void)extra;
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->timer            = view_register_periodic_task(500UL, LV_TASK_PRIO_OFF, 0);
    pdata->index            = 0;
    pdata->mode             = 0;
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;

    view_common_title(lv_scr_act(), "TEST LOGO LED");

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_CENTER, 0, 0);
    pdata->lbl_color = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_CENTER, 0, 12);
    pdata->lbl_mode = lbl;

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
            msg.cmsg.code  = VIEW_CONTROLLER_COMMAND_CODE_TEST_LED_CONTROl;
            msg.cmsg.value = pdata->index;
            break;

        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                switch (event.key_event.code) {
                    case BUTTON_SINISTRA: {
                        msg.vmsg.code  = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE_EXTRA;
                        msg.vmsg.page  = &page_test_coin;
                        msg.vmsg.extra = (void *)(uintptr_t)2;
                        break;
                    }

                    case BUTTON_DESTRA: {
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE;
                        msg.vmsg.page = &page_test_mems;
                        break;
                    }

                    case BUTTON_MENO: {
                        if (pdata->index > 0) {
                            pdata->index--;
                        } else {
                            pdata->index = 7;
                        }
                        msg.cmsg.code  = VIEW_CONTROLLER_COMMAND_CODE_TEST_LED_CONTROl;
                        msg.cmsg.value = pdata->index;
                        update_page(pmodel, pdata);
                        break;
                    }

                    case BUTTON_PIU: {
                        pdata->index   = (pdata->index + 1) % 8;
                        msg.cmsg.code  = VIEW_CONTROLLER_COMMAND_CODE_TEST_LED_CONTROl;
                        msg.cmsg.value = pdata->index;
                        update_page(pmodel, pdata);
                        break;
                    }

                    case BUTTON_STOP: {
                        break;
                    }

                    case BUTTON_START: {
                        pdata->mode = 1;
                        lv_task_set_prio(pdata->timer, LV_TASK_PRIO_MID);
                        msg.cmsg.code  = VIEW_CONTROLLER_COMMAND_CODE_TEST_LED_CONTROl;
                        msg.cmsg.value = pdata->index;
                        update_page(pmodel, pdata);
                        break;
                    }

                    case BUTTON_LINGUA: {
                        pdata->mode = 0;
                        lv_task_set_prio(pdata->timer, LV_TASK_PRIO_OFF);
                        update_page(pmodel, pdata);
                        break;
                    }

                    default:
                        break;
                }
            }
            break;
        }

        case VIEW_EVENT_CODE_TIMER:
            pdata->index   = (pdata->index + 1) % 8;
            msg.cmsg.code  = VIEW_CONTROLLER_COMMAND_CODE_TEST_LED_CONTROl;
            msg.cmsg.value = pdata->index;
            update_page(pmodel, pdata);
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



const pman_page_t page_test_led = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};