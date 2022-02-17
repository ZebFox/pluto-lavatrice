#include <stdlib.h>
#include <assert.h>
#include "lvgl/lvgl.h"
#include "view/view.h"
#include "view/images/legacy.h"
#include "view/view_types.h"
#include "view/widgets/custom_lv_img.h"
#include "gel/pagemanager/page_manager.h"
#include "peripherals/keyboard.h"
#include "view/common.h"
#include "utils/utils.h"


// enum { };


struct page_data {
    size_t index;

    lv_obj_t *lbl_nome;
};


static void update_page(model_t *pmodel, struct page_data *pdata) {

}


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->index            = 0;
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    pdata->lbl_nome = lbl;
}


static view_message_t process_page_event(model_t *pmodel, void *arg, view_event_t event) {
    view_message_t    msg  = VIEW_EMPTY_MSG;
    struct page_data *data = arg;
    (void)data;

    switch (event.code) {
        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {

                switch (event.key_event.code) {
                    case BUTTON_LANA: {
                        break;
                    }
                    default:
                        break;
                }
            }
        } break;

        case VIEW_EVENT_CODE_TIMER:
            break;

        default:
            break;
    }
    return msg;
}


static void close_page(void *arg) {
    struct page_data *data = arg;
    (void)data;
    lv_obj_clean(lv_scr_act());
}


static void destroy_page(void *arg, void *extra) {
    struct page_data *data = arg;
    free(data);
    free(extra);
}


const pman_page_t page_programs = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};