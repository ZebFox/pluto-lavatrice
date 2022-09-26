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
#include "utils/utils.h"
#include "gel/timer/timecheck.h"
#include "esp_log.h"


#define ALLARME_CHIAVISTELLO 15
#define ALLARME_SCARICO      32


struct page_data {
    lv_obj_t *lbl_step;
    lv_obj_t *lbl_phase;
    lv_obj_t *lbl_remaining;
    lv_obj_t *lbl_total_remaining;
    lv_obj_t *lbl_step_num;
    lv_obj_t *lbl_alarm;
    lv_obj_t *lbl_alarm_code;

    lv_obj_t *popup_alarm;

    lv_task_t *timer;

    uint16_t      alarm;
    unsigned long alarm_ts;
    unsigned long timestamp;
    unsigned long force_ts;
    int           scarico_fallito;
    uint8_t       wait_for_release;
};


static const char *TAG = "PageWork";


static void update_page(model_t *pmodel, struct page_data *pdata) {
    parametri_step_t *step = model_get_current_step(pmodel);
    lv_label_set_text(pdata->lbl_step, view_common_step2str(pmodel, step->tipo));

    uint16_t rimanente = pmodel->run.macchina.rimanente;
    lv_label_set_text_fmt(pdata->lbl_remaining, "%02im%02is [%i %i]", rimanente / 60, rimanente % 60,
                          pmodel->run.macchina.temperatura, model_get_livello(pmodel));

    rimanente = model_program_remaining(pmodel);
    lv_label_set_text_fmt(pdata->lbl_total_remaining, "%02im%02is", rimanente / 60, rimanente % 60);

    const programma_lavatrice_t *program = model_get_program(pmodel);
    lv_label_set_text_fmt(pdata->lbl_step_num, "# %s %02i/%02i #",
                          view_intl_get_string_from_language(model_get_temporary_language(pmodel), STRINGS_PASSO),
                          model_get_current_step_number(pmodel) + 1, program->num_steps);

    if (pmodel->run.f_richiedi_scarico) {
        lv_label_set_text(pdata->lbl_phase, view_intl_get_string_from_language(model_get_temporary_language(pmodel),
                                                                               STRINGS_SCARICO_NECESSARIO));
    } else if (model_macchina_in_scarico_forzato(pmodel)) {
        lv_label_set_text(pdata->lbl_phase, view_intl_get_string_from_language(model_get_temporary_language(pmodel),
                                                                               STRINGS_SCARICO_FORZATO));
    } else if (model_macchina_in_pausa(pmodel)) {
        lv_label_set_text(pdata->lbl_phase, view_intl_get_string_from_language(model_get_temporary_language(pmodel),
                                                                               STRINGS_PAUSA_LAVAGGIO));
    } else if (pmodel->run.macchina.descrizione_pedante == 0) {
        lv_label_set_text(pdata->lbl_phase, view_common_step2str(pmodel, step->tipo));
    } else {
        lv_label_set_text(pdata->lbl_phase, view_common_pedantic_string(pmodel));
    }

    view_common_update_alarm_popup(pmodel, &pdata->alarm, &pdata->alarm_ts, pdata->popup_alarm, pdata->lbl_alarm,
                                   pdata->lbl_alarm_code);
}


static void *create_page(model_t *model, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->timer            = view_register_periodic_task(500UL, LV_TASK_PRIO_OFF, 0);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    pdata->wait_for_release = 0;

    pdata->alarm    = 0;
    pdata->alarm_ts = 0;

    lv_task_set_prio(pdata->timer, LV_TASK_PRIO_MID);

    const programma_lavatrice_t *program = model_get_program(pmodel);

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_label_set_text_fmt(lbl, "%02i", model_get_program_num(pmodel) + 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 1, 1);

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_SROLL_CIRC);
    lv_obj_set_width(lbl, 112);
    lv_label_set_text(lbl, program->nomi[model_get_language(pmodel)]);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 17, 1);

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_CENTER, 0, 2);
    pdata->lbl_step_num = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lbl, 1);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_align(lbl, pdata->lbl_step_num, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    pdata->lbl_step = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text_fmt(lbl, "# %s #",
                          view_intl_get_string_from_language(model_get_temporary_language(pmodel), STRINGS_FASE));
    lv_obj_align(lbl, NULL, LV_ALIGN_CENTER, 0, 18);
    lv_obj_t *lbl_prev = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lbl, 1);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_SROLL);
    lv_obj_set_width(lbl, 128);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_align(lbl, lbl_prev, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    pdata->lbl_phase = lbl;

    lv_obj_t *img = custom_lv_img_create(lv_scr_act(), NULL);
    custom_lv_img_set_src(img, &legacy_img_time);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_MID, -48, 12);
    // -16

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_align(lbl, img, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_set_auto_realign(lbl, 1);
    pdata->lbl_total_remaining = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, img, LV_ALIGN_OUT_RIGHT_BOTTOM, 0, 0);
    pdata->lbl_remaining = lbl;

    lv_obj_t *line = view_common_horizontal_line();
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);

    static lv_point_t points[2] = {{0, 0}, {0, 10}};
    line                        = view_common_line(points, 2);
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_LEFT, 14, 0);

    line = view_common_horizontal_line();
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_MID, 0, 28);

    pdata->popup_alarm = view_common_alarm_popup(&pdata->lbl_alarm, &pdata->lbl_alarm_code);

    lv_obj_set_hidden(pdata->popup_alarm, 1);

    update_page(pmodel, pdata);
}


static view_message_t process_page_event(model_t *pmodel, void *arg, pman_event_t event) {
    view_message_t    msg   = VIEW_EMPTY_MSG;
    struct page_data *pdata = arg;

    switch (event.code) {
        case VIEW_EVENT_CODE_MODEL_UPDATE:
            if (model_macchina_in_stop(pmodel) || !model_can_work(pmodel)) {
                pmodel->run.done = 1;
                msg.vmsg.code    = VIEW_PAGE_COMMAND_CODE_REBASE;
                msg.vmsg.page    = (void *)view_main_page(pmodel);
            }

            if (model_alarm_code(pmodel) == ALLARME_SCARICO || model_alarm_code(pmodel) == ALLARME_CHIAVISTELLO) {
                if (pdata->scarico_fallito == 0) {
                    ESP_LOGI(TAG, "Scarico fallito");
                    pdata->scarico_fallito = 1;
                }
            }
            update_page(pmodel, pdata);
            break;

        case VIEW_EVENT_CODE_TIMER:
            update_page(pmodel, pdata);
            break;


        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                lv_task_reset(pdata->timer);

                switch (event.key_event.code) {
                    case BUTTON_STOP:
                        pdata->timestamp = get_millis();
                        break;

                    case BUTTON_PIU:
                        if (model_macchina_in_pausa(pmodel)) {
                            model_avanza_step(pmodel);
                            update_page(pmodel, pdata);
                        }
                        break;

                    case BUTTON_MENO:
                        if (model_macchina_in_pausa(pmodel)) {
                            model_arretra_step(pmodel);
                            update_page(pmodel, pdata);
                        }
                        break;

                    case BUTTON_SINISTRA:
                        break;

                    case BUTTON_DESTRA:
                        break;

                    case BUTTON_START:
                        if (model_alarm_code(pmodel) > 0) {
                            lv_obj_set_hidden(pdata->popup_alarm, 1);
                            pdata->alarm_ts = get_millis();
                            msg.cmsg.code   = VIEW_CONTROLLER_COMMAND_CODE_AZZERA_ALLARMI;
                        } else {
                            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_START_PROGRAM;
                        }
                        break;

                    case BUTTON_STOP_KEY:
                        pdata->force_ts = get_millis();
                        break;

                    default:
                        break;
                }
            } else if ((event.key_event.event == KEY_PRESSING || event.key_event.event == KEY_LONGPRESS) &&
                       event.key_event.code == BUTTON_STOP) {
                if (pdata->wait_for_release) {
                    break;
                }

                if (model_macchina_in_pausa(pmodel)) {
                    if (is_expired(pdata->timestamp, get_millis(), pmodel->prog.parmac.secondi_stop * 1000UL)) {
                        ESP_LOGI(TAG, "Requesting stop");
                        msg.cmsg.code    = VIEW_CONTROLLER_COMMAND_CODE_STOP;
                        pdata->timestamp = get_millis();
                    }
                } else {
                    if (is_expired(pdata->timestamp, get_millis(), pmodel->prog.parmac.secondi_pausa * 1000UL)) {
                        if (model_macchina_in_marcia(pmodel)) {
                            ESP_LOGI(TAG, "Requesting pause");
                            msg.cmsg.code           = VIEW_CONTROLLER_COMMAND_CODE_PAUSE;
                            pdata->timestamp        = get_millis();
                            pdata->wait_for_release = 1;
                        } else {
                            ESP_LOGI(TAG, "State: %i", pmodel->run.macchina.stato);
                        }
                    }
                }
            } else if (event.key_event.event == KEY_LONGPRESS && event.key_event.code == BUTTON_STOP_KEY) {
                if (model_macchina_in_pausa(pmodel) && pdata->scarico_fallito) {
                    if (is_expired(pdata->force_ts, get_millis(), 10000UL)) {
                        msg.cmsg.code   = VIEW_CONTROLLER_COMMAND_CODE_FORZA_APERTURA_OBLO;
                        pdata->force_ts = get_millis();
                    }
                } else {
                    pdata->force_ts = get_millis();
                }
            } else if (event.key_event.event == KEY_LONGCLICK) {
                switch (event.key_event.code) {
                    case BUTTON_STOP:
                        if (pmodel->run.f_richiedi_scarico) {
                            pmodel->run.f_richiedi_scarico = 0;
                            msg.cmsg.code                  = VIEW_CONTROLLER_COMMAND_CODE_FORCE_DRAIN;
                            update_page(pmodel, pdata);
                        }
                        break;

                    case BUTTON_START:
                        break;
                }
            } else if (event.key_event.event == KEY_RELEASE && event.key_event.code == BUTTON_STOP) {
                pdata->wait_for_release = 0;
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


const pman_page_t page_work = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};