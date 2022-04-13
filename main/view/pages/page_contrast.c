#include <stdio.h>
#include "view/view.h"
#include "gel/pagemanager/page_manager.h"
#include "model/model.h"
#include "peripherals/keyboard.h"
#include "lvgl/lvgl.h"
#include "view/fonts/legacy_fonts.h"
#include "view/common.h"
#include "view/styles.h"
#include "view/intl/intl.h"
#include "esp_log.h"
#include "app_config.h"
#include "utils/utils.h"


struct page_data {
    lv_obj_t *label_contrast;
};


static const char *TAG = "PageContrast";


static void update_page(model_t *pmodel, struct page_data *pdata) {
    lv_label_set_text_fmt(pdata->label_contrast, "%2i", pmodel->prog.contrast);
}


static void *create_page(model_t *pmodel, void *extra) {
    (void)TAG;
    struct page_data *pdata = malloc(sizeof(struct page_data));
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;

    view_common_title(lv_scr_act(), view_intl_get_string(pmodel, STRINGS_CONTRASTO_LCD));

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_8x16);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_CENTER, 0, 8);
    pdata->label_contrast = lbl;

    update_page(pmodel, pdata);
}


static view_message_t process_page_event(model_t *pmodel, void *args, pman_event_t event) {
    view_message_t    msg   = VIEW_EMPTY_MSG;
    struct page_data *pdata = args;

    switch (event.code) {
        case VIEW_EVENT_CODE_MODEL_UPDATE:
            break;

        case VIEW_EVENT_CODE_OPEN:
            break;

        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                switch (event.key_event.code) {
                    case BUTTON_SINISTRA: {
                        break;
                    }

                    case BUTTON_DESTRA: {
                        break;
                    }

                    case BUTTON_MENO: {
                        if (pmodel->prog.contrast > 0x10) {
                            pmodel->prog.contrast--;
                        } else {
                            pmodel->prog.contrast = 0x30 - 1;
                        }
                        update_page(pmodel, pdata);
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_UPDATE_CONTRAST;
                        break;
                    }

                    case BUTTON_PIU: {
                        if (pmodel->prog.contrast < 0x30) {
                            pmodel->prog.contrast++;
                        } else {
                            pmodel->prog.contrast = 0x10;
                        }
                        update_page(pmodel, pdata);
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_UPDATE_CONTRAST;
                        break;
                    }

                    case BUTTON_STOP: {
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_SAVE_CONTRAST;
                        break;
                    }

                    case BUTTON_LINGUA: {
                        break;
                    }

                    default:
                        break;
                }
            }
            break;
        }

        case VIEW_EVENT_CODE_TIMER:
            update_page(pmodel, pdata);
            break;

        default:
            break;
    }

    return msg;
}


const pman_page_t page_contrast = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = view_close_all,
    .destroy       = view_destroy_all,
};