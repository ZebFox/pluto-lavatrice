#include <stdio.h>
#include <assert.h>
#include "common.h"
#include "gel/timer/timecheck.h"
#include "src/lv_objx/lv_label.h"
#include "styles.h"
#include "model/programs.h"
#include "intl/intl.h"
#include "images/legacy.h"
#include "widgets/custom_lv_img.h"
#include "utils/utils.h"
#include "config/app_config.h"


static void timer_task(lv_task_t *task);
static int  find_password_start(view_common_password_t *password);


static const button_t preamble[3] = {BUTTON_STOP, BUTTON_STOP, BUTTON_STOP};


const char *view_common_alarm_description(model_t *pmodel) {
    static char codice_generico[32] = {0};
    uint16_t    alarm_code          = model_alarm_code(pmodel);

    switch (alarm_code) {
        case 1:
            return view_intl_get_string(pmodel, STRINGS_ERRORE_EEPROM);
        case 2:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_SPEGNIMENTO);
        case 3:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_COMUNICAZIONE);
        case 4:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_OBLO_APERTO);
        case 5:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_OBLO_SBLOCCATO);
        case 6 ... 7:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_EMERGENZA);
        case 8:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_INVERTER);
        case 9:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_TEMPERATURA_1);
        case 10 ... 11:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_SBILANCIAMENTO);
        case 12:
        case 14:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_CHIAVISTELLO_BLOCCATO);
        case 13:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_SERRATURA);
        case 15:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_APERTO_H2O);
        case 16:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_SERRATURA_FORZATA);
        case 17:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_ACCELEROMETRO);
        case 18:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_ACCELEROMETRO_FUORI_SCALA);
        case 19:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_VELOCITA);
        case 20:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_NO_MOTO);
        case 21:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_NO_FERMO);
        case 22:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_TEMPERATURA);
        case 23:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_H2O_IN_VASCA);
        case 24:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_LIVELLO);
        case 30:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_RIEMPIMENTO);
        case 31:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_RISCALDAMENTO);
        case 32:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_SCARICO);
        case 50:
            return view_intl_get_string(pmodel, STRINGS_MACCHINA_ACCESA);
        case 51:
            return view_intl_get_string(pmodel, STRINGS_ALLARME_SPEGNIMENTO);
        case 52:
            return view_intl_get_string(pmodel, STRINGS_INIZIO_LAVAGGIO);
        case 53:
            return view_intl_get_string(pmodel, STRINGS_FINE_LAVAGGIO);
        case 54:
            return view_intl_get_string(pmodel, STRINGS_LAVAGGIO_INTERROTTO);
        case 60:
            return view_intl_get_string(pmodel, STRINGS_APRIRE_OBLO);
        case 70:
            return view_intl_get_string(pmodel, STRINGS_STANDBY_SAPONI);

        default:
            snprintf(codice_generico, sizeof(codice_generico), "%s %i", view_intl_get_string(pmodel, STRINGS_ALLARME),
                     alarm_code);
            return codice_generico;
    }
}


void view_common_program_type_name(model_t *pmodel, lv_obj_t *lbl, uint8_t type) {
    const strings_t labels[] = {
        STRINGS_MOLTO_SPORCHI_CON_PRELAVAGGIO,
        STRINGS_SPORCHI_CON_PRELAVAGGIO,
        STRINGS_MOLTO_SPORCHI,
        STRINGS_SPORCHI,
        STRINGS_COLORATI,
        STRINGS_SINTETICI,
        STRINGS_PIUMONI,
        STRINGS_DELICATI,
        STRINGS_LANA,
        STRINGS_LINO_E_TENDAGGI,
        STRINGS_CENTRIFUGA,
        STRINGS_CENTRIFUGA_PER_DELICATI,
        STRINGS_SANIFICAZIONE,
        STRINGS_AMMOLLO,
        STRINGS_PRELAVAGGIO,
        STRINGS_RISCIACQUO,
    };

    lv_label_set_text(lbl, view_intl_get_string(pmodel, labels[type]));
}


void view_common_program_type_image(lv_obj_t *img, uint8_t ptype) {
    const GSYMBOL *images[] = {
        &legacy_img_molto_sporchi, &legacy_img_sporchi,         &legacy_img_molto_sporchi,
        &legacy_img_sporchi,       &legacy_img_colorati,        &legacy_img_sintetici,
        &legacy_img_piumoni,       &legacy_img_delicati,        &legacy_img_lana,
        &legacy_img_lino_tendaggi, &legacy_img_solo_centrifuga, &legacy_img_solo_centrifuga_delicati,
        &legacy_img_sanificazione, &legacy_img_ammollo,         &legacy_img_prelavaggio,
        &legacy_img_risciacquo,
    };
    assert(ptype < sizeof(images) / sizeof(images[0]));
    custom_lv_img_set_src(img, images[ptype]);
}


const char *view_common_step2str(model_t *pmodel, uint16_t step) {
    const strings_t step2str[NUM_STEPS] = {
        STRINGS_AMMOLLO, STRINGS_PRELAVAGGIO, STRINGS_LAVAGGIO,     STRINGS_RISCIACQUO,
        STRINGS_SCARICO, STRINGS_CENTRIFUGA,  STRINGS_SROTOLAMENTO, STRINGS_ATTESA_OPERATORE,
    };

    assert(step <= NUM_STEPS && step > 0);
    return view_intl_get_string(pmodel, step2str[step - 1]);
}


const char *view_common_pedantic_string(model_t *pmodel) {
    const strings_t pedantic2str[] = {
        STRINGS_,
        STRINGS_PRECARICA_IN_CORSO,
        STRINGS_ATTESA_LIVELLO_E_TEMPERATURA,
        STRINGS_ATTESA_LIVELLO,
        STRINGS_ATTESA_TEMPERATURA,
        STRINGS_RIEMPIMENTO,
        STRINGS_ATTESA_TERMODEGRADAZIONE,
        STRINGS_ATTESA_LIVELLO_SCARICO,
        STRINGS_PRESCARICO,
        STRINGS_PREPARAZIONE,
        STRINGS_RAGGIUNGIMENTO_VELOCITA,
        STRINGS_IN_FRENATA,
        STRINGS_ATTESA_FRENATA,
        STRINGS_SCARICO_FORZATO,
        STRINGS_RECUPERO,
        STRINGS_USCITA_LAVAGGIO,
    };

    if (pmodel->run.macchina.descrizione_pedante >= sizeof(pedantic2str) / sizeof(pedantic2str[0])) {
        return "";
    } else {
        return view_intl_get_string(pmodel, pedantic2str[pmodel->run.macchina.descrizione_pedante]);
    }
}


lv_obj_t *view_common_title(lv_obj_t *root, const char *str) {
    lv_obj_t *cont = lv_cont_create(root, NULL);

    static lv_style_t style;
    lv_style_copy(&style, lv_obj_get_style(cont));
    style.body.border.part    = LV_BORDER_BOTTOM | LV_BORDER_TOP;
    style.body.border.width   = 1;
    style.body.border.color   = LV_COLOR_BLACK;
    style.body.padding.inner  = 0;
    style.body.padding.top    = 2;
    style.body.padding.bottom = 2;
    style.text.letter_space   = 0;
    lv_obj_set_style(cont, &style);

    lv_obj_t *title = lv_label_create(cont, NULL);
    lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
    lv_label_set_long_mode(title, LV_LABEL_LONG_CROP);
    lv_label_set_text(title, str);
    lv_obj_set_width(title, LV_HOR_RES_MAX);

    lv_cont_set_fit2(cont, LV_FIT_NONE, LV_FIT_TIGHT);
    lv_obj_set_width(cont, LV_HOR_RES_MAX);
    lv_obj_align(cont, NULL, LV_ALIGN_IN_TOP_MID, 0, 2);
    lv_obj_align(title, NULL, LV_ALIGN_CENTER, 0, 0);

    return title;
}


lv_obj_t *view_common_popup(lv_obj_t *root, lv_obj_t **content) {
    lv_obj_t *cont = lv_cont_create(root, NULL);
    lv_obj_set_size(cont, 116, 50);
    lv_obj_align(cont, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style(cont, &style_container_bordered);
    lv_obj_t *cont_in = lv_cont_create(cont, NULL);
    lv_obj_set_size(cont_in, 112, 46);
    lv_obj_align(cont_in, cont, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style(cont_in, &style_container_bordered);

    if (content != NULL) {
        *content = cont_in;
    }

    return cont;
}


void view_common_password_add_key(view_common_password_t *inserted, button_t new, unsigned long timestamp) {
    if (is_expired(inserted->last_timestamp, timestamp, VIEW_PASSWORD_TIMEOUT)) {
        view_common_password_reset(inserted, timestamp);
    }
    inserted->password[inserted->index] = new;
    inserted->index                     = (inserted->index + 1) % VIEW_PASSWORD_MAX_SIZE;
    inserted->last_timestamp            = timestamp;
}


void view_common_password_reset(view_common_password_t *inserted, unsigned long timestamp) {
    size_t i = 0;
    for (i = 0; i < VIEW_PASSWORD_MAX_SIZE; i++) {
        inserted->password[i] = BUTTON_NONE;
    }
    inserted->index          = 0;
    inserted->last_timestamp = timestamp;
}


int view_common_check_password(view_common_password_t *inserted, button_t *password, size_t length,
                               unsigned long timestamp) {
    if (is_expired(inserted->last_timestamp, timestamp, VIEW_PASSWORD_TIMEOUT)) {
        view_common_password_reset(inserted, timestamp);
        return 0;
    } else if (length + sizeof(preamble) / sizeof(preamble[0]) > VIEW_PASSWORD_MAX_SIZE) {
        return 0;
    }


    size_t i = 0;

    int res = find_password_start(inserted);
    if (res < 0) {
        return 0;
    }

    size_t start = (size_t)res;
    for (i = 0; i < length; i++) {
        if (inserted->password[(start + i) % VIEW_PASSWORD_MAX_SIZE] != password[i]) {
            return 0;
        }
    }

    return inserted->index == (start + length) % (VIEW_PASSWORD_MAX_SIZE);
}


int view_common_check_password_started(view_common_password_t *inserted) {
    int res = find_password_start(inserted);
    return res >= 0;
}


lv_task_t *view_common_register_timer(unsigned long period) {
    return lv_task_create(timer_task, period, LV_TASK_PRIO_OFF, NULL);
}


lv_obj_t *view_common_line(lv_point_t *points, size_t len) {
    /*Create new style (thick dark blue)*/
    static lv_style_t style_line;
    lv_style_copy(&style_line, &lv_style_plain);
    style_line.line.color   = LV_COLOR_BLACK;
    style_line.line.width   = 1;
    style_line.line.rounded = 0;

    /*Copy the previous line and apply the new style*/
    lv_obj_t *line1;
    line1 = lv_line_create(lv_scr_act(), NULL);
    lv_line_set_points(line1, points, len); /*Set the points*/
    lv_line_set_style(line1, LV_LINE_STYLE_MAIN, &style_line);
    return line1;
}


lv_obj_t *view_common_horizontal_line(void) {
    /*Create an array for the points of the line*/
    static lv_point_t line_points[] = {{0, 0}, {128, 0}};
    return view_common_line(line_points, 2);
}


void view_common_set_hidden(lv_obj_t *obj, int hidden) {
    if (lv_obj_get_hidden(obj) != hidden) {
        lv_obj_set_hidden(obj, hidden);
    }
}


lv_obj_t *view_common_braking_popup(lv_obj_t **label, uint16_t language) {
    lv_obj_t *cont;
    lv_obj_t *popup = view_common_popup(lv_scr_act(), &cont);

    lv_obj_t *lbl = lv_label_create(cont, NULL);
    lv_label_set_text(lbl, view_intl_get_string_from_language(language, STRINGS_IN_FRENATA));
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_MID, 0, -4);

    lbl = lv_label_create(cont, NULL);
    lv_obj_align(lbl, NULL, LV_ALIGN_CENTER, 0, -4);
    lv_obj_set_auto_realign(lbl, 1);
    *label = lbl;

    return popup;
}


lv_obj_t *view_common_alarm_popup(lv_obj_t **label, lv_obj_t **code) {
    lv_obj_t *alarm_cont;
    lv_obj_t *popup_alarm = view_common_popup(lv_scr_act(), &alarm_cont);

    lv_obj_t *img = custom_lv_img_create(alarm_cont, NULL);
    custom_lv_img_set_src(img, &legacy_img_alt);
    lv_obj_align(img, NULL, LV_ALIGN_CENTER, -20, -12);

    img = custom_lv_img_create(alarm_cont, NULL);
    custom_lv_img_set_src(img, &legacy_img_alt);
    lv_obj_align(img, NULL, LV_ALIGN_CENTER, 20, -12);

    lv_obj_t *lbl = lv_label_create(alarm_cont, NULL);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_align(lbl, NULL, LV_ALIGN_CENTER, 0, -10);
    *code = lbl;

    lbl = lv_label_create(alarm_cont, NULL);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_auto_realign(lbl, 1);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(lbl, 100);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_align(lbl, NULL, LV_ALIGN_CENTER, 0, 10);
    *label = lbl;

    return popup_alarm;
}


int view_common_update_alarm_popup(model_t *pmodel, uint16_t *alarm, unsigned long *timestamp, lv_obj_t *popup,
                                   lv_obj_t *label, lv_obj_t *lbl_code) {
    if (model_alarm_code(pmodel) > 0) {
        if (*alarm != model_alarm_code(pmodel)) {
            *alarm = model_alarm_code(pmodel);
            lv_label_set_text(label, view_common_alarm_description(pmodel));
            lv_label_set_text_fmt(lbl_code, "%02i", model_alarm_code(pmodel));
            lv_obj_set_hidden(popup, 0);
            *timestamp = get_millis();
            return 1;
        }
        if (lv_obj_get_hidden(popup) && is_expired(*timestamp, get_millis(), ALARM_TIMEOUT)) {
            lv_obj_set_hidden(popup, 0);
            *timestamp = get_millis();
        }
    } else {
        lv_obj_set_hidden(popup, 1);
    }

    return 0;
}


static void timer_task(lv_task_t *task) {
    (void)task;
    view_event((view_event_t){.code = VIEW_EVENT_CODE_TIMER});
}


static int find_password_start(view_common_password_t *password) {
    int    result = -1;
    size_t i = 0, start = 0;

    for (start = 0; start < VIEW_PASSWORD_MAX_SIZE; start++) {
        int found = 1;

        for (i = 0; i < sizeof(preamble) / sizeof(preamble[0]); i++) {
            if (password->password[(start + i) % VIEW_PASSWORD_MAX_SIZE] != preamble[i]) {
                found = 0;
                break;
            }
        }

        if (found) {
            result = start + 3;
            // Do not break this cycle; we consider the latest possible password
        }
    }

    return result;
}