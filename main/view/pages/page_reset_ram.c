#include <stdio.h>
#include "view/images/legacy.h"
#include "view/view.h"
#include "gel/pagemanager/page_manager.h"
#include "model/model.h"
#include "lvgl/lvgl.h"
#include "view/fonts/legacy_fonts.h"
#include "view/common.h"
#include "view/widgets/custom_lv_img.h"
#include "peripherals/keyboard.h"


struct page_data {
    lv_obj_t *lbl;
    size_t    count;
    size_t    count2;
};


static void *create_page(model_t *model, void *extra) {
    return malloc(sizeof(struct page_data));
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *data = args;
    data->count            = 0;
    data->count2           = 0;

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(lbl, "* DEL RAM  ? *");
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_MID, 0, 16);

    data->lbl = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text_fmt(data->lbl, "[Start]   (%i)", data->count);
    lv_obj_align(data->lbl, lbl, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
}


static view_message_t process_page_event(model_t *pmodel, void *arg, pman_event_t event) {
    view_message_t    msg  = VIEW_EMPTY_MSG;
    struct page_data *data = arg;

    switch (event.code) {
        case VIEW_EVENT_CODE_TIMER:
            break;

        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                switch (event.key_event.code) {
                    case BUTTON_STOP:
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_REBASE;
                        msg.vmsg.page = view_main_page(pmodel);
                        break;

                    case BUTTON_LINGUA:
                        data->count++;
                        if (data->count >= 3) {
                            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_RESET_RAM;
                            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_REBASE;
                            msg.vmsg.page = &page_splash;
                        } else {
                            lv_label_set_text_fmt(data->lbl, "[Start]   (%i)", data->count);
                        }
                        break;

                    case BUTTON_PIU:
                        data->count2++;
                        if (data->count2 >= 3) {
                            // pmodel->pmac.abilita_parametri_ridotti = 0;
                            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_REBASE;
                            msg.vmsg.page = &page_splash;
                        }
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


const pman_page_t page_reset_ram = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = view_close_all,
    .destroy       = view_destroy_all,
};