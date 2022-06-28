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
    lv_task_t *timer;

    lv_obj_t *lbl_maniglia;
    lv_obj_t *lbl_oblo_chiuso;
    lv_obj_t *lbl_oblo_aperto;
    lv_obj_t *lbl_status;
};


static const char *TAG = "PageTestOblo";


static void update_page(model_t *pmodel, struct page_data *pdata) {
    lv_label_set_text_fmt(pdata->lbl_maniglia, "MICRO MANIGLIA = %s", model_oblo_chiuso(pmodel) ? " ON" : "OFF");
    lv_label_set_text_fmt(pdata->lbl_oblo_chiuso, "MICRO OBLO'CHI = %s",
                          pmodel->run.macchina.chiavistello_chiuso ? " ON" : "OFF");
    lv_label_set_text_fmt(pdata->lbl_oblo_aperto, "MICRO OBLO'APE = %s",
                          pmodel->run.macchina.chiavistello_aperto ? " ON" : "OFF");

    if (pmodel->run.macchina.chiavistello_aperto && !pmodel->run.macchina.chiavistello_chiuso) {
        lv_label_set_text(pdata->lbl_status, "serr. aperta");
    } else if (!pmodel->run.macchina.chiavistello_aperto && pmodel->run.macchina.chiavistello_chiuso) {
        lv_label_set_text(pdata->lbl_status, "serr. chiusa");
    } else {
        lv_label_set_text(pdata->lbl_status, "ERRORE");
    }
}


static void *create_page(model_t *pmodel, void *extra) {
    (void)TAG;
    (void)pmodel;
    (void)extra;
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->timer            = view_register_periodic_task(500UL, LV_TASK_PRIO_OFF, 0);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;

    lv_task_set_prio(pdata->timer, LV_TASK_PRIO_MID);

    view_common_title(lv_scr_act(), "TEST OBLO'");

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 18);
    pdata->lbl_maniglia = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 26);
    pdata->lbl_oblo_chiuso = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 34);
    pdata->lbl_oblo_aperto = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 42);
    pdata->lbl_status = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_label_set_text(lbl, "+ Chiudi     - Apri");
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 50);

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
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_APRI_OBLO;
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE;
                        msg.vmsg.page = &page_test_proximity;
                        break;
                    }

                    case BUTTON_DESTRA: {
                        msg.cmsg.code  = VIEW_CONTROLLER_COMMAND_CODE_APRI_OBLO;
                        msg.vmsg.code  = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE_EXTRA;
                        msg.vmsg.page  = &page_test_coin;
                        msg.vmsg.extra = (void *)(uintptr_t)0;
                        break;
                    }

                    case BUTTON_PIU: {
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_CHIUDI_OBLO;
                        break;
                    }

                    case BUTTON_MENO: {
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_APRI_OBLO;
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


const pman_page_t page_test_oblo = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};