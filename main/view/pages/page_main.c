#include <stdlib.h>
#include <assert.h>
#include "controller/com/machine.h"
#include "lvgl/lvgl.h"
#include "src/lv_core/lv_obj.h"
#include "view/view.h"
#include "view/images/legacy.h"
#include "view/view_types.h"
#include "view/widgets/custom_lv_img.h"
#include "gel/pagemanager/page_manager.h"
#include "gel/timer/timecheck.h"
#include "peripherals/keyboard.h"
#include "view/common.h"
#include "view/styles.h"
#include "view/intl/intl.h"
#include "utils/utils.h"
#include "esp_log.h"
#include "config/app_config.h"


enum {
    TIMER_500MS_ID,
};


struct page_data {
    size_t                 index;
    view_common_password_t password;

    lv_task_t *timer;

    lv_obj_t *lbl_name;
    lv_obj_t *lbl_status;
    lv_obj_t *lbl_time;
    lv_obj_t *lbl_washes;
    lv_obj_t *lbl_info;
    lv_obj_t *lbl_language;
    lv_obj_t *lbl_alarm;
    lv_obj_t *lbl_alarm_code;
    lv_obj_t *lbl_done;

    lv_obj_t *img_left;
    lv_obj_t *img_right;
    lv_obj_t *img_washes_sm;
    lv_obj_t *img_info;
    lv_obj_t *img_tipo;
    lv_obj_t *img_rpm;

    size_t index_info;

    unsigned long popup_language_ts;
    unsigned long language_ts;
    int           flag_status;
    uint16_t      counter;

    lv_obj_t *popup_comunication_error;
    lv_obj_t *popup_language;
    lv_obj_t *popup_alarm;
    lv_obj_t *popup_done;

    uint16_t      allarme;
    unsigned long alarm_ts;
};


static void update_prog_data(model_t *pmodel, struct page_data *pdata);
static void update_timer_data(model_t *pmodel, struct page_data *pdata);
static void update_status(model_t *pmodel, struct page_data *pdata);
static void update_language_popup(model_t *pmodel, struct page_data *pdata);
static void update_done_popup(model_t *pmodel, struct page_data *pdata);


static const char *TAG = "PageMain";


static void *create_page(model_t *pmodel, void *extra) {
    (void)TAG;
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->timer            = view_register_periodic_task(500UL, LV_TASK_PRIO_OFF, TIMER_500MS_ID);
    pdata->index            = 0;

    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    pdata->flag_status      = 0;
    pdata->index_info       = 0;
    pdata->allarme          = 0;

    lv_task_set_prio(pdata->timer, LV_TASK_PRIO_MID);

    view_common_password_reset(&pdata->password, get_millis());

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_8x16);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_SROLL_CIRC);
    lv_obj_set_width(lbl, 128);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, -14);
    pdata->lbl_name = lbl;

    lv_obj_t *img = custom_lv_img_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(img, 1);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_LEFT, 16, 8);
    pdata->img_tipo = img;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_8x16);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_SROLL_CIRC);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_width(lbl, 128);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    pdata->lbl_status = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
    pdata->lbl_time = lbl;

    img = custom_lv_img_create(lv_scr_act(), NULL);
    custom_lv_img_set_src(img, &legacy_img_left);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_LEFT, 1, 16);
    pdata->img_left = img;

    img = custom_lv_img_create(lv_scr_act(), NULL);
    custom_lv_img_set_src(img, &legacy_img_right);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_RIGHT, -1, 16);
    pdata->img_right = img;

    img = custom_lv_img_create(lv_scr_act(), NULL);
    custom_lv_img_set_src(img, &legacy_img_wash_sm);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_MID, 0, 8);
    pdata->img_washes_sm = img;

    img = custom_lv_img_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(img, 1);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_MID, -8, 22);
    pdata->img_info = img;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_8x16);
    lv_obj_align(lbl, pdata->img_washes_sm, LV_ALIGN_OUT_RIGHT_MID, 0, -2);
    pdata->lbl_washes = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_8x16);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, pdata->img_info, LV_ALIGN_OUT_RIGHT_MID, 0, -2);
    pdata->lbl_info = lbl;

    img = custom_lv_img_create(lv_scr_act(), NULL);
    custom_lv_img_set_src(img, &legacy_img_rpm);
    lv_obj_set_auto_realign(img, 1);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_MID, 40, 22);
    pdata->img_rpm = img;

    pdata->flag_status = 0;

    lv_obj_t *line = view_common_horizontal_line();
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_MID, 0, 8);
    line = view_common_horizontal_line();
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_MID, 0, 36);

    line = view_common_horizontal_line();
    lv_obj_align(line, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -12);

    static lv_point_t points[2] = {{0, 0}, {0, 26}};
    line                        = view_common_line(points, 2);
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_LEFT, 14, 10);
    line = view_common_line(points, 2);
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_LEFT, 16 + 30, 10);
    line = view_common_line(points, 2);
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_RIGHT, -14, 10);

    static lv_point_t hor_points[2] = {{0, 0}, {65, 0}};
    line                            = view_common_line(hor_points, 2);
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_MID, 15, 22);

    lv_obj_t *done_cont;
    pdata->popup_done = view_common_popup(lv_scr_act(), &done_cont);

    lbl = lv_label_create(done_cont, NULL);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_BREAK);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_width(lbl, 95);
    lv_obj_align(lbl, NULL, LV_ALIGN_CENTER, 0, 0);
    pdata->lbl_done = lbl;

    lv_obj_t *lang_cont;
    pdata->popup_language = view_common_popup(lv_scr_act(), &lang_cont);

    lbl = lv_label_create(lang_cont, NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_label_set_text(lbl,
                      view_intl_get_string_from_language(model_get_temporary_language(pmodel), STRINGS_LINGUA_VIDEATE));
    lv_obj_align(lbl, NULL, LV_ALIGN_CENTER, 0, -10);

    lbl = lv_label_create(lang_cont, NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_CENTER, 0, 10);
    pdata->lbl_language = lbl;

    pdata->popup_alarm = view_common_alarm_popup(&pdata->lbl_alarm, &pdata->lbl_alarm_code);

    lv_obj_t *comm_cont;
    pdata->popup_comunication_error = view_common_popup(lv_scr_act(), &comm_cont);

    lbl = lv_label_create(comm_cont, NULL);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_BREAK);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_width(lbl, 95);
    lv_label_set_text(
        lbl, view_intl_get_string_from_language(model_get_temporary_language(pmodel), STRINGS_ERRORE_COMUNICAZIONE));
    lv_obj_align(lbl, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_hidden(pdata->popup_comunication_error, 1);
    lv_obj_set_hidden(pdata->popup_language, 1);
    lv_obj_set_hidden(pdata->popup_alarm, 1);

    update_prog_data(pmodel, pdata);
    update_timer_data(pmodel, pdata);
    update_done_popup(pmodel, pdata);
}


static view_message_t process_page_event(model_t *pmodel, void *arg, view_event_t event) {
    view_message_t    msg   = VIEW_EMPTY_MSG;
    struct page_data *pdata = arg;

    if (!model_is_communication_ok(pmodel) && lv_obj_get_hidden(pdata->popup_comunication_error)) {
        lv_obj_set_hidden(pdata->popup_comunication_error, 0);
    } else if (model_is_communication_ok(pmodel) && !lv_obj_get_hidden(pdata->popup_comunication_error)) {
        lv_obj_set_hidden(pdata->popup_comunication_error, 1);
    }

    switch (event.code) {
        case VIEW_EVENT_CODE_OPEN:
            if (pmodel->prog.parmac.visualizzazione_stop) {
                msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_REBASE;
                msg.vmsg.page = view_main_page(pmodel);
            }
            break;

        case VIEW_EVENT_CODE_PREVIEWS_LOADED:
            update_prog_data(pmodel, pdata);
            break;

        case VIEW_EVENT_CODE_PROGRAM_LOADED:
            msg.cmsg.code  = VIEW_CONTROLLER_COMMAND_CODE_AZZERA_ALLARMI;
            msg.vmsg.code  = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE_EXTRA;
            msg.vmsg.extra = (void *)(uint32_t)model_get_program_num(pmodel);
            msg.vmsg.page  = (void *)view_choice_page(pmodel);
            break;

        case VIEW_EVENT_CODE_MODEL_UPDATE:
            if (!model_macchina_in_stop(pmodel) && model_can_work(pmodel)) {
                msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                msg.vmsg.page = (void *)view_work_page(pmodel);
            } else {
                update_status(pmodel, pdata);
            }

            if (!model_oblo_chiuso(pmodel) && pmodel->run.done) {
                pmodel->run.done = 0;
                update_done_popup(pmodel, pdata);
            }
            break;

        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                pdata->language_ts = get_millis();

                view_common_password_add_key(&pdata->password, event.key_event.code, get_millis());
                if (view_common_check_password(&pdata->password, VIEW_PASSWORD_MINUS, VIEW_SHORT_PASSWORD_LEN,
                                               get_millis())) {
                    msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                    msg.vmsg.page = &page_test_digin;
                    break;
                } else if (view_common_check_password(&pdata->password, VIEW_PASSWORD_RIGHT, VIEW_SHORT_PASSWORD_LEN,
                                                      get_millis())) {
                    msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                    msg.vmsg.page = &page_parmac;
                    break;
                } else if (view_common_check_password(&pdata->password, VIEW_PASSWORD_LEFT, VIEW_SHORT_PASSWORD_LEN,
                                                      get_millis())) {
                    msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                    msg.vmsg.page = &page_programs;
                    break;
                } else if (view_common_check_password(&pdata->password, VIEW_PASSWORD_RESET, VIEW_LONG_PASSWORD_LEN,
                                                      get_millis())) {
                    msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                    msg.vmsg.page = &page_reset_ram;
                    break;
                } else if (view_common_check_password(&pdata->password, VIEW_PASSWORD_TIEPIDO, VIEW_SHORT_PASSWORD_LEN,
                                                      get_millis())) {
                    msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                    msg.vmsg.page = &page_stats;
                    break;
                } else if (view_common_check_password(&pdata->password, VIEW_PASSWORD_SET_DATETIME,
                                                      VIEW_SHORT_PASSWORD_LEN, get_millis())) {
                    msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                    msg.vmsg.page = &page_set_datetime;
                    break;
                } else if (view_common_check_password(&pdata->password, VIEW_PASSWORD_LANA, VIEW_LONG_PASSWORD_LEN,
                                                      get_millis())) {
                    msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                    msg.vmsg.page = &page_communication_settings;
                    break;
                } else if (view_common_check_password(&pdata->password, VIEW_PASSWORD_LOCK, VIEW_SHORT_PASSWORD_LEN,
                                                      get_millis())) {
                    msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                    msg.vmsg.page = &page_removable_drive;
                    break;
                } else if (view_common_check_password_started(&pdata->password)) {
                    if (event.key_event.code != BUTTON_STOP) {
                        break;
                    }
                }

                switch (event.key_event.code) {
                    case BUTTON_KEY:
                        if (model_oblo_serrato(pmodel)) {
                            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_APRI_OBLO;
                        } else if (model_oblo_libero(pmodel) && model_oblo_chiuso(pmodel)) {
                            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_CHIUDI_OBLO;
                        }
                        break;

                    case BUTTON_STOP_PIU: {
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                        msg.vmsg.page = &page_datetime;
                        break;
                    }

                    case BUTTON_STOP_MENO: {
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                        msg.vmsg.page = &page_info;
                        break;
                    }

                    case BUTTON_STOP_START: {
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                        msg.vmsg.page = &page_contrast;
                        break;
                    }

                    case BUTTON_STOP: {
                        if (pmodel->system.errore_comunicazione && pmodel->system.comunicazione_abilitata) {
                            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_RETRY_COMMUNICATION;
                        } else if (!lv_obj_get_hidden(pdata->popup_alarm)) {
                            lv_obj_set_hidden(pdata->popup_alarm, 1);
                            pdata->alarm_ts = get_millis();
                        } else if (!lv_obj_get_hidden(pdata->popup_language)) {
                            lv_obj_set_hidden(pdata->popup_language, 1);
                            update_language_popup(pmodel, pdata);
                            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_RESET_PAGE;
                            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_RELOAD_PREVIEWS;
                        }
                        pmodel->run.done = 0;
                        update_done_popup(pmodel, pdata);
                        break;
                    }

                    case BUTTON_DESTRA: {
                        if (model_get_num_user_programs(pmodel) > 1) {
                            pdata->index = (pdata->index + 1) % model_get_num_user_programs(pmodel);
                            update_prog_data(pmodel, pdata);
                        }
                        break;
                    }


                    case BUTTON_SINISTRA: {
                        if (model_get_num_user_programs(pmodel) > 1) {
                            pdata->index = utils_circular_decrease(pdata->index, model_get_num_user_programs(pmodel));
                            update_prog_data(pmodel, pdata);
                        }
                        break;
                    }

                    case BUTTON_START:
                        if (pdata->index < model_get_num_user_programs(pmodel)) {
                            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_LOAD_PROGRAM;
                            msg.cmsg.num  = pdata->index;
                        }
                        break;

                    case BUTTON_LINGUA:
                        if (lv_obj_get_hidden(pdata->popup_language)) {
                            lv_obj_set_hidden(pdata->popup_language, 0);
                        } else {
                            pmodel->run.lingua = (pmodel->run.lingua + 1) % NUM_LINGUE;
                        }
                        pdata->popup_language_ts = get_millis();
                        update_language_popup(pmodel, pdata);
                        break;

                    case BUTTON_MENO:
                        if (!lv_obj_get_hidden(pdata->popup_language)) {
                            if (pmodel->run.lingua > 0) {
                                pmodel->run.lingua--;
                            } else {
                                pmodel->run.lingua = NUM_LINGUE - 1;
                            }
                            pdata->popup_language_ts = get_millis();
                            update_language_popup(pmodel, pdata);
                        }
                        break;

                    case BUTTON_PIU:
                        if (!lv_obj_get_hidden(pdata->popup_language)) {
                            pmodel->run.lingua       = (pmodel->run.lingua + 1) % NUM_LINGUE;
                            pdata->popup_language_ts = get_millis();
                            update_language_popup(pmodel, pdata);
                        }
                        break;

                    default:
                        break;
                }
            }
            break;
        }

        case VIEW_EVENT_CODE_TIMER:
            switch (event.timer_id) {
                case TIMER_500MS_ID:
                    pdata->counter++;
                    if ((pdata->counter % 4) == 0) {
                        pdata->flag_status = !pdata->flag_status;
                    }
                    if ((pdata->counter % 2) == 0) {
                        pdata->index_info = (pdata->index_info + 1) % 4;
                    }
                    update_timer_data(pmodel, pdata);

                    if (!lv_obj_get_hidden(pdata->popup_language) &&
                        is_expired(pdata->popup_language_ts, get_millis(), LANGUAGE_TIMEOUT)) {
                        lv_obj_set_hidden(pdata->popup_language, 1);
                        update_language_popup(pmodel, pdata);
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_RESET_PAGE;
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_RELOAD_PREVIEWS;
                    } else if (model_get_language(pmodel) != model_get_temporary_language(pmodel) &&
                               is_expired(pdata->language_ts, get_millis(), LANGUAGE_TIMEOUT)) {
                        model_reset_temporary_language(pmodel);
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_RELOAD_PREVIEWS;
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_RESET_PAGE;
                    }
                    break;

                default:
                    break;
            }
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


static void update_prog_data(model_t *pmodel, struct page_data *pdata) {
    if (model_get_num_user_programs(pmodel) == 0) {
        lv_label_set_text(pdata->lbl_name, view_intl_get_string_from_language(model_get_temporary_language(pmodel),
                                                                              STRINGS_NESSUN_PROGRAMMA));
        lv_obj_set_hidden(pdata->img_left, 1);
        lv_obj_set_hidden(pdata->img_right, 1);
        lv_obj_set_hidden(pdata->img_washes_sm, 1);
        lv_obj_set_hidden(pdata->lbl_washes, 1);
        lv_obj_set_hidden(pdata->img_tipo, 1);
        lv_obj_set_hidden(pdata->img_info, 1);
        lv_obj_set_hidden(pdata->lbl_info, 1);
        lv_obj_set_hidden(pdata->img_rpm, 1);
    } else {
        const programma_preview_t *preview = model_get_preview(pmodel, pdata->index);

        lv_label_set_text_fmt(pdata->lbl_name, "%02i-%s", pdata->index + 1, preview->name);
        lv_obj_set_hidden(pdata->img_left, 0);
        lv_obj_set_hidden(pdata->img_right, 0);
        lv_obj_set_hidden(pdata->img_washes_sm, 0);
        lv_obj_set_hidden(pdata->lbl_washes, 0);
        lv_obj_set_hidden(pdata->img_tipo, 0);
        lv_obj_set_hidden(pdata->img_info, 0);
        lv_obj_set_hidden(pdata->lbl_info, 0);
        lv_obj_set_hidden(pdata->img_rpm, 0);
        lv_label_set_text_fmt(pdata->lbl_washes, "x%02i", preview->lavaggi);

        view_common_program_type_image(pdata->img_tipo, model_get_preview(pmodel, pdata->index)->tipo);
    }
}


static void update_timer_data(model_t *pmodel, struct page_data *pdata) {
    struct tm time;
    utils_get_sys_time(&time);
    lv_label_set_text_fmt(pdata->lbl_time, "%02i/%02i/%02i %02i:%02i:%02i", time.tm_mday, time.tm_mon + 1,
                          time.tm_year - 100, time.tm_hour, time.tm_min, time.tm_sec);

    update_status(pmodel, pdata);

    if (model_get_num_user_programs(pmodel) > 0) {
        const programma_preview_t *preview   = model_get_preview(pmodel, pdata->index);
        const GSYMBOL             *symbols[] = {
            &legacy_img_time,
            &legacy_img_temperature,
            &legacy_img_speed,
            &legacy_img_level,
        };
        custom_lv_img_set_src(pdata->img_info, symbols[pdata->index_info]);
        switch (pdata->index_info) {
            case 0:
                lv_label_set_text_fmt(pdata->lbl_info, "%02im%02is", preview->durata / 60, preview->durata % 60);
                lv_obj_set_hidden(pdata->img_rpm, 1);
                break;
            case 1:
                lv_label_set_text_fmt(pdata->lbl_info, "%02i C", preview->temperatura);
                lv_obj_set_hidden(pdata->img_rpm, 1);
                break;
            case 2:
                lv_label_set_text_fmt(pdata->lbl_info, "%02i", preview->velocita);
                lv_obj_set_hidden(pdata->img_rpm, 0);
                break;
            case 3:
                lv_label_set_text_fmt(pdata->lbl_info, "%02i %s", preview->livello,
                                      model_is_level_in_cm(&pmodel->prog.parmac) ? "cm" : "lt");
                lv_obj_set_hidden(pdata->img_rpm, 1);
                break;
            default:
                assert(0);
        }
    }
}


static void update_language_popup(model_t *pmodel, struct page_data *pdata) {
    const strings_t languages[] = {STRINGS_ITALIANO, STRINGS_INGLESE};
    uint16_t        language    = model_get_temporary_language(pmodel);
    lv_label_set_text(pdata->lbl_language, view_intl_get_string_from_language(language, languages[language]));
}


static void update_done_popup(model_t *pmodel, struct page_data *pdata) {
    lv_label_set_text(pdata->lbl_done,
                      view_intl_get_string_from_language(model_get_temporary_language(pmodel), STRINGS_APRI_E_SVUOTA));
    lv_obj_set_hidden(pdata->popup_done, !pmodel->run.done);
}


static void update_status(model_t *pmodel, struct page_data *pdata) {
    if (view_common_update_alarm_popup(pmodel, &pdata->allarme, &pdata->alarm_ts, pdata->popup_alarm, pdata->lbl_alarm,
                                       pdata->lbl_alarm_code)) {
        lv_label_set_text(pdata->lbl_status, view_common_alarm_description(pmodel));
    } else if (model_lavaggio_pagato(pmodel, pdata->index)) {
        lv_label_set_text(pdata->lbl_status,
                          view_intl_get_string_from_language(model_get_temporary_language(pmodel),
                                                             pdata->flag_status ? STRINGS_SCELTA_PROGRAMMA
                                                                                : STRINGS_E_PREMERE_START));
    } else {
        lv_label_set_text(pdata->lbl_status, view_require_payment_string(pmodel, model_get_temporary_language(pmodel)));
    }
}


const pman_page_t page_main = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};
