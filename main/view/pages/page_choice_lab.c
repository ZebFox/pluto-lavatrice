#include <stdio.h>
#include "view/images/legacy.h"
#include "view/view.h"
#include "gel/pagemanager/page_manager.h"
#include "model/model.h"
#include "lvgl/lvgl.h"
#include "view/fonts/legacy_fonts.h"
#include "view/common.h"
#include "view/styles.h"
#include "view/widgets/custom_lv_img.h"
#include "view/intl/intl.h"
#include "peripherals/keyboard.h"
#include "esp_log.h"


static const char *TAG = "PageChoiceLab";


struct page_data {
    lv_task_t *timer;

    lv_obj_t *lbl_status;

    uint16_t allarme;
    uint16_t number;
};


static void update_page(model_t *pmodel, struct page_data *pdata) {
    if (model_alarm_code(pmodel) > 0) {
        if (pdata->allarme != model_alarm_code(pmodel)) {
            pdata->allarme = model_alarm_code(pmodel);
            lv_label_set_text(pdata->lbl_status, view_common_alarm_description(pmodel));
        }
    } else {
        lv_label_set_text(pdata->lbl_status,
                          view_intl_get_string_from_language(model_get_temporary_language(pmodel), STRINGS_INIZIO));
    }

    const programma_preview_t *preview    = model_get_preview(pmodel, pdata->number);
    char                       string[32] = {0};
    model_formatta_prezzo(string, pmodel, preview->prezzo);
}


static void *create_page(model_t *model, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->timer            = view_register_periodic_task(5000UL, LV_TASK_PRIO_OFF, 0);
    pdata->number           = (uint16_t)(uint32_t)extra;
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    pdata->allarme          = 0;
    lv_task_set_prio(pdata->timer, LV_TASK_PRIO_MID);

    const programma_preview_t *preview = model_get_preview(pmodel, pdata->number);

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_label_set_text_fmt(lbl, "%02i", pdata->number + 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 1, 1);

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_SROLL_CIRC);
    lv_obj_set_width(lbl, 112);
    lv_label_set_text(lbl, preview->name);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 17, 1);

    lv_obj_t *img = custom_lv_img_create(lv_scr_act(), NULL);
    custom_lv_img_set_src(img, &legacy_img_time);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 12);
    lv_obj_t *img_duration = img;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_8x16);
    lv_label_set_text_fmt(lbl, "%02im%02is", preview->durata / 60, preview->durata % 60);
    lv_obj_align(lbl, img_duration, LV_ALIGN_OUT_RIGHT_MID, 2, 0);

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_8x16);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_RIGHT);
    lv_label_set_text_fmt(lbl, "%02iC", preview->temperatura);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 12);
    lv_obj_t *lbl_temperature = lbl;

    img = custom_lv_img_create(lv_scr_act(), NULL);
    custom_lv_img_set_src(img, &legacy_img_temperature);
    lv_obj_align(img, lbl_temperature, LV_ALIGN_OUT_LEFT_MID, -2, 0);

    img = custom_lv_img_create(lv_scr_act(), NULL);
    custom_lv_img_set_src(img, &legacy_img_level);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 34);
    lv_obj_t *img_level = img;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_8x16);
    lv_label_set_text_fmt(lbl, "%02i%s", preview->livello, model_is_level_in_cm(&pmodel->prog.parmac) ? "cm" : "lt");
    lv_obj_align(lbl, img_level, LV_ALIGN_OUT_RIGHT_MID, 2, 0);

    img = custom_lv_img_create(lv_scr_act(), NULL);
    custom_lv_img_set_src(img, &legacy_img_rpm);
    lv_obj_set_auto_realign(img, 1);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 34);

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_8x16);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_RIGHT);
    lv_label_set_text_fmt(lbl, "%i", preview->velocita);
    lv_obj_align(lbl, img, LV_ALIGN_OUT_LEFT_TOP, 0, 0);
    lv_obj_t *lbl_speed = lbl;

    img = custom_lv_img_create(lv_scr_act(), NULL);
    custom_lv_img_set_src(img, &legacy_img_speed);
    lv_obj_align(img, lbl_speed, LV_ALIGN_OUT_LEFT_MID, -2, 0);

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_SROLL_CIRC);
    lv_obj_set_width(lbl, 128);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    pdata->lbl_status = lbl;

    lv_obj_t *line = view_common_horizontal_line();
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);

    static lv_point_t points[2] = {{0, 0}, {0, 10}};
    line                        = view_common_line(points, 2);
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_LEFT, 14, 0);

    line = view_common_horizontal_line();
    lv_obj_align(line, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -8);

    update_page(pmodel, pdata);
}


static view_message_t process_page_event(model_t *pmodel, void *arg, pman_event_t event) {
    view_message_t    msg   = VIEW_EMPTY_MSG;
    struct page_data *pdata = arg;

    switch (event.code) {
        case VIEW_EVENT_CODE_TIMER:
            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
            break;

        case VIEW_EVENT_CODE_MODEL_UPDATE:
            if (!model_macchina_in_stop(pmodel) && model_can_work(pmodel)) {
                msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                msg.vmsg.page = (void *)view_work_page(pmodel);
            } else if (model_alarm_code(pmodel) > 0) {
                msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
            } else {
                update_page(pmodel, pdata);
            }
            break;

        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                lv_task_reset(pdata->timer);

                switch (event.key_event.code) {
                    case BUTTON_STOP:
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
                        break;

                    case BUTTON_SINISTRA:
                        break;

                    case BUTTON_DESTRA:
                        break;

                    case BUTTON_START:
                        if (model_lavaggio_pagato(pmodel)) {
                            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_START_PROGRAM;
                        } else {
                            ESP_LOGI(TAG, "Missing credit!");
                        }
                        break;

                    default:
                        break;
                }
            }
            break;
        }

        default:
            break;
    }

    return msg;
}


static void close_page(void *arg) {
    struct page_data *pdata = arg;
    lv_obj_clean(lv_scr_act());
    lv_task_set_prio(pdata->timer, LV_TASK_PRIO_OFF);
}


static void destroy_page(void *arg, void *extra) {
    struct page_data *pdata = arg;
    lv_task_del(pdata->timer);
    free(pdata);
}


const pman_page_t page_choice_lab = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};