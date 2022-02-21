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
#include "view/styles.h"
#include "view/intl/intl.h"
#include "utils/utils.h"


enum {
    TIMEOUT_TIMER_ID,
};


struct page_data {
    size_t                 index;
    view_common_password_t password;

    lv_obj_t *lbl_name;
    lv_obj_t *lbl_index;
};


static void update_prog_data(model_t *pmodel, struct page_data *data);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    pdata->index            = 0;
    view_common_password_reset(&pdata->password, get_millis());
    lv_obj_t *line = view_common_horizontal_line();
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_MID, 0, 16);
    static lv_point_t points[2] = {{0, 0}, {0, 16}};
    line                        = view_common_line(points, 2);
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_LEFT, 16, 0);

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(lbl, 110);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 18, 0);
    pdata->lbl_name = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_8x16);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    pdata->lbl_index = lbl;

    update_prog_data(pmodel, pdata);
}


static view_message_t process_page_event(model_t *pmodel, void *arg, view_event_t event) {
    view_message_t    msg   = VIEW_EMPTY_MSG;
    struct page_data *pdata = arg;

    switch (event.code) {
        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                view_common_password_add_key(&pdata->password, event.key_event.code, get_millis());
                if (view_common_check_password(&pdata->password, VIEW_PASSWORD_MINUS, VIEW_SHORT_PASSWORD_LEN,
                                               get_millis())) {
                    msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                    msg.vmsg.page = &page_test_digout;
                    break;
                } else if (view_common_check_password(&pdata->password, VIEW_PASSWORD_RIGHT, VIEW_SHORT_PASSWORD_LEN,
                                                      get_millis())) {
                    msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                    msg.vmsg.page = &page_parmac;
                    break;
                } else if (view_common_check_password(&pdata->password, VIEW_PASSWORD_LEFT, VIEW_SHORT_PASSWORD_LEN,
                                                      get_millis())) {
                    msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                    msg.vmsg.page = &page_programs;
                    break;
                } else if (view_common_check_password(&pdata->password, VIEW_PASSWORD_RESET, VIEW_LONG_PASSWORD_LEN,
                                                      get_millis())) {
                    msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                    msg.vmsg.page = &page_reset_ram;
                    break;
                } else if (view_common_check_password(&pdata->password, VIEW_PASSWORD_TIEPIDO, VIEW_SHORT_PASSWORD_LEN,
                                                      get_millis())) {
                    msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                    msg.vmsg.page = &page_stats;
                    break;
                } else if (view_common_check_password(&pdata->password, VIEW_PASSWORD_SET_DATETIME,
                                                      VIEW_SHORT_PASSWORD_LEN, get_millis())) {
                    // uuuuu
                    msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                    msg.vmsg.page = &page_set_datetime;
                    break;
                } else if (view_common_check_password_started(&pdata->password)) {
                    break;
                }
                switch (event.key_event.code) {
                    case BUTTON_STOP_MENU: {
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                        msg.vmsg.page = &page_datetime;
                        break;
                    }

                    case BUTTON_DESTRA:
                        if (model_get_num_programs(pmodel) > 1) {
                            pdata->index = (pdata->index + 1) % model_get_num_programs(pmodel);
                            update_prog_data(pmodel, pdata);
                        }
                        break;

                    case BUTTON_SINISTRA:
                        if (model_get_num_programs(pmodel) > 1) {
                            if (pdata->index > 0) {
                                pdata->index--;
                            } else {
                                pdata->index = model_get_num_programs(pmodel);
                            }
                            update_prog_data(pmodel, pdata);
                        }
                        break;

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
    struct page_data *pdata = arg;
    (void)pdata;
    lv_obj_clean(lv_scr_act());
}


static void destroy_page(void *arg, void *extra) {
    struct page_data *pdata = arg;
    free(pdata);
    free(extra);
}


static void update_prog_data(model_t *pmodel, struct page_data *data) {
    lv_label_set_text_fmt(data->lbl_index, "%02i", data->index + 1);
    if (model_get_num_programs(pmodel) == 0) {
        lv_label_set_text(data->lbl_name, view_intl_get_string(pmodel, STRINGS_NESSUN_PROGRAMMA));
    } else {
        const programma_preview_t *preview = model_get_preview(pmodel, data->index);
        lv_label_set_text(data->lbl_name, preview->name);
    }
}


const pman_page_t page_main = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};