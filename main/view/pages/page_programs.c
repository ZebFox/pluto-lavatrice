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
#include "view/intl/intl.h"
#include "utils/utils.h"
#include "view/styles.h"


struct page_data {
    size_t index;

    lv_obj_t *lbl_nome;
    lv_obj_t *lbl_nuovo_programma;
    lv_obj_t *lbl_index;
    lv_obj_t *lbl_prezzo;

    void *destination;
};


static void update_page(model_t *pmodel, struct page_data *pdata);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->index            = 0;
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    view_common_title(lv_scr_act(), view_intl_get_string(pmodel, STRINGS_MODIF_PROGRAMMA));

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_BREAK);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_width(lbl, 128);
    lv_label_set_text(lbl, view_intl_get_string(pmodel, STRINGS_NUOVO_PROGRAMMA));
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, -8);
    pdata->lbl_nuovo_programma = lbl;

    static lv_point_t points[2] = {{0, 0}, {0, 20}};
    lv_obj_t         *line      = view_common_line(points, 2);
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_LEFT, 18, 14);

    line = view_common_horizontal_line();
    lv_obj_align(line, NULL, LV_ALIGN_CENTER, 0, 4);

    static lv_point_t other_points[2] = {{0, 0}, {0, 26}};
    line                              = view_common_line(other_points, 2);
    lv_obj_align(line, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 26, 0);

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 32, -18);
    pdata->lbl_prezzo = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 15);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_BREAK);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_width(lbl, 110);
    pdata->lbl_nome = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 2, 16);
    pdata->lbl_index = lbl;

    update_page(pmodel, pdata);
}


static view_message_t process_page_event(model_t *pmodel, void *arg, view_event_t event) {
    view_message_t    msg   = VIEW_EMPTY_MSG;
    struct page_data *pdata = arg;
    (void)pdata;

    switch (event.code) {
        case VIEW_EVENT_CODE_PROGRAM_LOADED:
            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
            msg.vmsg.page = pdata->destination;
            break;

        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                switch (event.key_event.code) {
                    case BUTTON_STOP:
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
                        break;

                    case BUTTON_MENU:
                        msg.cmsg.code      = VIEW_CONTROLLER_COMMAND_CODE_LOAD_PROGRAM;
                        msg.cmsg.num       = pdata->index;
                        pdata->destination = (void *)&page_program_name;
                        break;

                    case BUTTON_DESTRA:
                        pdata->index = (pdata->index + 1) % (model_get_num_programs(pmodel) + 1);
                        update_page(pmodel, pdata);
                        break;

                    case BUTTON_SINISTRA:
                        if (pdata->index > 0) {
                            pdata->index--;
                        } else {
                            pdata->index = model_get_num_programs(pmodel);
                        }
                        update_page(pmodel, pdata);
                        break;

                    case BUTTON_LINGUA:
                        if (pdata->index == model_get_num_programs(pmodel)) {
                            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_CREATE_PROGRAM;
                        } else {
                            msg.cmsg.code      = VIEW_CONTROLLER_COMMAND_CODE_LOAD_PROGRAM;
                            msg.cmsg.num       = pdata->index;
                            pdata->destination = (void *)&page_program_steps;
                        }
                        break;

                    default:
                        break;
                }
            }
            break;
        }

        case VIEW_EVENT_CODE_DATA_REFRESH:
            update_page(pmodel, pdata);
            break;

        case VIEW_EVENT_CODE_TIMER:
            break;

        default:
            break;
    }
    return msg;
}


static void close_page(void *arg) {
    struct page_data *pdata = arg;
    (void)pdata;
    lv_obj_clean(lv_scr_act());
}


static void destroy_page(void *arg, void *extra) {
    struct page_data *pdata = arg;
    free(pdata);
    free(extra);
}


static void update_page(model_t *pmodel, struct page_data *pdata) {
    if (pdata->index >= model_get_num_programs(pmodel)) {
        lv_obj_set_hidden(pdata->lbl_nuovo_programma, 0);
        lv_obj_set_hidden(pdata->lbl_nome, 1);
        lv_obj_set_hidden(pdata->lbl_prezzo, 1);
    } else {
        lv_obj_set_hidden(pdata->lbl_nuovo_programma, 1);
        lv_obj_set_hidden(pdata->lbl_nome, 0);
        lv_obj_set_hidden(pdata->lbl_prezzo, 0);
        lv_label_set_text(pdata->lbl_nome, model_get_preview(pmodel, pdata->index)->name);
        lv_label_set_text_fmt(pdata->lbl_prezzo, "%s: %i", view_intl_get_string(pmodel, STRINGS_PREZZO_LOWER),
                              model_get_program(pmodel)->prezzo);
    }
    lv_label_set_text_fmt(pdata->lbl_index, "%02i", pdata->index + 1);
}


const pman_page_t page_programs = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};