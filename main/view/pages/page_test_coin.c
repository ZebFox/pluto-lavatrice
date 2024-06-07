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

    lv_obj_t *lbl_gettoni;
    lv_obj_t *lbl_min_max;
    lv_obj_t *lbl_type_contact;

    lv_obj_t *led_segnale;

    size_t line;
};


static const char *TAG       = "PageTestCoin";
static uint16_t    inputs[3] = {1 << 2, 1 << 3, 1 << 12};


static void update_page(model_t *pmodel, struct page_data *pdata) {
    char *type_string;
    switch (pmodel->prog.parmac.tipo_gettoniera) {
        case PAGAMENTO_CASSA_NA:
        case PAGAMENTO_1_LINEA_NA:
        case PAGAMENTO_2_LINEA_NA:
            type_string = "NA";
            break;

        case PAGAMENTO_CASSA_NC:
        case PAGAMENTO_1_LINEA_NC:
        case PAGAMENTO_2_LINEA_NC:
            type_string = "NC";
            break;

        default:
            type_string = "--";
            break;
    }

    lv_label_set_text_fmt(pdata->lbl_gettoni, "Gett: %i", pmodel->run.macchina.credito[6 + pdata->line]);
    lv_label_set_text_fmt(pdata->lbl_min_max, "min=%04i  max=%04i", pmodel->test.minp[pdata->line],
                          pmodel->test.maxp[pdata->line]);
    lv_label_set_text_fmt(pdata->lbl_type_contact, "tipo=%s segnale=", type_string);
    if ((pmodel->test.inputs & inputs[pdata->line]) > 0) {
        lv_led_off(pdata->led_segnale);
    } else {
        lv_led_on(pdata->led_segnale);
    }
    lv_obj_align(pdata->led_segnale, pdata->lbl_type_contact, LV_ALIGN_OUT_RIGHT_MID, 2, 0);
}


static void *create_page(model_t *pmodel, void *extra) {
    (void)TAG;
    (void)pmodel;
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->timer            = view_register_periodic_task(200UL, LV_TASK_PRIO_OFF, 0);
    pdata->line             = (size_t)(uintptr_t)extra;
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;

    lv_task_set_prio(pdata->timer, LV_TASK_PRIO_MID);

    char string[32] = {0};
    snprintf(string, sizeof(string), "TEST GETT %zu", pdata->line + 1);
    view_common_title(lv_scr_act(), string);

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 18);
    pdata->lbl_gettoni = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 34);
    pdata->lbl_min_max = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 50);
    pdata->lbl_type_contact = lbl;

    lv_obj_t *led = lv_led_create(lv_scr_act(), NULL);
    lv_obj_set_size(led, 10, 6);
    pdata->led_segnale = led;

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
                        if (pdata->line == 0) {
                            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE;
                            msg.vmsg.page = &page_test_oblo;
                        } else {
                            msg.vmsg.code  = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE_EXTRA;
                            msg.vmsg.page  = &page_test_coin;
                            msg.vmsg.extra = (void *)(uintptr_t)pdata->line - 1;
                        }
                        break;
                    }

                    case BUTTON_DESTRA: {
                        if (pdata->line == 2) {
                            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE;
                            msg.vmsg.page = &page_test_digital_coin;
                        } else {
                            msg.vmsg.code  = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE_EXTRA;
                            msg.vmsg.page  = &page_test_coin;
                            msg.vmsg.extra = (void *)(uintptr_t)pdata->line + 1;
                        }
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

                    case BUTTON_MENU: {
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_TEST_CLEAR_CREDIT;
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
}


const pman_page_t page_test_coin = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};
