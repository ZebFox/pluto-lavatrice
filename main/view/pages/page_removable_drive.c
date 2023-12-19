#include <stdio.h>
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
#include "view/styles.h"


#define NUM_LINES 4


typedef enum {
    ARCHIVE_OPERATION_NONE,
    ARCHIVE_OPERATION_IN_PROGRESS,
    ARCHIVE_OPERATION_SUCCESS,
    ARCHIVE_OPERATION_FAILURE,
} archive_operation_t;


struct page_data {
    windowed_list_t     list;
    lv_obj_t           *lines[NUM_LINES];
    lv_obj_t           *lbl_info;
    archive_operation_t import_operation;
    archive_operation_t export_operation;
};


static view_t update_page(model_t *pmodel, struct page_data *pdata);


static void *create_page(model_t *model, void *extra) {
    struct page_data *data = malloc(sizeof(struct page_data));
    return data;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    pdata->list             = WINDOWED_LIST(NUM_LINES);
    pdata->import_operation = ARCHIVE_OPERATION_NONE;
    pdata->export_operation = ARCHIVE_OPERATION_NONE;

    view_common_title(lv_scr_act(), view_intl_get_string(pmodel, STRINGS_ARCHIVIAZIONE));

    for (size_t i = 0; i < NUM_LINES; i++) {
        lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
        lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 2, 16 + 9 * i);
        lv_obj_set_style(lbl, &style_label_6x8);
        pdata->lines[i] = lbl;
    }

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_SROLL_CIRC);
    lv_obj_set_width(lbl, 128);
    // lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    pdata->lbl_info = lbl;

    update_page(pmodel, pdata);
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
                        if (data->import_operation != ARCHIVE_OPERATION_IN_PROGRESS &&
                            data->export_operation != ARCHIVE_OPERATION_IN_PROGRESS) {
                            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
                        }
                        break;
                    }

                    case BUTTON_SINISTRA: {
                        break;
                    }

                    case BUTTON_DESTRA: {
                        break;
                    }

                    case BUTTON_KEY: {
                        if (model->system.removable_drive_state == REMOVABLE_DRIVE_STATE_MOUNTED &&
                            data->export_operation == ARCHIVE_OPERATION_NONE) {
                            data->export_operation = ARCHIVE_OPERATION_IN_PROGRESS;
                            msg.cmsg.code          = VIEW_CONTROLLER_COMMAND_CODE_SAVE_ARCHIVE;
                            update_page(model, data);
                        }
                        break;
                    }

                    case BUTTON_LINGUA: {
                        if (model->system.removable_drive_state == REMOVABLE_DRIVE_STATE_MOUNTED &&
                            data->import_operation == ARCHIVE_OPERATION_NONE) {
                            strcpy(msg.cmsg.archive_name, model->system.archivi[data->list.index]);
                            data->import_operation = ARCHIVE_OPERATION_IN_PROGRESS;
                            msg.cmsg.code          = VIEW_CONTROLLER_COMMAND_CODE_LOAD_ARCHIVE;
                            update_page(model, data);
                        }
                        break;
                    }

                    case BUTTON_PIU:
                        windowed_list_next(&data->list, model->system.num_archivi);
                        update_page(model, data);
                        break;

                    case BUTTON_MENO:
                        windowed_list_prev(&data->list, model->system.num_archivi);
                        update_page(model, data);
                        break;

                    case BUTTON_MENU:
                        if (model->system.removable_drive_state == REMOVABLE_DRIVE_STATE_INVALID) {
                            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_RESET;
                        }
                        break;

                    default:
                        break;
                }
            } else if (event.key_event.event == KEY_LONGPRESS) {
            }
            break;
        }

        case VIEW_EVENT_CODE_CONFIGURATION_LOADED:
            if (event.error) {
                data->import_operation = ARCHIVE_OPERATION_FAILURE;
            } else {
                data->import_operation = ARCHIVE_OPERATION_SUCCESS;
            }
            update_page(model, data);
            break;

        case VIEW_EVENT_CODE_CONFIGURATION_SAVED:
            if (event.error) {
                data->export_operation = ARCHIVE_OPERATION_FAILURE;
            } else {
                data->export_operation = ARCHIVE_OPERATION_SUCCESS;
            }
            update_page(model, data);
            break;


        case VIEW_EVENT_CODE_DEVICE_UPDATE:
            update_page(model, data);
            break;

        default:
            break;
    }

    return msg;
}


static view_t update_page(model_t *pmodel, struct page_data *pdata) {
    switch (pmodel->system.removable_drive_state) {
        case REMOVABLE_DRIVE_STATE_MISSING:
            lv_obj_set_hidden(pdata->lbl_info, 0);
            for (size_t i = 0; i < NUM_LINES; i++) {
                lv_obj_set_hidden(pdata->lines[i], 1);
            }
            view_common_set_label_text(pdata->lbl_info,
                                       view_intl_get_string(pmodel, STRINGS_INSERIRE_UN_DISPOSITIVO_DI_ARCHIVIAZIONE));
            break;

        case REMOVABLE_DRIVE_STATE_INVALID:
            lv_obj_set_hidden(pdata->lbl_info, 0);
            for (size_t i = 0; i < NUM_LINES; i++) {
                lv_obj_set_hidden(pdata->lines[i], 1);
            }
            view_common_set_label_text(
                pdata->lbl_info,
                view_intl_get_string(pmodel, STRINGS_DISPOSITIVO_RIMOSSO_PREMERE_MENU_PER_RIAVVIARE_LA_MACCHINA));
            break;

        case REMOVABLE_DRIVE_STATE_MOUNTED: {
            if (pdata->export_operation == ARCHIVE_OPERATION_NONE &&
                pdata->import_operation == ARCHIVE_OPERATION_NONE) {
                lv_obj_set_hidden(pdata->lbl_info, 0);

                if (pmodel->system.num_archivi > 0) {
                    for (size_t i = 0; i < NUM_LINES; i++) {
                        size_t archive_index = pdata->list.start + i;

                        if (archive_index >= pmodel->system.num_archivi) {
                            lv_obj_set_hidden(pdata->lines[i], 1);
                        } else {
                            lv_obj_set_hidden(pdata->lines[i], 0);
                            lv_label_set_text_fmt(pdata->lines[i], "%c %s",
                                                  archive_index == pdata->list.index ? '>' : ' ',
                                                  pmodel->system.archivi[archive_index]);
                        }
                    }

                    view_common_set_label_text(
                        pdata->lbl_info,
                        view_intl_get_string(pmodel, STRINGS_CHIAVE_PER_ESPORTARE_BANDIERA_PER_IMPORTARE));
                } else {
                    lv_obj_set_hidden(pdata->lbl_info, 0);
                    for (size_t i = 0; i < NUM_LINES; i++) {
                        lv_obj_set_hidden(pdata->lines[i], 1);
                    }
                    view_common_set_label_text(pdata->lbl_info,
                                               view_intl_get_string(pmodel, STRINGS_CHIAVE_PER_ESPORTARE));
                }
            } else if (pdata->export_operation == ARCHIVE_OPERATION_NONE) {
                for (size_t i = 0; i < NUM_LINES; i++) {
                    lv_obj_set_hidden(pdata->lines[i], 1);
                }

                switch (pdata->import_operation) {
                    case ARCHIVE_OPERATION_NONE:
                        view_common_set_label_text(pdata->lbl_info,
                                                   view_intl_get_string(pmodel, STRINGS_CHIAVE_PER_ESPORTARE));
                        break;

                    case ARCHIVE_OPERATION_IN_PROGRESS:
                        view_common_set_label_text(pdata->lbl_info,
                                                   view_intl_get_string(pmodel, STRINGS_OPERAZIONE_IN_CORSO));
                        break;

                    case ARCHIVE_OPERATION_SUCCESS:
                        view_common_set_label_text(pdata->lbl_info,
                                                   view_intl_get_string(pmodel, STRINGS_CONFIGURAZIONE_CARICATA_CON_SUCCESSO));
                        break;

                    case ARCHIVE_OPERATION_FAILURE:
                        view_common_set_label_text(
                            pdata->lbl_info,
                            view_intl_get_string(pmodel, STRINGS_ERRORE_NEL_CARICAMENTO_DELLA_CONFIGURAZIONE));
                        break;
                }
            } else if (pdata->import_operation == ARCHIVE_OPERATION_NONE) {
                for (size_t i = 0; i < NUM_LINES; i++) {
                    lv_obj_set_hidden(pdata->lines[i], 1);
                }

                switch (pdata->export_operation) {
                    case ARCHIVE_OPERATION_NONE:
                        view_common_set_label_text(pdata->lbl_info,
                                                   view_intl_get_string(pmodel, STRINGS_CHIAVE_PER_ESPORTARE));
                        break;

                    case ARCHIVE_OPERATION_IN_PROGRESS:
                        view_common_set_label_text(pdata->lbl_info,
                                                   view_intl_get_string(pmodel, STRINGS_OPERAZIONE_IN_CORSO));
                        break;

                    case ARCHIVE_OPERATION_SUCCESS:
                        view_common_set_label_text(
                            pdata->lbl_info,
                            view_intl_get_string(pmodel, STRINGS_CONFIGURAZIONE_ESPORTATA));
                        break;

                    case ARCHIVE_OPERATION_FAILURE:
                        view_common_set_label_text(
                            pdata->lbl_info,
                            view_intl_get_string(pmodel, STRINGS_ERRORE_NEL_SALVATAGGIO_DELLA_CONFIGURAZIONE));
                        break;
                }
            }
            break;
        }
    }
    return 0;
}


const pman_page_t page_removable_drive = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = view_close_all,
    .destroy       = view_destroy_all,
};
