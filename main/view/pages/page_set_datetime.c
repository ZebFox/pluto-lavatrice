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

enum {
    TIMEOUT_TIMER_ID,
};


struct page_data {
    lv_obj_t *lora;
    lv_obj_t *ldata;

    int flag;

    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t day;
    uint8_t month;
    uint8_t year;

    size_t     index;
    char       prec;
    lv_task_t *task;
};


static void timedata_update(struct page_data *data);
static void time_op(struct page_data *data, int op);


static void *create_page(model_t *model, void *extra) {
    struct page_data *data = malloc(sizeof(struct page_data));
    data->task             = view_register_periodic_task(500UL, LV_TASK_PRIO_OFF, TIMEOUT_TIMER_ID);
    return data;
}


static void open_page(model_t *model, void *args) {
    struct page_data *data = args;
    view_common_title(lv_scr_act(), "SET ORA/DATA");

    lv_task_set_prio(data->task, LV_TASK_PRIO_MID);
    data->flag = 1;

    lv_obj_t *title_ora = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(title_ora, "ORA");
    lv_obj_align(title_ora, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 20);

    lv_obj_t *title_data = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(title_data, "DATA");
    lv_obj_align(title_data, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 30);

    lv_obj_t *lora = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lora, 1);
    lv_obj_align(lora, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 20);
    data->lora = lora;

    lv_obj_t *ldata = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(ldata, 1);
    lv_obj_align(ldata, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 30);
    data->ldata = ldata;

    data->index = 0;

    struct tm time;
    utils_get_sys_time(&time);
    data->hour  = time.tm_hour;
    data->min   = time.tm_min;
    data->sec   = time.tm_sec;
    data->day   = time.tm_mday;
    data->month = time.tm_mon + 1;
    data->year  = time.tm_year - 100;
    lv_label_set_text_fmt(data->lora, ":%02i:%02i:%02i", data->hour, data->min, data->sec);
    lv_label_set_text_fmt(data->ldata, ":%02i/%02i/%02i", data->day, data->month, data->year);
}


static view_message_t process_page_event(model_t *model, void *args, pman_event_t event) {
    struct page_data *data = args;
    view_message_t    msg  = VIEW_EMPTY_MSG;

    switch (event.code) {
        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                switch (event.key_event.code) {
                    case BUTTON_STOP:
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
                        break;
                    case BUTTON_DESTRA:     // skip destra
                        if (data->index < 12) {
                            data->index++;
                            data->flag = 1;
                            timedata_update(data);
                        }
                        break;
                    case BUTTON_CALDO:     // skip sinistra
                        if (data->index > 0) {
                            data->index--;
                            data->flag = 1;
                            timedata_update(data);
                        }
                        break;
                    case BUTTON_PIU:     // più
                        time_op(data, 1);
                        break;
                    case BUTTON_FREDDO:     // meno
                        time_op(data, -1);
                        break;
                    default:
                        break;
                }
            }
            break;
        }

        case VIEW_EVENT_CODE_TIMER: {
            timedata_update(data);
        } break;
        default:
            break;
    }
    return msg;
}



static void close_page(void *arg) {
    struct page_data *data = arg;
    lv_task_set_prio(data->task, LV_TASK_PRIO_OFF);
    lv_obj_clean(lv_scr_act());
}


static void destroy_page(void *arg, void *extra) {
    struct page_data *data = arg;
    lv_task_del(data->task);
    free(data);
    free(extra);
}



static void time_op(struct page_data *data, int op) {
    switch (data->index) {
        case (0): {
            int newhour = data->hour + op * 10;
            if (newhour >= 0 && newhour < 24) {
                data->hour = newhour;
            }
            break;
        }

        case (1): {
            int newhour = data->hour + op;
            if (newhour >= 0 && newhour < 24) {
                data->hour = newhour;
            }
            break;
        }

        case (2): {
            int newmin = data->min + op * 10;
            if (newmin >= 0 && newmin < 60) {
                data->min = newmin;
            }
            break;
        }
        case (3): {
            int newmin = data->min + op;
            if (newmin >= 0 && newmin < 60) {
                data->min = newmin;
            }
            break;
        }
        case (4): {
            int newsec = data->sec + op * 10;
            if (newsec >= 0 && newsec < 60) {
                data->sec = newsec;
            }
            break;
        }
        case (5): {
            int newsec = data->sec + op;
            if (newsec >= 0 && newsec < 60) {
                data->sec = newsec;
            }
            break;
        }
    }

    data->flag = 0;
    timedata_update(data);
}


static void timedata_update(struct page_data *data) {

    char timedata[12];
    timedata[0]  = (data->hour / 10) + 48;
    timedata[1]  = (data->hour % 10) + 48;
    timedata[2]  = (data->min / 10) + 48;
    timedata[3]  = (data->min % 10) + 48;
    timedata[4]  = (data->sec / 10) + 48;
    timedata[5]  = (data->sec % 10) + 48;
    timedata[6]  = (data->day / 10) + 48;
    timedata[7]  = (data->day % 10) + 48;
    timedata[8]  = (data->month / 10) + 48;
    timedata[9]  = (data->month % 10) + 48;
    timedata[10] = (data->year / 10) + 48;
    timedata[11] = (data->year % 10) + 48;

    if (data->flag) {
        timedata[data->index] = '_';
    }
    data->flag = !data->flag;

    lv_label_set_text_fmt(data->lora, ":%c%c:%c%c:%c%c", timedata[0], timedata[1], timedata[2], timedata[3],
                          timedata[4], timedata[5]);
    lv_label_set_text_fmt(data->ldata, ":%c%c/%c%c/%c%c", timedata[6], timedata[7], timedata[8], timedata[9],
                          timedata[10], timedata[11]);

    lv_task_reset(data->task);
}




const pman_page_t page_set_datetime = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};