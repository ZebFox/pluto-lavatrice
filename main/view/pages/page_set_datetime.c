#include <stdio.h>
#include <string.h>
#include <time.h>
#include "src/lv_core/lv_obj.h"
#include "src/lv_misc/lv_task.h"
#include "src/lv_objx/lv_label.h"
#include "view/styles.h"
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
#include "esp_log.h"


#define STRING_MAX_LEN 255

enum {
    TIMEOUT_TIMER_ID,
};


struct page_data {
    lv_obj_t *lora;
    lv_obj_t *ldata;
    lv_obj_t *lday;

    int flag;

    struct tm datetime;

    size_t     index;
    char       prec;
    lv_task_t *task;
};


static void        time_op(model_t *pmodel, struct page_data *data, int op);
static const char *get_string_day(model_t *pmodel, struct page_data *data);
static void        timedata_update(model_t *pmodel, struct page_data *data);
// static int         check_date(struct tm tm);


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

    lv_obj_t *lday = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_body_draw(lday, 1);
    lv_obj_set_auto_realign(lday, 1);
    lv_obj_align(lday, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
    data->lday = lday;

    data->index = 0;

    utils_get_sys_time(&data->datetime);
    lv_label_set_text_fmt(data->lora, ":%02i:%02i:%02i", data->datetime.tm_hour, data->datetime.tm_min,
                          data->datetime.tm_sec);
    lv_label_set_text_fmt(data->ldata, ":%02i/%02i/%02i", data->datetime.tm_mday, data->datetime.tm_mon + 1,
                          data->datetime.tm_year % 100);
    lv_label_set_text(data->lday, get_string_day(model, data));
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
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_AGGIORNA_ORA_DATA;
                        utils_set_system_time(data->datetime);
                        utils_set_rtc_time(data->datetime);
                        break;

                    case BUTTON_DESTRA:     // skip destra
                        if (data->index < 11) {
                            data->index++;
                            data->flag = 1;
                            timedata_update(model, data);
                        }
                        break;
                    case BUTTON_SINISTRA:     // skip sinistra
                        if (data->index > 0) {
                            data->index--;
                            data->flag = 1;
                            timedata_update(model, data);
                        }
                        break;
                    case BUTTON_PIU:     // più
                        time_op(model, data, 1);
                        break;
                    case BUTTON_MENO:     // meno
                        time_op(model, data, -1);
                        break;
                    default:
                        break;
                }
            }
            break;
        }

        case VIEW_EVENT_CODE_TIMER: {
            timedata_update(model, data);
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



static void time_op(model_t *pmodel, struct page_data *data, int op) {
    switch (data->index) {
        case (0): {
            int newhour = data->datetime.tm_hour + op * 10;
            if (newhour >= 0 && newhour < 24) {
                data->datetime.tm_hour = newhour;
            }
            break;
        }

        case (1): {
            int newhour = data->datetime.tm_hour + op;
            if (newhour >= 0 && newhour < 24) {
                data->datetime.tm_hour = newhour;
            }
            break;
        }

        case (2): {
            int newmin = data->datetime.tm_min + op * 10;
            if (newmin >= 0 && newmin < 60) {
                data->datetime.tm_min = newmin;
            }
            break;
        }
        case (3): {
            int newmin = data->datetime.tm_min + op;
            if (newmin >= 0 && newmin < 60) {
                data->datetime.tm_min = newmin;
            }
            break;
        }
        case (4): {
            int newsec = data->datetime.tm_sec + op * 10;
            if (newsec >= 0 && newsec < 60) {
                data->datetime.tm_sec = newsec;
            }
            break;
        }
        case (5): {
            int newsec = data->datetime.tm_sec + op;
            if (newsec >= 0 && newsec < 60) {
                data->datetime.tm_sec = newsec;
            }
            break;
        }
        case (6): {
            /*int oldday             = data->datetime.tm_mday;
            data->datetime.tm_mday = oldday + op * 10;
            if (!check_date(data->datetime)) {
                data->datetime.tm_mday = oldday;
            }*/
            data->datetime.tm_mday = data->datetime.tm_mday + op * 10;
            mktime(&data->datetime);
            break;
        }
        case (7): {
            /*int oldday             = data->datetime.tm_mday;
            data->datetime.tm_mday = oldday + op;
            if (!check_date(data->datetime)) {
                data->datetime.tm_mday = oldday;
            }*/
            data->datetime.tm_mday = data->datetime.tm_mday + op;
            mktime(&data->datetime);
            break;
        }
        case (8): {
            /*int oldday             = data->datetime.tm_mday;
            data->datetime.tm_mday = oldday + op;
            if (!check_date(data->datetime)) {
                data->datetime.tm_mday = oldday;
            }*/
            data->datetime.tm_mon = data->datetime.tm_mon + op * 10;
            mktime(&data->datetime);
            break;
        }
        case (9): {
            /*int oldday             = data->datetime.tm_mday;
            data->datetime.tm_mday = oldday + op;
            if (!check_date(data->datetime)) {
                data->datetime.tm_mday = oldday;
            }*/
            data->datetime.tm_mon = data->datetime.tm_mon + op;
            mktime(&data->datetime);
            break;
        }
        case (10): {
            int newyear = data->datetime.tm_year + op * 10;
            if (newyear < 100 || newyear >= 200) {
                newyear = (newyear % 100) + 100;
            }
            data->datetime.tm_year = newyear;
            mktime(&data->datetime);
            break;
        }
        case (11): {
            int newyear = data->datetime.tm_year + op;
            if (newyear < 100 || newyear >= 200) {
                newyear = (newyear % 100) + 100;
            }
            data->datetime.tm_year = newyear;
            mktime(&data->datetime);
            break;
        }
    }

    data->flag = 0;
    timedata_update(pmodel, data);
}


static void timedata_update(model_t *pmodel, struct page_data *data) {

    char timedata[12];
    timedata[0]  = (data->datetime.tm_hour / 10) + 48;
    timedata[1]  = (data->datetime.tm_hour % 10) + 48;
    timedata[2]  = (data->datetime.tm_min / 10) + 48;
    timedata[3]  = (data->datetime.tm_min % 10) + 48;
    timedata[4]  = (data->datetime.tm_sec / 10) + 48;
    timedata[5]  = (data->datetime.tm_sec % 10) + 48;
    timedata[6]  = (data->datetime.tm_mday / 10) + 48;
    timedata[7]  = (data->datetime.tm_mday % 10) + 48;
    timedata[8]  = ((data->datetime.tm_mon + 1) / 10) + 48;
    timedata[9]  = ((data->datetime.tm_mon + 1) % 10) + 48;
    timedata[10] = ((data->datetime.tm_year % 100) / 10) + 48;
    timedata[11] = ((data->datetime.tm_year % 100) % 10) + 48;

    char weekday[STRING_MAX_LEN] = {0};
    memcpy(weekday, get_string_day(pmodel, data), strlen(get_string_day(pmodel, data)));

    if (data->flag) {
        if (data->index < 11) {
            timedata[data->index] = '_';
        }
    } else {
        lv_obj_set_style(data->lday, &style_label_normal);
    }
    data->flag = !data->flag;

    lv_label_set_text_fmt(data->lora, ":%c%c:%c%c:%c%c", timedata[0], timedata[1], timedata[2], timedata[3],
                          timedata[4], timedata[5]);
    lv_label_set_text_fmt(data->ldata, ":%c%c/%c%c/%c%c", timedata[6], timedata[7], timedata[8], timedata[9],
                          timedata[10], timedata[11]);
    lv_label_set_text(data->lday, weekday);

    lv_task_reset(data->task);
}

static const char *get_string_day(model_t *pmodel, struct page_data *data) {
    switch (data->datetime.tm_wday) {
        case (0):
            return view_intl_get_string(pmodel, STRINGS_DOMENICA);
        case (1):
            return view_intl_get_string(pmodel, STRINGS_LUNEDI);
        case (2):
            return view_intl_get_string(pmodel, STRINGS_MARTEDI);
        case (3):
            return view_intl_get_string(pmodel, STRINGS_MERCOLEDI);
        case (4):
            return view_intl_get_string(pmodel, STRINGS_GIOVEDI);
        case (5):
            return view_intl_get_string(pmodel, STRINGS_VENERDI);
        case (6):
            return view_intl_get_string(pmodel, STRINGS_SABATO);
        default:
            return 0;
    }
}

const pman_page_t page_set_datetime = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};

/*
static int check_date(struct tm tm) {
    struct tm res  = tm;
    time_t    time = mktime(&res);
    localtime(&time);
    ESP_LOGI("x", "%i %i %i %i %i %i", tm.tm_mday, res.tm_mday, tm.tm_mon, res.tm_mon, tm.tm_year, res.tm_year);
    if (time == -1 || res.tm_mday != tm.tm_mday || res.tm_mon != tm.tm_mon || res.tm_year != tm.tm_year) {
        return 0;
    } else {
        return 1;
    }
}
*/
