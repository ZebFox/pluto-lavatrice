#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "lvgl/lvgl.h"
#include "src/lv_core/lv_obj.h"
#include "src/lv_misc/lv_task.h"
#include "src/lv_objx/lv_label.h"
#include "view/styles.h"
#include "view/view.h"
#include "view/images/legacy.h"
#include "view/view_types.h"
#include "view/widgets/custom_lv_img.h"
#include "gel/pagemanager/page_manager.h"
#include "peripherals/keyboard.h"
#include "view/common.h"
#include "view/intl/intl.h"
#include "utils/utils.h"
#include "esp_log.h"


enum {
    TIMEOUT_TIMER_ID,
};


struct page_data {
    lv_obj_t *img;
    lv_obj_t *lbl;
};


static void update_page(model_t *pmodel, struct page_data *pdata);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;

    view_common_title(lv_scr_act(), view_intl_get_string(pmodel, STRINGS_TIPO));

    lv_obj_t *img = custom_lv_img_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(img, 1);
    lv_obj_set_size(img, 32, 28);
    lv_obj_align(img, NULL, LV_ALIGN_IN_LEFT_MID, 0, 0);
    pdata->img = img;

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lbl, 1);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(lbl, 96);
    lv_obj_align(lbl, img, LV_ALIGN_OUT_RIGHT_MID, 4, 0);
    pdata->lbl = lbl;

    update_page(pmodel, pdata);
}


static view_message_t process_page_event(model_t *pmodel, void *arg, view_event_t event) {
    view_message_t    msg  = VIEW_EMPTY_MSG;
    struct page_data *data = arg;
    (void)data;

    switch (event.code) {
        case VIEW_EVENT_CODE_PROGRAM_SAVED:
            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
            break;

        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                switch (event.key_event.code) {
                    case BUTTON_MENU:
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE;
                        msg.vmsg.page = (void *)&page_program_name;
                        break;

                    case BUTTON_STOP: {
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_UPDATE_PROGRAM;
                        break;
                    }

                    case BUTTON_DESTRA: {
                    }

                    case BUTTON_SINISTRA: {
                        break;
                    }

                    case BUTTON_MENO: {
                        if (model_get_program(pmodel)->tipo > 0) {
                            model_get_program(pmodel)->tipo--;
                        } else {
                            model_get_program(pmodel)->tipo = NUM_TIPI_PROGRAMMA - 1;
                        }
                        model_update_preview(pmodel);
                        update_page(pmodel, data);
                        break;
                    }

                    case BUTTON_PIU: {
                        model_get_program(pmodel)->tipo = (model_get_program(pmodel)->tipo + 1) % NUM_TIPI_PROGRAMMA;
                        model_update_preview(pmodel);
                        update_page(pmodel, data);
                        break;
                    }

                    case BUTTON_START:
                        break;

                    case BUTTON_LINGUA: {
                        break;
                    }
                }
            }
            break;
        }

        case VIEW_EVENT_CODE_TIMER: {
            break;
        }

        break;

        default:
            break;
    }
    return msg;
}


static void update_page(model_t *pmodel, struct page_data *pdata) {
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

    uint8_t type = model_get_program(pmodel)->tipo;
    printf("%i\n", type);

    view_common_program_type_image(pdata->img, type);
    lv_label_set_text(pdata->lbl, view_intl_get_string(pmodel, labels[type]));
}


const pman_page_t page_program_type = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = view_close_all,
    .destroy       = view_destroy_all,
};