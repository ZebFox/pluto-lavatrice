#include <stdio.h>
#include "view/view.h"
#include "gel/pagemanager/page_manager.h"
#include "model/model.h"
#include "model/parmac.h"
#include "lvgl/lvgl.h"
#include "view/fonts/legacy_fonts.h"
#include "view/common.h"
#include "peripherals/keyboard.h"

struct page_data {
    lv_obj_t *lnum;
    lv_obj_t *ldesc;
    lv_obj_t *lval;

    size_t parameter;
    size_t num_parameters;
    size_t livello_accesso;
    int    par_to_save;
};


static void *create_page(model_t *model, void *extra) {
    struct page_data *data = malloc(sizeof(struct page_data));
    data->par_to_save      = 0;
    data->livello_accesso  = LVL_COSTRUTTORE;
    return data;
}


static void open_page(model_t *model, void *args) {
    struct page_data *data = args;
    view_common_title(lv_scr_act(), "PARAMETRI MAC.");

    data->num_parameters = parmac_get_tot_parameters(data->livello_accesso);
    data->parameter      = 0;

    lv_obj_t *lnum = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lnum, 1);
    lv_obj_align(lnum, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);
    data->lnum = lnum;

    lv_obj_t *ldesc = lv_label_create(lv_scr_act(), lnum);
    lv_obj_align(ldesc, lnum, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
    data->ldesc = ldesc;

    lv_obj_t *lval = lv_label_create(lv_scr_act(), lnum);
    lv_obj_align(lval, ldesc, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
    data->lval = lval;
}


static view_message_t process_page_event(model_t *model, void *args, pman_event_t event) {
    struct page_data *data = args;
    view_message_t    msg  = VIEW_EMPTY_MSG;

    switch (event.code) {
        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                switch (event.key_event.code) {
                    case BUTTON_DESTRA:
                        if (data->parameter > 0) {
                            data->parameter--;
                        } else {
                            data->parameter = data->num_parameters - 1;
                        }
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_UPDATE;
                        break;

                    case BUTTON_CALDO:
                        data->parameter = (data->parameter + 1) % data->num_parameters;
                        msg.vmsg.code   = VIEW_PAGE_COMMAND_CODE_UPDATE;
                        break;

                    case BUTTON_FREDDO:
                        parmac_operation(model, data->parameter, -1, data->livello_accesso);
                        msg.vmsg.code     = VIEW_PAGE_COMMAND_CODE_UPDATE;
                        data->par_to_save = 1;
                        break;

                    case BUTTON_PIU:
                        parmac_operation(model, data->parameter, +1, data->livello_accesso);
                        msg.vmsg.code     = VIEW_PAGE_COMMAND_CODE_UPDATE;
                        data->par_to_save = 1;
                        break;

                    case BUTTON_STOP:
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
#warning "Rimuovere i parametri estesi in uscita dalla pagina!"
                        // model->pmac.abilita_parametri_ridotti = 1;
                        if (data->par_to_save) {
                            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                        }
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
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

static view_t update_page(model_t *pmodel, void *args) {
    struct page_data *data       = args;
    char              string[64] = {0};

    lv_label_set_text_fmt(data->lnum, "Param. %2i/%i", data->parameter + 1, data->num_parameters);
    lv_label_set_text(data->ldesc, parmac_get_description(pmodel, data->parameter, data->livello_accesso));
    parmac_format_value(pmodel, string, data->parameter, data->livello_accesso);
    lv_label_set_text(data->lval, string);
    return 0;
}


const pman_page_t page_parmac = {
    .create        = create_page,
    .open          = open_page,
    .update        = update_page,
    .process_event = process_page_event,
    .close         = view_close_all,
    .destroy       = view_destroy_all,
};