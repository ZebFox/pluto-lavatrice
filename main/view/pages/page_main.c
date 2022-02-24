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
#include "esp_log.h"


enum {
    TIMER_500MS_ID,
};


struct page_data {
    size_t                 index;
    view_common_password_t password;

    lv_task_t *timer;

    lv_obj_t *lbl_name;
    lv_obj_t *lbl_status;
    lv_obj_t *lbl_time;
    lv_obj_t *lbl_washes;
    lv_obj_t *lbl_info;

    lv_obj_t *img_left;
    lv_obj_t *img_right;
    lv_obj_t *img_washes_sm;
    lv_obj_t *img_info;

    size_t index_info;

    int      flag_status;
    uint16_t counter;
};


static void update_prog_data(model_t *pmodel, struct page_data *data);
static void update_timer_data(model_t *pmodel, struct page_data *pdata);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->timer            = view_register_periodic_task(500UL, LV_TASK_PRIO_OFF, TIMER_500MS_ID);
    pdata->index            = 0;

    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    pdata->flag_status      = 0;
    pdata->index_info       = 0;

    lv_task_set_prio(pdata->timer, LV_TASK_PRIO_MID);

    view_common_password_reset(&pdata->password, get_millis());


    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_8x16);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_SROLL_CIRC);
    lv_obj_set_width(lbl, 128);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, -14);
    pdata->lbl_name = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_8x16);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    pdata->lbl_status = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
    pdata->lbl_time = lbl;

    lv_obj_t *img = custom_lv_img_create(lv_scr_act(), NULL);
    custom_lv_img_set_src(img, &legacy_img_left);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_LEFT, 1, 16);
    pdata->img_left = img;

    img = custom_lv_img_create(lv_scr_act(), NULL);
    custom_lv_img_set_src(img, &legacy_img_right);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_RIGHT, -1, 16);
    pdata->img_right = img;

    img = custom_lv_img_create(lv_scr_act(), NULL);
    custom_lv_img_set_src(img, &legacy_img_wash_sm);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_MID, 0, 9);
    pdata->img_washes_sm = img;

    img = custom_lv_img_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(img, 1);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_MID, -8, 22);
    pdata->img_info = img;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_8x16);
    lv_obj_align(lbl, pdata->img_washes_sm, LV_ALIGN_OUT_RIGHT_MID, 0, -2);
    pdata->lbl_washes = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_8x16);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, pdata->img_info, LV_ALIGN_OUT_RIGHT_MID, 0, -2);
    pdata->lbl_info = lbl;

    pdata->flag_status = 0;
    update_prog_data(pmodel, pdata);
    update_timer_data(pmodel, pdata);

    lv_obj_t *line = view_common_horizontal_line();
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_MID, 0, 8);
    line = view_common_horizontal_line();
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_MID, 0, 9);
    line = view_common_horizontal_line();
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_MID, 0, 36);

    line = view_common_horizontal_line();
    lv_obj_align(line, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -12);
    line = view_common_horizontal_line();
    lv_obj_align(line, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -13);

    static lv_point_t points[2] = {{0, 0}, {0, 26}};
    line                        = view_common_line(points, 2);
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_LEFT, 14, 10);
    line = view_common_line(points, 2);
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_LEFT, 16 + 30, 10);
    line = view_common_line(points, 2);
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_RIGHT, -14, 10);

    static lv_point_t hor_points[2] = {{0, 0}, {65, 0}};
    line                            = view_common_line(hor_points, 2);
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_MID, 15, 22);
}


static view_message_t process_page_event(model_t *pmodel, void *arg, view_event_t event) {
    view_message_t    msg   = VIEW_EMPTY_MSG;
    struct page_data *pdata = arg;

    switch (event.code) {
        case VIEW_EVENT_CODE_PROGRAM_LOADED:
            msg.vmsg.code  = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE_EXTRA;
            msg.vmsg.extra = (void *)(uintptr_t)pmodel->run.num_prog_corrente;
            msg.vmsg.page  = (void *)&page_choice;
            break;

        case VIEW_EVENT_CODE_MODEL_UPDATE:
            if (!model_macchina_in_stop(pmodel)) {
                msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                msg.vmsg.page = (void *)&page_work;
            }
            break;

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
                                pdata->index = model_get_num_programs(pmodel) - 1;
                            }
                            update_prog_data(pmodel, pdata);
                        }
                        break;

                    case BUTTON_LANA:
                        if (pdata->index < model_get_num_programs(pmodel)) {
                            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_LOAD_PROGRAM;
                            msg.cmsg.num  = pdata->index;
                        }
                        break;

                    default:
                        break;
                }
            }
        } break;

        case VIEW_EVENT_CODE_TIMER:
            switch (event.timer_id) {
                case TIMER_500MS_ID:
                    pdata->counter++;
                    if ((pdata->counter % 4) == 0) {
                        pdata->flag_status = !pdata->flag_status;
                    }
                    if ((pdata->counter % 2) == 0) {
                        pdata->index_info = (pdata->index_info + 1) % 4;
                    }
                    update_timer_data(pmodel, pdata);
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }
    return msg;
}


static void close_page(void *arg) {
    struct page_data *pdata = arg;
    lv_task_set_prio(pdata->timer, LV_TASK_PRIO_OFF);
    lv_obj_clean(lv_scr_act());
}


static void destroy_page(void *arg, void *extra) {
    struct page_data *pdata = arg;
    lv_task_del(pdata->timer);
    free(pdata);
    free(extra);
}


static void update_prog_data(model_t *pmodel, struct page_data *data) {
    if (model_get_num_programs(pmodel) == 0) {
        lv_label_set_text(data->lbl_name, view_intl_get_string(pmodel, STRINGS_NESSUN_PROGRAMMA));
        lv_obj_set_hidden(data->img_left, 1);
        lv_obj_set_hidden(data->img_right, 1);
        lv_obj_set_hidden(data->img_washes_sm, 1);
        lv_obj_set_hidden(data->lbl_washes, 1);
        lv_obj_set_hidden(data->img_info, 1);
        lv_obj_set_hidden(data->lbl_info, 1);
    } else {
        const programma_preview_t *preview = model_get_preview(pmodel, data->index);

        lv_label_set_text_fmt(data->lbl_name, "%02i-%s", data->index + 1, preview->name);
        lv_obj_set_hidden(data->img_left, 0);
        lv_obj_set_hidden(data->img_right, 0);
        lv_obj_set_hidden(data->img_washes_sm, 0);
        lv_obj_set_hidden(data->lbl_washes, 0);
        lv_obj_set_hidden(data->img_info, 0);
        lv_obj_set_hidden(data->lbl_info, 0);
        lv_label_set_text_fmt(data->lbl_washes, "x%02i", preview->lavaggi);
    }
}


static void update_timer_data(model_t *pmodel, struct page_data *pdata) {
    struct tm time;
    utils_get_sys_time(&time);
    lv_label_set_text_fmt(pdata->lbl_time, "%02i/%02i/%02i %02i:%02i:%02i", time.tm_mday, time.tm_mon,
                          time.tm_year - 100, time.tm_hour, time.tm_min, time.tm_sec);

    if (pdata->flag_status) {
        lv_label_set_text(pdata->lbl_status, view_intl_get_string(pmodel, STRINGS_SCELTA_PROGRAMMA));
    } else {
        lv_label_set_text(pdata->lbl_status, view_intl_get_string(pmodel, STRINGS_E_PREMERE_START));
    }

    if (model_get_num_programs(pmodel) > 0) {
        const programma_preview_t *preview   = model_get_preview(pmodel, pdata->index);
        const GSYMBOL             *symbols[] = {
            &legacy_img_time,
            &legacy_img_temperature,
            &legacy_img_speed,
            &legacy_img_level,
        };
        custom_lv_img_set_src(pdata->img_info, symbols[pdata->index_info]);
        switch (pdata->index_info) {
            case 0:
                lv_label_set_text_fmt(pdata->lbl_info, "%02im%02is", preview->durata / 60, preview->durata % 60);
                break;
            case 1:
                lv_label_set_text_fmt(pdata->lbl_info, "%02i C", preview->temperatura);
                break;
            case 2:
                lv_label_set_text_fmt(pdata->lbl_info, "%02irpm", preview->velocita);
                break;
            case 3:
                lv_label_set_text_fmt(pdata->lbl_info, "%02i lt", preview->livello);
                break;
            default:
                assert(0);
        }
    }
}


const pman_page_t page_main = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};