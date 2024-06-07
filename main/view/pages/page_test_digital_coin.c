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

    lv_obj_t *lbl_coins[5];
    lv_obj_t *lbl_total;
    lv_obj_t *led_enabled;
};


static unsigned int get_credito_gettoniera_digitale(model_t *model);


static const char *TAG = "PageTestDigitalCoin";


static void update_page(model_t *pmodel, struct page_data *pdata) {
    const size_t tf[]     = {5, 1, 2, 3, 4};
    const char  *values[] = {"2.0", "1.0", "0.5", "0.2", "0.1"};

    for (size_t i = 0; i < 5; i++) {
        lv_label_set_text_fmt(pdata->lbl_coins[i], "[%s] %i", values[i], pmodel->run.macchina.credito[tf[i]]);
    }

    lv_label_set_text_fmt(pdata->lbl_total, "Totale: %.2f", ((float)get_credito_gettoniera_digitale(pmodel)) / 100.);

    if (pmodel->run.digital_coin_reader_test_override == TEST_OVERRIDE_ON) {
        lv_led_off(pdata->led_enabled);
    } else {
        lv_led_on(pdata->led_enabled);
    }
}


static void *create_page(model_t *pmodel, void *extra) {
    (void)TAG;
    (void)pmodel;
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->timer            = view_register_periodic_task(200UL, LV_TASK_PRIO_OFF, 0);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;

    lv_task_set_prio(pdata->timer, LV_TASK_PRIO_MID);

    char string[32] = {0};
    snprintf(string, sizeof(string), "TEST GETT DIG");
    view_common_title(lv_scr_act(), string);

    for (size_t i = 0; i < 5; i++) {
        lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
        lv_obj_set_style(lbl, &style_label_6x8);
        lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 16 + i * 8);
        pdata->lbl_coins[i] = lbl;
    }

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_label_set_text(lbl, "Abil.:");
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_RIGHT, -20, 20);

    lv_obj_t *led = lv_led_create(lv_scr_act(), NULL);
    lv_obj_set_size(led, 10, 6);
    lv_obj_align(led, lbl, LV_ALIGN_OUT_RIGHT_MID, 2, 0);
    pdata->led_enabled = led;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 4, 0);
    pdata->lbl_total = lbl;

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
            pmodel->run.digital_coin_reader_test_override = TEST_OVERRIDE_OFF;
            msg.cmsg.code                                 = VIEW_CONTROLLER_COMMAND_CODE_TEST;
            msg.cmsg.test                                 = 1;
            update_page(pmodel, pdata);
            break;

        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                switch (event.key_event.code) {
                    case BUTTON_SINISTRA: {
                        pmodel->run.digital_coin_reader_test_override = TEST_OVERRIDE_NONE;
                        msg.vmsg.code                                 = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE_EXTRA;
                        msg.vmsg.page                                 = &page_test_coin;
                        msg.vmsg.extra                                = (void *)(uintptr_t)2;
                        msg.cmsg.code                                 = VIEW_CONTROLLER_COMMAND_CODE_TEST_CLEAR_CREDIT;
                        break;
                    }

                    case BUTTON_DESTRA: {
                        pmodel->run.digital_coin_reader_test_override = TEST_OVERRIDE_NONE;
                        msg.vmsg.code                                 = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE;
                        msg.vmsg.page                                 = &page_test_led;
                        msg.cmsg.code                                 = VIEW_CONTROLLER_COMMAND_CODE_TEST_CLEAR_CREDIT;
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

                    case BUTTON_LINGUA: {
                        if (pmodel->run.digital_coin_reader_test_override == TEST_OVERRIDE_ON) {
                            pmodel->run.digital_coin_reader_test_override = TEST_OVERRIDE_OFF;
                        } else {
                            pmodel->run.digital_coin_reader_test_override = TEST_OVERRIDE_ON;
                        }
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


static unsigned int get_credito_gettoniera_digitale(model_t *model) {
    assert(model != NULL);
    unsigned int       m      = 100;
    const unsigned int mult[] = {1 * m, (5 * m) / 10, (2 * m) / 10, (1 * m) / 10, 2 * m};
    unsigned int       tot    = 0;
    for (size_t i = LINEA_1_GETTONIERA_DIGITALE; i < LINEA_1_GETTONIERA_DIGITALE + LINEE_GETTONIERA_DIGITALE; i++) {
        size_t ti = i - LINEA_1_GETTONIERA_DIGITALE;
        tot += mult[ti] * model->run.macchina.credito[i];
    }
    return tot;
}


const pman_page_t page_test_digital_coin = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};
