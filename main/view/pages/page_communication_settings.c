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


#define OPTIONS 3


struct page_data {
    lv_obj_t *lerror;
    lv_obj_t *lenable;
    lv_obj_t *lbl_debug;

    uint8_t debug_code;
    size_t  index;
};


static const char *get_string(model_t *pmodel, int flag);
static view_t      update_page(model_t *pmodel, struct page_data *pdata);


static void *create_page(model_t *model, void *extra) {
    struct page_data *data = malloc(sizeof(struct page_data));
    data->index            = 0;
    data->debug_code       = 0;
    return data;
}


static void open_page(model_t *model, void *args) {
    struct page_data *data = args;
    // stringa
    view_common_title(lv_scr_act(), "COMUNICATION");


    lv_obj_t *lerror = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lerror, 1);
    lv_obj_align(lerror, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 18);
    data->lerror = lerror;

    lv_obj_t *lenable = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lenable, 1);
    lv_obj_align(lenable, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 28);
    data->lenable = lenable;

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 38);
    data->lbl_debug = lbl;

    update_page(model, data);
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
                        if (data->index > 0) {
                            data->index--;
                        } else {
                            data->index = OPTIONS - 1;
                        }
                        update_page(model, data);
                        break;
                    }

                    case BUTTON_DESTRA: {
                        data->index = (data->index + 1) % OPTIONS;
                        update_page(model, data);
                        break;
                    }

                    case BUTTON_LINGUA: {
                        if (data->index == 0) {
                            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_RETRY_COMMUNICATION;
                        } else if (data->index == 1) {
                            model->system.comunicazione_abilitata = !model->system.comunicazione_abilitata;
                            msg.cmsg.code  = VIEW_CONTROLLER_COMMAND_CODE_CHANGE_AB_COMMUNICATION;
                            msg.cmsg.value = model->system.comunicazione_abilitata;
                            update_page(model, data);
                        } else if (data->index == 2) {
                            msg.cmsg.code  = VIEW_CONTROLLER_COMMAND_CODE_SEND_DEBUG_CODE;
                            msg.cmsg.value = data->debug_code;
                        }
                        break;
                    }

                    case BUTTON_PIU:
                        if (data->index == 2) {
                            data->debug_code++;
                            update_page(model, data);
                        }
                        break;

                    case BUTTON_MENO:
                        if (data->index == 2) {
                            data->debug_code--;
                            update_page(model, data);
                        }
                        break;

                    default:
                        break;
                }
            } else if (event.key_event.event == KEY_LONGPRESS) {
                switch (event.key_event.code) {
                    case BUTTON_PIU:
                        if (data->index == 2) {
                            data->debug_code++;
                            update_page(model, data);
                        }
                        break;

                    case BUTTON_MENO:
                        if (data->index == 2) {
                            data->debug_code--;
                            update_page(model, data);
                        }
                        break;

                    default:
                        break;
                }
            }
            break;
        }

        case VIEW_EVENT_CODE_MODEL_UPDATE:
            update_page(model, data);
            break;

        default:
            break;
    }

    return msg;
}

static view_t update_page(model_t *pmodel, struct page_data *pdata) {
    lv_label_set_text_fmt(pdata->lerror, "%c%-10s: %s", VIEW_COMMON_CURSOR(pdata->index, 0),
                          view_intl_get_string(pmodel, STRINGS_ERRORE),
                          get_string(pmodel, pmodel->system.errore_comunicazione));
    lv_label_set_text_fmt(pdata->lenable, "%c%-10s: %s", VIEW_COMMON_CURSOR(pdata->index, 1),
                          view_intl_get_string(pmodel, STRINGS_ABILITATA),
                          get_string(pmodel, pmodel->system.comunicazione_abilitata));

    lv_label_set_text_fmt(pdata->lbl_debug, "%c%-10s: %i", VIEW_COMMON_CURSOR(pdata->index, 2),
                          view_intl_get_string(pmodel, STRINGS_DEBUG), pdata->debug_code);
    return 0;
}





const pman_page_t page_communication_settings = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = view_close_all,
    .destroy       = view_destroy_all,
};


static const char *get_string(model_t *pmodel, int flag) {
    if (flag) {
        return view_intl_get_string(pmodel, STRINGS_SI);
    } else {
        return view_intl_get_string(pmodel, STRINGS_NO);
    }
}