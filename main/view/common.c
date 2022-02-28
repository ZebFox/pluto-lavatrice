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


static void timer_task(lv_task_t *task);
static int  find_password_start(view_common_password_t *password);


static const button_t preamble[3] = {BUTTON_STOP, BUTTON_STOP, BUTTON_STOP};


const char *view_common_alarm_description(model_t *pmodel) {
    static char codice_generico[32] = {0};
    uint16_t    alarm_code          = pmodel->run.macchina.codice_allarme;

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

        default:
            snprintf(codice_generico, sizeof(codice_generico), "%s %i", view_intl_get_string(pmodel, STRINGS_ALLARME),
                     alarm_code);
            return codice_generico;
    }
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

lv_obj_t *view_common_popup(lv_obj_t *root, const char *str) {
    lv_obj_t *cont = lv_cont_create(root, NULL);
    lv_obj_set_size(cont, 110, 50);
    lv_obj_align(cont, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style(cont, &style_container_bordered);
    lv_obj_t *cont_in = lv_cont_create(cont, NULL);
    lv_obj_set_size(cont_in, 105, 45);
    lv_obj_align(cont_in, cont, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style(cont_in, &style_container_bordered);

    lv_obj_t *label = lv_label_create(cont, NULL);
    lv_label_set_text(label, str);
    lv_obj_align(label, cont, LV_ALIGN_CENTER, 0, 0);

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

    return 1;
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