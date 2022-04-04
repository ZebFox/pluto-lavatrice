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
#include "app_config.h"


typedef enum {
    OVERLAY_NONE,
    OVERLAY_DELETE,
    OVERLAY_CLONE,
} overlay_t;


struct page_data {
    size_t index;

    lv_obj_t *lbl_nome;
    lv_obj_t *lbl_message;
    lv_obj_t *lbl_index;
    lv_obj_t *lbl_num_steps;
    lv_obj_t *lbl_prezzo;
    lv_obj_t *lbl_tipo;
    lv_obj_t *img_tipo;
    lv_obj_t *line_vertical;

    lv_task_t *return_task;

    uint16_t  copy_index;
    overlay_t overlay;
    void     *destination;
};


static void update_page(model_t *pmodel, struct page_data *pdata);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->index            = 0;
    pdata->return_task      = view_register_periodic_task(PAGE_TIMEOUT, LV_TASK_PRIO_OFF, 0);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    pdata->overlay          = OVERLAY_NONE;
    view_common_title(lv_scr_act(), view_intl_get_string(pmodel, STRINGS_MODIF_PROGRAMMA));

    lv_task_set_prio(pdata->return_task, LV_TASK_PRIO_MID);
    lv_task_reset(pdata->return_task);

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_BREAK);
    lv_obj_set_auto_realign(lbl, 1);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_width(lbl, 128);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, -8);
    pdata->lbl_message = lbl;

    lv_obj_t *img = custom_lv_img_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(img, 1);
    lv_obj_align(img, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    pdata->img_tipo = img;

    static lv_point_t points[2] = {{0, 0}, {0, 20}};
    lv_obj_t         *line      = view_common_line(points, 2);
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 14);

    line = view_common_horizontal_line();
    lv_obj_align(line, NULL, LV_ALIGN_CENTER, 0, 4);

    static lv_point_t other_points[2] = {{0, 0}, {0, 26}};
    line                              = view_common_line(other_points, 2);
    lv_obj_align(line, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 28, 0);
    pdata->line_vertical = line;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 32, -18);
    pdata->lbl_prezzo = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_SROLL);
    lv_obj_set_width(lbl, 128 - 32);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 32, -2);
    pdata->lbl_tipo = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 22, 15);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_BREAK);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_width(lbl, 110);
    pdata->lbl_nome = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 2, 16);
    pdata->lbl_index = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 2, 26);
    pdata->lbl_num_steps = lbl;

    update_page(pmodel, pdata);
}


static view_message_t process_page_event(model_t *pmodel, void *arg, view_event_t event) {
    view_message_t    msg   = VIEW_EMPTY_MSG;
    struct page_data *pdata = arg;
    (void)pdata;

    switch (event.code) {
        case VIEW_EVENT_CODE_PROGRAM_CHANGE_DONE:
            pdata->overlay = OVERLAY_NONE;
            update_page(pmodel, pdata);
            break;

        case VIEW_EVENT_CODE_PROGRAM_LOADED:
            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
            msg.vmsg.page = pdata->destination;
            break;

        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                lv_task_reset(pdata->return_task);

                switch (event.key_event.code) {
                    case BUTTON_STOP:
                        if (pdata->overlay != OVERLAY_NONE) {
                            pdata->overlay = OVERLAY_NONE;
                            update_page(pmodel, pdata);
                        } else {
                            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
                        }
                        break;

                    case BUTTON_MENU:
                        msg.cmsg.code      = VIEW_CONTROLLER_COMMAND_CODE_LOAD_PROGRAM;
                        msg.cmsg.num       = pdata->index;
                        pdata->destination = (void *)&page_program_name;
                        break;

                    case BUTTON_MENO:
                        if (pdata->overlay == OVERLAY_NONE && pdata->index < model_get_num_programs(pmodel)) {
                            pdata->overlay = OVERLAY_DELETE;
                            update_page(pmodel, pdata);
                        } else if (pdata->overlay == OVERLAY_CLONE) {
                            pdata->copy_index =
                                utils_circular_decrease(pdata->copy_index, model_get_num_programs(pmodel) + 1);
                            // Skip source program
                            if (pdata->copy_index == pdata->index) {
                                pdata->copy_index =
                                    utils_circular_decrease(pdata->copy_index, model_get_num_programs(pmodel) + 1);
                            }
                            update_page(pmodel, pdata);
                        }
                        break;

                    case BUTTON_PIU:
                        if (pdata->overlay == OVERLAY_NONE && pdata->index < model_get_num_programs(pmodel) &&
                            model_get_num_programs(pmodel) < MAX_PROGRAMMI) {
                            pdata->overlay    = OVERLAY_CLONE;
                            pdata->copy_index = pdata->index + 1;
                            update_page(pmodel, pdata);
                        } else if (pdata->overlay == OVERLAY_CLONE) {
                            pdata->copy_index = (pdata->copy_index + 1) % (model_get_num_programs(pmodel) + 1);
                            // Skip source program
                            if (pdata->copy_index == pdata->index) {
                                pdata->copy_index = (pdata->copy_index + 1) % (model_get_num_programs(pmodel) + 1);
                            }
                            update_page(pmodel, pdata);
                        }
                        break;

                    case BUTTON_DESTRA:
                        pdata->overlay = OVERLAY_NONE;
                        pdata->index   = (pdata->index + 1) % (model_get_num_programs(pmodel) + 1);
                        update_page(pmodel, pdata);
                        break;

                    case BUTTON_SINISTRA:
                        pdata->overlay = OVERLAY_NONE;
                        pdata->index   = utils_circular_decrease(pdata->index, model_get_num_programs(pmodel) + 1);
                        update_page(pmodel, pdata);
                        break;

                    case BUTTON_LINGUA:
                        if (pdata->overlay == OVERLAY_DELETE) {
                            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_REMOVE_PROGRAM;
                            msg.cmsg.num  = pdata->index;
                        } else if (pdata->overlay == OVERLAY_CLONE) {
                            msg.cmsg.code        = VIEW_CONTROLLER_COMMAND_CODE_CLONE_PROGRAM;
                            msg.cmsg.source      = pdata->index;
                            msg.cmsg.destination = pdata->copy_index;
                        } else if (pdata->index == model_get_num_programs(pmodel)) {
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
    lv_task_set_prio(pdata->return_task, LV_TASK_PRIO_OFF);
    lv_obj_clean(lv_scr_act());
}


static void destroy_page(void *arg, void *extra) {
    struct page_data *pdata = arg;
    lv_task_del(pdata->return_task);
    free(pdata);
    free(extra);
}


static void update_page(model_t *pmodel, struct page_data *pdata) {
    if (pdata->index >= model_get_num_programs(pmodel)) {
        view_common_set_hidden(pdata->lbl_message, 0);
        view_common_set_hidden(pdata->lbl_nome, 1);
        view_common_set_hidden(pdata->lbl_prezzo, 1);
        view_common_set_hidden(pdata->lbl_tipo, 1);
        view_common_set_hidden(pdata->img_tipo, 1);
        view_common_set_hidden(pdata->line_vertical, 1);
        view_common_set_hidden(pdata->lbl_num_steps, 1);
        lv_label_set_text(pdata->lbl_message, view_intl_get_string(pmodel, STRINGS_NUOVO_PROGRAMMA));
    } else if (pdata->overlay != OVERLAY_NONE) {
        view_common_set_hidden(pdata->lbl_message, 0);
        view_common_set_hidden(pdata->lbl_nome, 1);
        view_common_set_hidden(pdata->lbl_prezzo, 1);
        view_common_set_hidden(pdata->lbl_tipo, 1);
        view_common_set_hidden(pdata->img_tipo, 1);
        view_common_set_hidden(pdata->line_vertical, 1);
        view_common_set_hidden(pdata->lbl_num_steps, 0);
        if (pdata->overlay == OVERLAY_DELETE) {
            lv_label_set_text(pdata->lbl_message, view_intl_get_string(pmodel, STRINGS_CANCELLARE_IL_PROGRAMMA));
        } else if (pdata->overlay == OVERLAY_CLONE) {
            lv_label_set_text_fmt(pdata->lbl_message, "%s %i?",
                                  view_intl_get_string(pmodel, STRINGS_COPIARE_IN_POSIZIONE), pdata->copy_index + 1);
        }
        lv_label_set_text_fmt(pdata->lbl_num_steps, "%02i", model_get_preview(pmodel, pdata->index)->num_steps);
    } else {
        view_common_set_hidden(pdata->lbl_message, 1);
        view_common_set_hidden(pdata->img_tipo, 0);
        view_common_set_hidden(pdata->lbl_nome, 0);
        view_common_set_hidden(pdata->line_vertical, 0);
        view_common_set_hidden(pdata->lbl_prezzo, 0);
        view_common_set_hidden(pdata->lbl_tipo, 0);
        view_common_set_hidden(pdata->lbl_num_steps, 0);
        lv_label_set_text(pdata->lbl_nome, model_get_preview(pmodel, pdata->index)->name);
        lv_label_set_text_fmt(pdata->lbl_prezzo, "%s: %i", view_intl_get_string(pmodel, STRINGS_PREZZO_LOWER),
                              model_get_preview(pmodel, pdata->index)->prezzo);
        view_common_program_type_name(pmodel, pdata->lbl_tipo, model_get_preview(pmodel, pdata->index)->tipo);

        view_common_program_type_image(pdata->img_tipo, model_get_preview(pmodel, pdata->index)->tipo);
        lv_label_set_text_fmt(pdata->lbl_num_steps, "%02i", model_get_preview(pmodel, pdata->index)->num_steps);
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