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


#define OUT_DEPURATA 1
#define OUT_SCARICO  2
#define OUT_CALDO    3
#define OUT_FREDDO   4


struct page_data {
    size_t index;

    lv_task_t *timer;

    uint16_t outputs;

    lv_obj_t *lbl_level;
    lv_obj_t *lbl_offset;
    lv_obj_t *lbl_oblo;
    lv_obj_t *lbls[4];
    lv_obj_t *lbl_scarico_ok;

    lv_obj_t *led_oblo;
    lv_obj_t *leds[4];

    int show_offset;
};


static int test_scarico_chiuso(model_t *model, struct page_data *data);


static const char    *TAG          = "PageTestHeating";
static const uint16_t transform[4] = {OUT_SCARICO, OUT_FREDDO, OUT_CALDO, OUT_DEPURATA};


static void update_page(model_t *pmodel, struct page_data *pdata) {
    for (size_t i = 0; i < 4; i++) {
        if (pdata->index == i) {
            lv_obj_set_style(pdata->lbls[i], &style_label_6x8_reverse);
        } else {
            lv_obj_set_style(pdata->lbls[i], &style_label_6x8);
        }

        if ((pdata->outputs & (1 << transform[i])) > 0) {
            lv_led_off(pdata->leds[i]);
        } else {
            lv_led_on(pdata->leds[i]);
        }
    }

    uint16_t adc = 0;
    if (pmodel->test.adc_press > pmodel->test.offset_press) {
        adc = pmodel->test.adc_press - pmodel->test.offset_press;
    }
    lv_label_set_text_fmt(pdata->lbl_level, "Level [%04i] %icm", adc, model_get_livello_centimetri(pmodel));

    if (pdata->show_offset == 4) {
        lv_obj_set_hidden(pdata->lbl_offset, 0);
        lv_label_set_text_fmt(pdata->lbl_offset, "Offset=%4i[%04i]", pmodel->test.offset_press, pmodel->test.adc_press);
    } else {
        lv_obj_set_hidden(pdata->lbl_offset, 1);
    }

    if (model_oblo_chiuso(pmodel)) {
        lv_label_set_text(pdata->lbl_oblo, "OBLO CLOSED");
        lv_led_off(pdata->led_oblo);
    } else {
        lv_label_set_text(pdata->lbl_oblo, "OBLO OPENED");
        lv_led_on(pdata->led_oblo);
    }

    if (!model_oblo_chiuso(pmodel) || !test_scarico_chiuso(pmodel, pdata)) {
        lv_label_set_text(pdata->lbl_scarico_ok, "no");
    } else {
        lv_label_set_text(pdata->lbl_scarico_ok, "ok");
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
    pdata->outputs          = 0;
    pdata->index            = 0;
    pdata->show_offset      = 0;

    lv_task_set_prio(pdata->timer, LV_TASK_PRIO_MID);

    view_common_title(lv_scr_act(), view_intl_get_string(pmodel, STRINGS_TEST_LIVELLO));

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 18);
    pdata->lbl_level = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 26);
    pdata->lbl_offset = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_LEFT_MID, 4, 8);
    pdata->lbl_oblo = lbl;
    lv_obj_t *led   = lv_led_create(lv_scr_act(), NULL);
    lv_obj_set_size(led, 10, 6);
    lv_obj_align(led, NULL, LV_ALIGN_IN_RIGHT_MID, -4, 8);
    pdata->led_oblo = led;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, led, LV_ALIGN_OUT_LEFT_MID, -2, 0);
    pdata->lbl_scarico_ok = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_body_draw(lbl, 1);
    lv_obj_set_auto_realign(lbl, 1);
    lv_label_set_text(lbl, "SCA");
    led = lv_led_create(lv_scr_act(), NULL);
    lv_obj_set_size(led, 10, 6);
    lv_obj_align(led, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 4, -2);
    lv_obj_align(lbl, led, LV_ALIGN_OUT_TOP_MID, 0, -1);
    pdata->lbls[0] = lbl;
    pdata->leds[0] = led;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_body_draw(lbl, 1);
    lv_obj_set_auto_realign(lbl, 1);
    lv_label_set_text(lbl, "H2O F");
    led = lv_led_create(lv_scr_act(), led);
    lv_obj_align(led, NULL, LV_ALIGN_IN_BOTTOM_MID, -20, -2);
    lv_obj_align(lbl, led, LV_ALIGN_OUT_TOP_MID, 0, -1);
    pdata->lbls[1] = lbl;
    pdata->leds[1] = led;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_body_draw(lbl, 1);
    lv_obj_set_auto_realign(lbl, 1);
    lv_label_set_text(lbl, "H2O C");
    led = lv_led_create(lv_scr_act(), led);
    lv_obj_align(led, NULL, LV_ALIGN_IN_BOTTOM_MID, 20, -2);
    lv_obj_align(lbl, led, LV_ALIGN_OUT_TOP_MID, 0, -1);
    pdata->lbls[2] = lbl;
    pdata->leds[2] = led;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_body_draw(lbl, 1);
    lv_obj_set_auto_realign(lbl, 1);
    lv_label_set_text(lbl, "DEP");
    led = lv_led_create(lv_scr_act(), led);
    lv_obj_align(led, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -4, -2);
    lv_obj_align(lbl, led, LV_ALIGN_OUT_TOP_MID, 0, -1);
    pdata->lbls[3] = lbl;
    pdata->leds[3] = led;

    update_page(pmodel, pdata);
}


static view_message_t process_page_event(model_t *pmodel, void *args, pman_event_t event) {
    view_message_t    msg   = VIEW_EMPTY_MSG;
    struct page_data *pdata = args;

    switch (event.code) {
        case VIEW_EVENT_CODE_MODEL_UPDATE:
            update_page(pmodel, pdata);
            if (model_alarm_code(pmodel) > 0 && pdata->outputs > 0) {
                msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_DIGOUT_TURNOFF;
            } else if (model_get_livello_centimetri(pmodel) > 20 && ((pdata->outputs & (1 << OUT_FREDDO)) > 0)) {
                msg.cmsg.code   = VIEW_CONTROLLER_COMMAND_CODE_TEST_DIGOUT_MULTI;
                msg.cmsg.output = OUT_FREDDO;
                msg.cmsg.value  = 0;
            } else if (model_get_livello_centimetri(pmodel) > 20 && ((pdata->outputs & (1 << OUT_CALDO)) > 0)) {
                msg.cmsg.code   = VIEW_CONTROLLER_COMMAND_CODE_TEST_DIGOUT_MULTI;
                msg.cmsg.output = OUT_CALDO;
                msg.cmsg.value  = 0;
            }
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
                        msg.vmsg.page = &page_test_heating;
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_DIGOUT_TURNOFF;
                        break;
                    }

                    case BUTTON_DESTRA: {
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE;
                        msg.vmsg.page = &page_test_liter_counter;
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_DIGOUT_TURNOFF;
                        break;
                    }

                    case BUTTON_PIU: {
                        pdata->index = (pdata->index + 1) % 4;
                        update_page(pmodel, pdata);
                        break;
                    }

                    case BUTTON_MENO: {
                        if (pdata->index > 0) {
                            pdata->index--;
                        } else {
                            pdata->index = 3;
                        }
                        update_page(pmodel, pdata);
                        break;
                    }

                    case BUTTON_STOP: {
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_DIGOUT_TURNOFF;
                        break;
                    }

                    case BUTTON_LINGUA: {
                        pdata->show_offset = (pdata->show_offset + 1) % 5;
                        update_page(pmodel, pdata);
                        break;
                    }

                    case BUTTON_MENU: {
                        if (pdata->show_offset == 4) {
                            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_OFFSET_PRESSIONE;
                            update_page(pmodel, pdata);
                        }
                        break;
                    }

                    case BUTTON_START: {
                        if ((!test_scarico_chiuso(pmodel, pdata) || !model_oblo_chiuso(pmodel)) && pdata->index != 0) {
                            break;
                        }

                        msg.cmsg.code   = VIEW_CONTROLLER_COMMAND_CODE_TEST_DIGOUT_MULTI;
                        msg.cmsg.output = transform[pdata->index];
                        msg.cmsg.value  = (pdata->outputs & (1 << transform[pdata->index])) > 0;
                        if (msg.cmsg.value) {
                            msg.cmsg.value = 0;
                            pdata->outputs &= ~(1 << transform[pdata->index]);
                        } else {
                            msg.cmsg.value = 1;
                            pdata->outputs |= (1 << transform[pdata->index]);
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
    free(extra);
}


static int test_scarico_chiuso(model_t *model, struct page_data *data) {
    return (data->outputs & (1 << OUT_SCARICO)) > 0;
}


const pman_page_t page_test_level = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};