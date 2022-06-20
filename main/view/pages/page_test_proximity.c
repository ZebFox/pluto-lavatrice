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


#define DA2RPM(da) ((da * 1000) / 100)
#define OUT_AVANTI 14


struct page_data {
    lv_task_t *timer;
    lv_obj_t  *lbl_da;
    lv_obj_t  *lbl_min_max;
    lv_obj_t  *lbl_run;
    lv_obj_t  *lbl_level;

    lv_obj_t *led_oblo;
    lv_obj_t *led_inverter;
    lv_obj_t *led_sbilanciamento;
    lv_obj_t *led_emergenza;

    uint8_t da_value;
    uint8_t run;
};


static const char *TAG = "PageTestProximity";


static int test_cesto_in_sicurezza(model_t *model, struct page_data *data);


static void update_page(model_t *pmodel, struct page_data *pdata) {
    lv_label_set_text_fmt(pdata->lbl_da, "DA=[%04i] %4irpm", pdata->da_value, model_get_velocita_corretta(pmodel));
    lv_label_set_text_fmt(pdata->lbl_min_max, "m=%04i    M=%04i", pmodel->test.pmin, pmodel->test.pmax);
    lv_label_set_text_fmt(pdata->lbl_run, "[marcia] %s %s", pdata->run ? "on " : "off",
                          test_cesto_in_sicurezza(pmodel, pdata) ? "ok" : "no");

    lv_label_set_text(pdata->lbl_level, model_get_livello(pmodel) == 0 ? "ok" : "no");

    if ((pmodel->test.inputs & 1) > 0) {
        lv_led_off(pdata->led_emergenza);
    } else {
        lv_led_on(pdata->led_emergenza);
    }

    if (model_oblo_chiuso(pmodel)) {
        lv_led_off(pdata->led_oblo);
    } else {
        lv_led_on(pdata->led_oblo);
    }

    if ((pmodel->test.inputs & 2) > 0) {
        lv_led_off(pdata->led_sbilanciamento);
    } else {
        lv_led_on(pdata->led_sbilanciamento);
    }

    if ((pmodel->test.inputs & (1 << 10)) > 0) {
        lv_led_off(pdata->led_inverter);
    } else {
        lv_led_on(pdata->led_inverter);
    }
}


static void *create_page(model_t *pmodel, void *extra) {
    (void)TAG;
    (void)pmodel;
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->timer            = view_register_periodic_task(500UL, LV_TASK_PRIO_OFF, 0);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;

    pdata->da_value = 0;
    pdata->run      = 0;

    lv_task_set_prio(pdata->timer, LV_TASK_PRIO_MID);

    view_common_title(lv_scr_act(), "TEST PROXIMITY");

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 18);
    pdata->lbl_da = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 26);
    pdata->lbl_min_max = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 34);
    pdata->lbl_run = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 4, -1);
    lv_obj_t *lbl_liv = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl_liv, &style_label_6x8);
    lv_obj_set_auto_realign(lbl_liv, 1);
    lv_label_set_text(lbl_liv, "LIV");
    lv_obj_align(lbl_liv, lbl, LV_ALIGN_OUT_TOP_MID, -5, 0);
    pdata->lbl_level = lbl;

    lv_obj_t *led = lv_led_create(lv_scr_act(), NULL);
    lv_obj_set_size(led, 10, 6);
    lv_obj_align(led, NULL, LV_ALIGN_IN_BOTTOM_MID, -26, -2);
    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_label_set_text(lbl, "OBLO");
    lv_obj_align(lbl, led, LV_ALIGN_OUT_TOP_MID, 0, -1);
    pdata->led_oblo = led;

    led = lv_led_create(lv_scr_act(), NULL);
    lv_obj_set_size(led, 10, 6);
    lv_obj_align(led, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -2);
    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_label_set_text(lbl, "INV");
    lv_obj_align(lbl, led, LV_ALIGN_OUT_TOP_MID, 0, -1);
    pdata->led_inverter = led;

    led = lv_led_create(lv_scr_act(), NULL);
    lv_obj_set_size(led, 10, 6);
    lv_obj_align(led, NULL, LV_ALIGN_IN_BOTTOM_MID, 26, -2);
    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_label_set_text(lbl, "SBIL");
    lv_obj_align(lbl, led, LV_ALIGN_OUT_TOP_MID, 0, -1);
    pdata->led_sbilanciamento = led;

    led = lv_led_create(lv_scr_act(), NULL);
    lv_obj_set_size(led, 10, 6);
    lv_obj_align(led, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -4, -2);
    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_label_set_text(lbl, "EME");
    lv_obj_align(lbl, led, LV_ALIGN_OUT_TOP_MID, 0, -1);
    pdata->led_emergenza = led;

    update_page(pmodel, pdata);
}


static view_message_t process_page_event(model_t *pmodel, void *args, pman_event_t event) {
    view_message_t    msg   = VIEW_EMPTY_MSG;
    struct page_data *pdata = args;

    switch (event.code) {
        case VIEW_EVENT_CODE_MODEL_UPDATE:
            if (pdata->run && !test_cesto_in_sicurezza(pmodel, pdata)) {
                pdata->run    = 0;
                msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_DIGOUT_TURNOFF;
            } else if (model_alarm_code(pmodel) > 0 && pdata->run > 0) {
                pdata->run    = 0;
                msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_DIGOUT_TURNOFF;
            }
            update_page(pmodel, pdata);
            break;

        case VIEW_EVENT_CODE_OPEN:
            msg.cmsg.code  = VIEW_CONTROLLER_COMMAND_CODE_TEST_DAC_CONTROL;
            msg.cmsg.value = pdata->da_value;
            break;

        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                switch (event.key_event.code) {
                    case BUTTON_SINISTRA: {
                        msg.vmsg.code  = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE_EXTRA;
                        msg.vmsg.page  = &page_test_rotation;
                        msg.vmsg.extra = (void *)(uintptr_t)0;
                        msg.cmsg.code  = VIEW_CONTROLLER_COMMAND_CODE_DIGOUT_TURNOFF;
                        break;
                    }

                    case BUTTON_DESTRA: {
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE;
                        msg.vmsg.page = &page_test_oblo;
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_DIGOUT_TURNOFF;
                        break;
                    }

                    case BUTTON_MENO: {
                        if (pdata->da_value > 0) {
                            pdata->da_value--;
                        } else {
                            pdata->da_value = 255;
                        }
                        update_page(pmodel, pdata);
                        msg.cmsg.code  = VIEW_CONTROLLER_COMMAND_CODE_TEST_DAC_CONTROL;
                        msg.cmsg.value = pdata->da_value;
                        break;
                    }

                    case BUTTON_PIU: {
                        pdata->da_value = (pdata->da_value + 1) % 1024;
                        update_page(pmodel, pdata);
                        msg.cmsg.code  = VIEW_CONTROLLER_COMMAND_CODE_TEST_DAC_CONTROL;
                        msg.cmsg.value = pdata->da_value;
                        break;
                    }

                    case BUTTON_STOP:
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_DIGOUT_TURNOFF;
                        pdata->run    = 0;
                        break;

                    case BUTTON_LINGUA: {
                        if (pdata->da_value < 25) {
                            pdata->da_value = 25;
                        } else if (pdata->da_value < 50) {
                            pdata->da_value = 50;
                        } else if (pdata->da_value < 75) {
                            pdata->da_value = 75;
                        } else if (pdata->da_value < 100) {
                            pdata->da_value = 100;
                        } else {
                            pdata->da_value = 0;
                        }
                        update_page(pmodel, pdata);
                        msg.cmsg.code  = VIEW_CONTROLLER_COMMAND_CODE_TEST_DAC_CONTROL;
                        msg.cmsg.value = pdata->da_value;
                        break;
                    }

                    case BUTTON_START:
                        if (pdata->run == 0 && !test_cesto_in_sicurezza(pmodel, pdata)) {
                            break;
                        }
                        pdata->run      = !pdata->run;
                        msg.cmsg.code   = VIEW_CONTROLLER_COMMAND_CODE_TEST_DIGOUT;
                        msg.cmsg.output = OUT_AVANTI;
                        msg.cmsg.value  = pdata->run;
                        update_page(pmodel, pdata);
                        break;

                    default:
                        break;
                }
            } else if (event.key_event.event == KEY_LONGPRESS) {
                switch (event.key_event.code) {
                    case BUTTON_MENO: {
                        if (pdata->da_value > 0) {
                            pdata->da_value--;
                        } else {
                            pdata->da_value = 100;
                        }
                        update_page(pmodel, pdata);
                        break;
                    }

                    case BUTTON_PIU: {
                        pdata->da_value = (pdata->da_value + 1) % 101;
                        update_page(pmodel, pdata);
                        break;
                    }
                }
            } else if (event.key_event.event == KEY_RELEASE) {
                switch (event.key_event.code) {
                    case BUTTON_MENO: {
                        msg.cmsg.code  = VIEW_CONTROLLER_COMMAND_CODE_TEST_DAC_CONTROL;
                        msg.cmsg.value = pdata->da_value;
                        break;
                    }

                    case BUTTON_PIU: {
                        msg.cmsg.code  = VIEW_CONTROLLER_COMMAND_CODE_TEST_DAC_CONTROL;
                        msg.cmsg.value = pdata->da_value;
                        break;
                    }
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


static int test_cesto_in_sicurezza(model_t *model, struct page_data *data) {
    return model_oblo_chiuso(model) && model_get_livello_centimetri(model) == 0 && (model->test.inputs & 0x01) &&
           (model->test.inputs & 0x02) && (model->test.inputs & (1 << 10));
}


const pman_page_t page_test_proximity = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};