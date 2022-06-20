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
#include "gel/parameter/parameter.h"
#include "model/descriptions/AUTOGEN_FILE_pars.h"
#include "model/descriptions/parstring.h"


#define ALLARME_CHIAVISTELLO 15
#define ALLARME_SCARICO      32

enum {
    DETERGENT_TIMER_ID,
    BLINK_TIMER_ID,
};


typedef enum {
    DETERGENT_STATE_OFF = 0,
    DETERGENT_STATE_ON_REQUESTED,
    DETERGENT_STATE_ON,
} detergent_state_t;


struct page_data {
    lv_obj_t *lbl_step;
    lv_obj_t *lbl_phase;
    lv_obj_t *lbl_remaining;
    lv_obj_t *lbl_total_remaining;
    lv_obj_t *lbl_step_num;
    lv_obj_t *lbl_alarm;
    lv_obj_t *lbl_alarm_code;
    lv_obj_t *lbl_required_temperature;
    lv_obj_t *lbl_temperature;
    lv_obj_t *lbl_required_level;
    lv_obj_t *lbl_level;
    lv_obj_t *lbl_required_speed;
    lv_obj_t *lbl_speed;
    lv_obj_t *lbl_menu_description;
    lv_obj_t *lbl_menu_value;
    lv_obj_t *lbl_detergent_num;
    lv_obj_t *lbl_detergent_configured_time;
    lv_obj_t *lbl_detergent_actual_time;

    lv_obj_t *img_menu;

    lv_obj_t *popup_alarm;
    lv_obj_t *popup_menu;
    lv_obj_t *popup_detergent;

    lv_task_t *blink_timer;
    lv_task_t *detergent_timer;

    uint16_t           params[4];
    parameter_handle_t ps[4];
    size_t             menu_index;

    uint16_t          alarm;
    uint16_t          detergent_index;
    unsigned long     alarm_ts;
    unsigned long     timestamp;
    unsigned long     detergent_time[MAX_DETERGENTS];
    detergent_state_t detergent_state[MAX_DETERGENTS];
    unsigned long     detergent_ts;
    int               scarico_fallito;
    uint16_t          counter;
    int               flag;
};


static void init_ephemeral_parameters(model_t *pmodel, struct page_data *pdata, int reset);
static void toggle_menu_popup(model_t *pmodel, struct page_data *pdata);
static void toggle_detergent_popup(model_t *pmodel, struct page_data *pdata);


static const char *TAG = "PageWorkLab";


static void update_popup_detergent(model_t *pmodel, struct page_data *pdata) {
    lv_label_set_text_fmt(pdata->lbl_detergent_num, "%s %i", view_intl_get_string(pmodel, STRINGS_SAPONE),
                          pdata->detergent_index + 1);

    lv_label_set_text_fmt(pdata->lbl_detergent_configured_time, "play   [%03is]",
                          pmodel->prog.parmac.tempo_tasto_carico_saponi);
    lv_label_set_text_fmt(pdata->lbl_detergent_actual_time, "menu   [%03is]",
                          pdata->detergent_time[pdata->detergent_index] / 1000UL);
}


static void update_popup_menu(model_t *pmodel, struct page_data *pdata) {
    const GSYMBOL *images[4] = {
        &legacy_img_time,
        &legacy_img_temperature,
        &legacy_img_level,
        &legacy_img_speed,
    };

    custom_lv_img_set_src(pdata->img_menu, images[pdata->menu_index]);

    parameter_user_data_t data = parameter_get_user_data(&pdata->ps[pdata->menu_index]);
    lv_label_set_text(pdata->lbl_menu_description, data.descrizione[model_get_temporary_language(pmodel)]);

    char string[32] = {0};
    data.format(string, model_get_temporary_language(pmodel), &pdata->ps[pdata->menu_index]);
    lv_label_set_text(pdata->lbl_menu_value, string);
}


static void update_page(model_t *pmodel, struct page_data *pdata) {
    view_common_set_hidden(pdata->lbl_step, pdata->flag);
    view_common_set_hidden(pdata->lbl_phase, !pdata->flag);

    parametri_step_t *step = model_get_current_step(pmodel);
    lv_label_set_text(pdata->lbl_step, view_common_step2str(pmodel, step->tipo));

    uint16_t rimanente = pmodel->run.macchina.rimanente;
    lv_label_set_text_fmt(pdata->lbl_remaining, "%02im%02is", rimanente / 60, rimanente % 60);

    rimanente = model_program_remaining(pmodel);
    lv_label_set_text_fmt(pdata->lbl_total_remaining, "%02im%02is", rimanente / 60, rimanente % 60);

    lv_label_set_text_fmt(pdata->lbl_required_temperature, "%02iC", pmodel->run.macchina.temperatura_impostata);
    lv_label_set_text_fmt(pdata->lbl_temperature, "%02iC", pmodel->run.macchina.temperatura);

    char    *level_unit = model_is_level_in_cm(&pmodel->prog.parmac) ? "cm" : "lt";
    uint16_t level =
        model_is_level_in_cm(&pmodel->prog.parmac) ? pmodel->run.macchina.livello : pmodel->run.macchina.livello_litri;
    lv_label_set_text_fmt(pdata->lbl_required_level, "%02i%s", pmodel->run.macchina.livello_impostato, level_unit);
    lv_label_set_text_fmt(pdata->lbl_level, "%02i%s", level, level_unit);

    lv_label_set_text_fmt(pdata->lbl_required_speed, "%02i", pmodel->run.macchina.velocita_rpm);
    lv_label_set_text_fmt(pdata->lbl_speed, "%02i", pmodel->run.macchina.velocita_rpm);

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
    pdata->blink_timer      = view_register_periodic_task(2000UL, LV_TASK_PRIO_OFF, BLINK_TIMER_ID);
    pdata->detergent_timer  = view_register_periodic_task(200UL, LV_TASK_PRIO_OFF, DETERGENT_TIMER_ID);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;

    for (size_t i = 0; i < MAX_DETERGENTS; i++) {
        pdata->detergent_time[i]  = 0;
        pdata->detergent_state[i] = DETERGENT_STATE_OFF;
    }

    pdata->counter         = 0;
    pdata->flag            = 0;
    pdata->alarm           = 0;
    pdata->alarm_ts        = 0;
    pdata->detergent_ts    = get_millis();
    pdata->detergent_index = 0;

    init_ephemeral_parameters(pmodel, pdata, 1);

    lv_task_set_prio(pdata->blink_timer, LV_TASK_PRIO_MID);
    lv_task_set_prio(pdata->detergent_timer, LV_TASK_PRIO_MID);

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
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -8);
    pdata->lbl_step_num = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lbl, 1);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    pdata->lbl_step = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    pdata->lbl_phase = lbl;

    lv_obj_t *img = custom_lv_img_create(lv_scr_act(), NULL);
    custom_lv_img_set_src(img, &legacy_img_time);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 12);

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(lbl, img, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_set_auto_realign(lbl, 1);
    pdata->lbl_total_remaining = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_8x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, img, LV_ALIGN_OUT_RIGHT_BOTTOM, 0, 0);
    pdata->lbl_remaining = lbl;

    img = custom_lv_img_create(lv_scr_act(), NULL);
    custom_lv_img_set_src(img, &legacy_img_temperature);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_RIGHT, -32, 12);

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(lbl, img, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_set_auto_realign(lbl, 1);
    pdata->lbl_required_temperature = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_8x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, img, LV_ALIGN_OUT_RIGHT_BOTTOM, 0, 0);
    pdata->lbl_temperature = lbl;

    img = custom_lv_img_create(lv_scr_act(), NULL);
    custom_lv_img_set_src(img, &legacy_img_level);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 30);

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(lbl, img, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_set_auto_realign(lbl, 1);
    pdata->lbl_required_level = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_8x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, img, LV_ALIGN_OUT_RIGHT_BOTTOM, 0, 0);
    pdata->lbl_level = lbl;

    img = custom_lv_img_create(lv_scr_act(), NULL);
    custom_lv_img_set_src(img, &legacy_img_speed);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_RIGHT, -34, 30);

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(lbl, img, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_set_auto_realign(lbl, 1);
    pdata->lbl_required_speed = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_8x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, img, LV_ALIGN_OUT_RIGHT_BOTTOM, 0, 0);
    pdata->lbl_speed = lbl;

    img = custom_lv_img_create(lv_scr_act(), NULL);
    custom_lv_img_set_src(img, &legacy_img_rpm);
    lv_obj_set_auto_realign(img, 1);
    lv_obj_align(img, pdata->lbl_required_speed, LV_ALIGN_OUT_RIGHT_TOP, -14, 0);

    lv_obj_t *line = view_common_horizontal_line();
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);

    static lv_point_t points[2] = {{0, 0}, {0, 10}};
    line                        = view_common_line(points, 2);
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_LEFT, 14, 0);

    line = view_common_horizontal_line();
    lv_obj_align(line, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -16);

    lv_obj_t *content;
    pdata->popup_menu = view_common_popup(lv_scr_act(), &content);

    img = custom_lv_img_create(content, NULL);
    custom_lv_img_set_src(img, &legacy_img_time);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_LEFT, 2, 4);
    pdata->img_menu = img;

    lbl = lv_label_create(content, NULL);
    lv_obj_set_style(lbl, &style_label_8x16);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, img, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    pdata->lbl_menu_description = lbl;

    lbl = lv_label_create(content, NULL);
    lv_obj_set_style(lbl, &style_label_8x16);
    lv_obj_set_auto_realign(lbl, 1);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -8);
    pdata->lbl_menu_value = lbl;

    pdata->popup_detergent = view_common_popup(lv_scr_act(), &content);

    lbl = lv_label_create(content, NULL);
    lv_obj_set_style(lbl, &style_label_8x16);
    lv_label_set_align(lbl, LV_ALIGN_CENTER);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_MID, 0, 4);
    pdata->lbl_detergent_num = lbl;

    lbl = lv_label_create(content, NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_label_set_align(lbl, LV_ALIGN_CENTER);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_MID, 0, 24);
    pdata->lbl_detergent_configured_time = lbl;

    lbl = lv_label_create(content, NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_label_set_align(lbl, LV_ALIGN_CENTER);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_MID, 0, 34);
    pdata->lbl_detergent_actual_time = lbl;

    pdata->popup_alarm = view_common_alarm_popup(&pdata->lbl_alarm, &pdata->lbl_alarm_code);

    lv_obj_set_hidden(pdata->popup_alarm, 1);
    lv_obj_set_hidden(pdata->popup_menu, 1);
    lv_obj_set_hidden(pdata->popup_detergent, 1);

    update_page(pmodel, pdata);
    update_popup_menu(pmodel, pdata);
}


static view_message_t process_page_event(model_t *pmodel, void *arg, pman_event_t event) {
    view_message_t    msg   = VIEW_EMPTY_MSG;
    struct page_data *pdata = arg;

    switch (event.code) {
        case VIEW_EVENT_CODE_MODEL_UPDATE:
            if (model_macchina_in_stop(pmodel) || !model_can_work(pmodel)) {
                msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_REBASE;
                msg.vmsg.page = (void *)view_main_page(pmodel);
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
            switch (event.timer_id) {
                case DETERGENT_TIMER_ID:
                    for (size_t i = 0; i < pmodel->prog.parmac.numero_saponi_utilizzabili; i++) {
                        if (pdata->detergent_state[i] == DETERGENT_STATE_ON && !pmodel->run.macchina.out_saponi[i]) {
                            pdata->detergent_state[i] = DETERGENT_STATE_OFF;
                        } else if (pdata->detergent_state[i] == DETERGENT_STATE_ON_REQUESTED &&
                                   pmodel->run.macchina.out_saponi[i]) {
                            pdata->detergent_state[i] = DETERGENT_STATE_ON;
                        }
                        if (pdata->detergent_state[i] == DETERGENT_STATE_ON) {
                            pdata->detergent_time[i] += time_interval(pdata->detergent_ts, get_millis());
                        }
                    }
                    pdata->detergent_ts = get_millis();
                    if (!lv_obj_get_hidden(pdata->popup_detergent)) {
                        update_popup_detergent(pmodel, pdata);
                    }
                    break;

                case BLINK_TIMER_ID:
                    pdata->flag = !pdata->flag;
                    update_page(pmodel, pdata);
                    break;
            }
            break;

        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                lv_task_reset(pdata->blink_timer);

                switch (event.key_event.code) {
                    case BUTTON_STOP:
                        pdata->timestamp = get_millis();
                        view_common_set_hidden(pdata->popup_menu, 1);
                        view_common_set_hidden(pdata->popup_detergent, 1);
                        break;

                    case BUTTON_PIU:
                        if (!lv_obj_get_hidden(pdata->popup_menu)) {
                            parameter_operator(&pdata->ps[pdata->menu_index], 1);
                            if (pdata->menu_index == 0) {
                                pdata->params[0] = (pdata->params[0] / 10) * 10;
                            }
                            update_popup_menu(pmodel, pdata);
                        } else if (model_macchina_in_pausa(pmodel)) {
                            model_avanza_step(pmodel);
                            update_page(pmodel, pdata);
                        }
                        break;

                    case BUTTON_MENO:
                        if (!lv_obj_get_hidden(pdata->popup_menu)) {
                            parameter_operator(&pdata->ps[pdata->menu_index], -1);
                            if (pdata->menu_index == 0) {
                                pdata->params[0] = (pdata->params[0] / 10) * 10;
                            }
                            update_popup_menu(pmodel, pdata);
                        } else if (model_macchina_in_pausa(pmodel)) {
                            model_arretra_step(pmodel);
                            update_page(pmodel, pdata);
                        }
                        break;

                    case BUTTON_SINISTRA:
                        if (!lv_obj_get_hidden(pdata->popup_menu)) {
                            pdata->menu_index = utils_circular_decrease(pdata->menu_index, 4);
                            update_popup_menu(pmodel, pdata);
                        } else if (!lv_obj_get_hidden(pdata->popup_detergent)) {
                            pdata->detergent_index = utils_circular_decrease(
                                pdata->detergent_index, pmodel->prog.parmac.numero_saponi_utilizzabili);
                            update_popup_detergent(pmodel, pdata);
                        }
                        break;

                    case BUTTON_DESTRA:
                        if (!lv_obj_get_hidden(pdata->popup_menu)) {
                            pdata->menu_index = (pdata->menu_index + 1) % 4;
                            update_popup_menu(pmodel, pdata);
                        } else if (!lv_obj_get_hidden(pdata->popup_detergent)) {
                            pdata->detergent_index =
                                (pdata->detergent_index + 1) % pmodel->prog.parmac.numero_saponi_utilizzabili;
                            update_popup_detergent(pmodel, pdata);
                        }
                        break;

                    case BUTTON_START:
                        if (!lv_obj_get_hidden(pdata->popup_detergent)) {
                            msg.cmsg.code                                  = VIEW_CONTROLLER_COMMAND_CODE_COLPO_SAPONE;
                            msg.cmsg.output                                = pdata->detergent_index;
                            pdata->detergent_state[pdata->detergent_index] = DETERGENT_STATE_ON_REQUESTED;
                        } else if (model_alarm_code(pmodel) > 0) {
                            lv_obj_set_hidden(pdata->popup_alarm, 1);
                            pdata->alarm_ts = get_millis();
                            msg.cmsg.code   = VIEW_CONTROLLER_COMMAND_CODE_AZZERA_ALLARMI;
                        } else {
                            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_START_PROGRAM;
                        }
                        break;

                    case BUTTON_MENU:
                        if (lv_obj_get_hidden(pdata->popup_detergent)) {
                            toggle_menu_popup(pmodel, pdata);
                        }
                        break;

                    case BUTTON_KEY:
                        if (!lv_obj_get_hidden(pdata->popup_menu)) {
                            toggle_detergent_popup(pmodel, pdata);
                        }
                        break;

                    default:
                        break;
                }
            } else if ((event.key_event.event == KEY_PRESSING || event.key_event.event == KEY_LONGPRESS)) {
                switch (event.key_event.code) {
                    case BUTTON_STOP:
                        if (model_macchina_in_pausa(pmodel)) {
                            if (is_expired(pdata->timestamp, get_millis(), pmodel->prog.parmac.secondi_stop * 1000UL)) {
                                ESP_LOGI(TAG, "Requesting stop");
                                msg.cmsg.code    = VIEW_CONTROLLER_COMMAND_CODE_STOP;
                                pdata->timestamp = get_millis();
                            }
                        } else {
                            if (is_expired(pdata->timestamp, get_millis(),
                                           pmodel->prog.parmac.secondi_pausa * 1000UL)) {
                                if (model_macchina_in_marcia(pmodel)) {
                                    ESP_LOGI(TAG, "Requesting pause");
                                    msg.cmsg.code    = VIEW_CONTROLLER_COMMAND_CODE_PAUSE;
                                    pdata->timestamp = get_millis();
                                } else {
                                    ESP_LOGI(TAG, "State: %i", pmodel->run.macchina.stato);
                                }
                            }
                        }
                        break;

                    case BUTTON_PIU:
                        if (event.key_event.event == KEY_LONGPRESS && !lv_obj_get_hidden(pdata->popup_menu)) {
                            parameter_operator(&pdata->ps[pdata->menu_index], 1);
                            if (pdata->menu_index == 0) {
                                pdata->params[0] = (pdata->params[0] / 10) * 10;
                            }
                            update_popup_menu(pmodel, pdata);
                        }
                        break;

                    case BUTTON_MENO:
                        if (event.key_event.event == KEY_LONGPRESS && !lv_obj_get_hidden(pdata->popup_menu)) {
                            parameter_operator(&pdata->ps[pdata->menu_index], -1);
                            if (pdata->menu_index == 0) {
                                pdata->params[0] = (pdata->params[0] / 10) * 10;
                            }
                            update_popup_menu(pmodel, pdata);
                        }
                        break;
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
                        if (model_macchina_in_pausa(pmodel) && pdata->scarico_fallito) {
                            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_FORZA_APERTURA_OBLO;
                        }
                        break;
                }
            } else if (event.key_event.event == KEY_RELEASE) {
                switch (event.key_event.code) {
                    case BUTTON_PIU:
                    case BUTTON_MENO:
                        if (!lv_obj_get_hidden(pdata->popup_menu)) {
                            msg.cmsg.code        = VIEW_CONTROLLER_COMMAND_CODE_MODIFICA_PARAMETRI_IN_LAVAGGIO;
                            msg.cmsg.duration    = pdata->params[0];
                            msg.cmsg.level       = pdata->params[1];
                            msg.cmsg.temperature = pdata->params[2];
                            msg.cmsg.speed       = pdata->params[3];
                        }
                        break;

                    case BUTTON_MENU:
                        if (!lv_obj_get_hidden(pdata->popup_detergent)) {
                            msg.cmsg.code   = VIEW_CONTROLLER_COMMAND_CODE_CONTROLLO_SAPONE;
                            msg.cmsg.output = pdata->detergent_index;
                            pdata->detergent_state[pdata->detergent_index] = DETERGENT_STATE_OFF;
                            msg.cmsg.value                                 = 0;
                        }
                        break;
                }
            } else if (event.key_event.event == KEY_PRESS) {
                switch (event.key_event.code) {
                    case BUTTON_MENU:
                        if (!lv_obj_get_hidden(pdata->popup_detergent)) {
                            msg.cmsg.code   = VIEW_CONTROLLER_COMMAND_CODE_CONTROLLO_SAPONE;
                            msg.cmsg.output = pdata->detergent_index;
                            msg.cmsg.value  = 1;
                            pdata->detergent_state[pdata->detergent_index] = DETERGENT_STATE_ON_REQUESTED;
                        }
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
    lv_task_set_prio(pdata->blink_timer, LV_TASK_PRIO_OFF);
    lv_task_set_prio(pdata->detergent_timer, LV_TASK_PRIO_OFF);
}


static void destroy_page(void *arg, void *extra) {
    struct page_data *pdata = arg;
    lv_task_del(pdata->blink_timer);
    lv_task_del(pdata->detergent_timer);
    free(pdata);
}


static void init_ephemeral_parameters(model_t *pmodel, struct page_data *pdata, int reset) {
    parmac_t               *parmac = &pmodel->prog.parmac;
    const parametri_step_t *s      = model_get_current_step(pmodel);
    uint16_t               *livmin, *livmax;
    pdata->menu_index = 0;

    if (model_is_level_in_cm(parmac)) {
        livmin = &parmac->centimetri_minimo_riscaldo;
        livmax = &parmac->centimetri_max_livello;
    } else {
        livmin = &parmac->litri_minimi_riscaldo;
        livmax = &parmac->litri_massimi;
    }

    pdata->params[3] = pmodel->run.macchina.velocita_rpm;
    pdata->params[0] = pmodel->run.macchina.rimanente;

    if (s != NULL && reset) {
        // pdata->params[3] = s->velocita_lavaggio;
        pdata->params[2] = s->temperatura;
        pdata->params[1] = s->livello;
    }

    char *fmt    = model_is_level_in_cm(parmac) ? "%i cm" : "%i lt";
    pdata->ps[0] = PARAMETER_STEP(&pdata->params[0], 10, 60 * 60, 0, 10, FTIME(PARS_DESCRIPTIONS_DURATA), 1);
    pdata->ps[1] =
        PARAMETER_DLIMITS(&pdata->params[1], livmin, livmax, 0, 0, 0, FFINT(PARS_DESCRIPTIONS_LIVELLO, fmt), 1);
    pdata->ps[2] = PARAMETER_DLIMITS(&pdata->params[2], NULL, &parmac->temperatura_massima, 0, 0, 0,
                                     FFINT(PARS_DESCRIPTIONS_TEMPERATURA, "%i C"), 1);
    pdata->ps[3] =
        PARAMETER_DLIMITS(&pdata->params[3], &parmac->velocita_minima_lavaggio, &parmac->velocita_massima_lavaggio, 0,
                          0, 0, FFINT(PARS_DESCRIPTIONS_VELOCITA, "%i rpm"), 1);
}


static void toggle_menu_popup(model_t *pmodel, struct page_data *pdata) {
    if (lv_obj_get_hidden(pdata->popup_menu) && pmodel->prog.parmac.visualizzazione_menu) {
        init_ephemeral_parameters(pmodel, pdata, 0);
        lv_obj_set_hidden(pdata->popup_menu, 0);
        update_popup_menu(pmodel, pdata);
        lv_obj_set_hidden(pdata->popup_detergent, 1);
    } else {
        lv_obj_set_hidden(pdata->popup_menu, 1);
    }
}


static void toggle_detergent_popup(model_t *pmodel, struct page_data *pdata) {
    if (lv_obj_get_hidden(pdata->popup_detergent) && pmodel->prog.parmac.visualizzazione_menu_saponi &&
        pmodel->prog.parmac.numero_saponi_utilizzabili > 0) {
        lv_obj_set_hidden(pdata->popup_detergent, 0);
        lv_obj_set_hidden(pdata->popup_menu, 1);
        update_popup_detergent(pmodel, pdata);
    } else {
        lv_obj_set_hidden(pdata->popup_detergent, 1);
    }
}


const pman_page_t page_work_lab = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};