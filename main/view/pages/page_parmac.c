#include <stdio.h>
#include "view/view.h"
#include "gel/pagemanager/page_manager.h"
#include "model/model.h"
#include "model/parmac.h"
#include "lvgl/lvgl.h"
#include "view/fonts/legacy_fonts.h"
#include "view/common.h"
#include "peripherals/keyboard.h"
#include "view/intl/intl.h"
#include "view/styles.h"
#include "esp_log.h"
#include "app_config.h"
#include "utils/utils.h"
#include "gel/timer/timecheck.h"


struct page_data {
    lv_obj_t *lnum;
    lv_obj_t *ldesc;
    lv_obj_t *lval;

    size_t  parameter;
    size_t  num_parameters;
    uint8_t livello_accesso;
    int     par_to_save;

    int           longpressing;
    unsigned long timestamp;
    int           step;

    lv_task_t *return_task;
};


static view_t update_page(model_t *pmodel, struct page_data *pdata);


static const char *TAG = "PageParmac";


static void *create_page(model_t *model, void *extra) {
    (void)TAG;
    struct page_data *data = malloc(sizeof(struct page_data));
    data->par_to_save      = 0;
    if (model->run.livello_accesso_temporaneo > 0) {
        data->livello_accesso                 = model->run.livello_accesso_temporaneo;
        model->run.livello_accesso_temporaneo = 0;
    } else {
        data->livello_accesso = model->prog.parmac.livello_accesso;
    }
    data->return_task = view_register_periodic_task(PAGE_TIMEOUT, LV_TASK_PRIO_OFF, 0);
    return data;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *data = args;

    data->longpressing = 0;
    data->step         = 1;

    view_common_title(lv_scr_act(), view_intl_get_string(pmodel, STRINGS_PARAMETRI_MAC));

    lv_task_set_prio(data->return_task, LV_TASK_PRIO_MID);
    lv_task_reset(data->return_task);

    data->num_parameters = parmac_get_tot_parameters(data->livello_accesso);
    data->parameter      = 0;

    lv_obj_t *lnum = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lnum, 1);
    lv_obj_align(lnum, NULL, LV_ALIGN_IN_TOP_MID, 0, 15);
    data->lnum = lnum;

    lv_obj_t *ldesc = lv_label_create(lv_scr_act(), lnum);
    lv_obj_set_style(ldesc, &style_label_6x8);
    lv_obj_align(ldesc, lnum, LV_ALIGN_OUT_BOTTOM_MID, 0, 1);
    lv_label_set_align(ldesc, LV_LABEL_ALIGN_CENTER);
    lv_label_set_long_mode(ldesc, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(ldesc, 128);
    data->ldesc = ldesc;

    lv_obj_t *line = view_common_horizontal_line();
    lv_obj_align(line, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -19);

    lv_obj_t *lval = lv_label_create(lv_scr_act(), lnum);
    lv_label_set_align(lval, LV_LABEL_ALIGN_CENTER);
    lv_label_set_long_mode(lval, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(lval, 128);
    lv_obj_align(lval, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -1);
    data->lval = lval;

    update_page(pmodel, data);
}


static view_message_t process_page_event(model_t *model, void *args, pman_event_t event) {
    struct page_data *data = args;
    view_message_t    msg  = VIEW_EMPTY_MSG;

    switch (event.code) {
        case VIEW_EVENT_CODE_TIMER:
            model_reset_temporary_language(model);
            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_SAVE_PARMAC;
            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
            break;

        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK || event.key_event.event == KEY_LONGPRESS) {
                lv_task_reset(data->return_task);

                if (event.key_event.event == KEY_LONGPRESS && !data->longpressing) {
                    data->longpressing = 1;
                    data->timestamp    = get_millis();
                }

                if (data->longpressing && is_expired(data->timestamp, get_millis(), LONG_PRESS_INCREASE)) {
                    data->step = 10;
                } else if (!data->longpressing) {
                    data->step = 1;
                }

                switch (event.key_event.code) {
                    case BUTTON_SINISTRA:
                        if (data->parameter > 0) {
                            data->parameter--;
                        } else {
                            data->parameter = data->num_parameters - 1;
                        }
                        update_page(model, data);
                        break;

                    case BUTTON_DESTRA:
                        data->parameter = (data->parameter + 1) % data->num_parameters;
                        update_page(model, data);
                        break;

                    case BUTTON_MENO:
                        parmac_operation(model, data->parameter, -data->step, data->livello_accesso);
                        update_page(model, data);
                        data->par_to_save = 1;
                        break;

                    case BUTTON_PIU:
                        parmac_operation(model, data->parameter, +data->step, data->livello_accesso);
                        update_page(model, data);
                        data->par_to_save = 1;
                        break;

                    case BUTTON_STOP:
                        model_reset_temporary_language(model);
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_SAVE_PARMAC;
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
                        break;

                    case BUTTON_MENU:
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                        msg.vmsg.page = &page_machine_name;
                        break;
                }
            } else if (event.key_event.event == KEY_RELEASE) {
                data->longpressing = 0;
            }
            break;
        }

        default:
            break;
    }

    return msg;
}


static view_t update_page(model_t *pmodel, struct page_data *pdata) {
    char string[64] = {0};

    lv_label_set_text_fmt(pdata->lnum, "Param. %2i/%i", pdata->parameter + 1, pdata->num_parameters);
    lv_label_set_text(pdata->ldesc, parmac_get_description(pmodel, pdata->parameter, pdata->livello_accesso));
    parmac_format_value(pmodel, string, pdata->parameter, pdata->livello_accesso);
    lv_label_set_text(pdata->lval, string);
    return 0;
}


static void close_page(void *arg) {
    struct page_data *pdata = arg;
    lv_task_set_prio(pdata->return_task, LV_TASK_PRIO_OFF);
    lv_obj_clean(lv_scr_act());
}


static void destroy_page(void *arg, void *extra) {
    struct page_data *pdata = arg;
    lv_task_del(pdata->return_task);
    free(pdata);
    free(extra);
}


const pman_page_t page_parmac = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};
