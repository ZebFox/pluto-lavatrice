#include <stdio.h>
#include "src/lv_core/lv_obj.h"
#include "src/lv_misc/lv_task.h"
#include "src/lv_objx/lv_label.h"
#include "view/view.h"
#include "gel/pagemanager/page_manager.h"
#include "model/model.h"
#include "model/parmac.h"
#include "lvgl/lvgl.h"
#include "view/fonts/legacy_fonts.h"
#include "view/common.h"
#include "peripherals/keyboard.h"
#include "controller/controller.h"
#include "utils/utils.h"
#include "view/intl/intl.h"

static char *get_string(int flag);

struct page_data {
    lv_obj_t *lerror;
    lv_obj_t *lenable;
    lv_obj_t *cursore_up;
    lv_obj_t *cursore_down;
};



static void *create_page(model_t *model, void *extra) {
    struct page_data *data = malloc(sizeof(struct page_data));
    return data;
}


static void open_page(model_t *model, void *args) {
    struct page_data *data = args;
    // stringa
    view_common_title(lv_scr_act(), "COMUNICATION");


    lv_obj_t *title_error = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(title_error, "ERRORE:");
    lv_obj_align(title_error, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 20);

    lv_obj_t *title_enable = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(title_enable, "ABILITATA:");
    lv_obj_align(title_enable, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 30);

    lv_obj_t *cursore = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(cursore, ">");
    lv_obj_align(cursore, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 20);
    data->cursore_up = cursore;

    cursore = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(cursore, ">");
    lv_obj_align(cursore, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 30);
    lv_obj_set_hidden(cursore, 1);
    data->cursore_down = cursore;

    lv_obj_t *lerror = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lerror, 1);
    lv_obj_align(lerror, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 20);
    data->lerror = lerror;
    lv_label_set_text(lerror, get_string(model->system.errore_comunicazione));

    lv_obj_t *lenable = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lenable, 1);
    lv_obj_align(lenable, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 30);
    data->lenable = lenable;
    lv_label_set_text(lenable, get_string(model->system.comunicazione_abilitata));
}


static view_message_t process_page_event(model_t *model, void *args, pman_event_t event) {
    struct page_data *data = args;
    (void)data;
    view_message_t msg = VIEW_EMPTY_MSG;

    switch (event.code) {
        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                switch (event.key_event.code) {
                    case BUTTON_STOP: {
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
                        break;
                    }
                    case BUTTON_SINISTRA: {
                        if (lv_obj_get_hidden(data->cursore_up)) {
                            lv_obj_set_hidden(data->cursore_up, 0);
                            lv_obj_set_hidden(data->cursore_down, 1);
                        } else if (lv_obj_get_hidden(data->cursore_down)) {
                            lv_obj_set_hidden(data->cursore_up, 1);
                            lv_obj_set_hidden(data->cursore_down, 0);
                        }
                        break;
                    }
                    case BUTTON_DESTRA: {
                        if (lv_obj_get_hidden(data->cursore_up)) {
                            lv_obj_set_hidden(data->cursore_up, 0);
                            lv_obj_set_hidden(data->cursore_down, 1);
                        } else if (lv_obj_get_hidden(data->cursore_down)) {
                            lv_obj_set_hidden(data->cursore_up, 1);
                            lv_obj_set_hidden(data->cursore_down, 0);
                        }
                        break;
                    }
                    case BUTTON_LINGUA: {
                        if (lv_obj_get_hidden(data->cursore_down)) {
                            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_RETRY_COMMUNICATION;
                            break;
                        } else if (lv_obj_get_hidden(data->cursore_up)) {
                            model->system.comunicazione_abilitata = !model->system.comunicazione_abilitata;
                            msg.cmsg.code                         = VIEW_CONTROLLER_COMMAND_CODE_CHANGE_AB_COMMUNICATION;
                            msg.cmsg.value                        = model->system.comunicazione_abilitata;
                            lv_label_set_text(data->lenable, get_string(model->system.comunicazione_abilitata));
                            
                            break;
                        }
                    }
                    default:
                        break;
                }
            }
            break;
        }

        case VIEW_EVENT_CODE_MODEL_UPDATE:
            lv_label_set_text(data->lerror, get_string(model->system.errore_comunicazione));
            lv_label_set_text(data->lenable, get_string(model->system.comunicazione_abilitata));



        default:
            break;
    }

    return msg;
}

static view_t update_page(model_t *pmodel, void *args) {
    return 0;
}





const pman_page_t page_communication_settings = {
    .create        = create_page,
    .open          = open_page,
    .update        = update_page,
    .process_event = process_page_event,
    .close         = view_close_all,
    .destroy       = view_destroy_all,
};


static char *get_string(int flag) {
    if (flag) {
        return "SI";
    } else {
        return "NO";
    }
}