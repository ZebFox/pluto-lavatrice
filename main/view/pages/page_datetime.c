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
#include "view/intl/intl.h"


enum {
    TIMEOUT_TIMER_ID,
};


struct page_data {
    lv_obj_t * lora;
    lv_obj_t * ldata;
    lv_task_t *task;
    lv_obj_t * lday;

    struct tm time;
};

static const char *get_string_day(model_t *pmodel, struct tm *data);


static void *create_page(model_t *model, void *extra) {
    struct page_data *data = malloc(sizeof(struct page_data));
    data->task             = view_register_periodic_task(500UL, LV_TASK_PRIO_OFF, TIMEOUT_TIMER_ID);
    return data;
}


static void open_page(model_t *model, void *args) {
    struct page_data *data = args;
    view_common_title(lv_scr_act(), "ORA/DATA");

    lv_task_set_prio(data->task, LV_TASK_PRIO_MID);

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
    lv_obj_set_auto_realign(lday, 1);
    lv_obj_align(lday, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
    data->lday = lday;

    utils_get_sys_time(&data->time);
    lv_label_set_text_fmt(data->lora, ":%02i:%02i:%02i", data->time.tm_hour, data->time.tm_min, data->time.tm_sec);
    lv_label_set_text_fmt(data->ldata, ":%02i/%02i/%02i", data->time.tm_mday, data->time.tm_mon + 1,
                          data->time.tm_year - 100);
    lv_label_set_text(data->lday, get_string_day(model, &data->time));
}


static view_message_t process_page_event(model_t *model, void *args, pman_event_t event) {
    struct page_data *data = args;
    (void)data;
    view_message_t msg = VIEW_EMPTY_MSG;

    switch (event.code) {
        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                switch (event.key_event.code) {
                    case BUTTON_STOP:
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
                        break;
                }
            }
            break;
        }

        case VIEW_EVENT_CODE_TIMER: {
            utils_get_sys_time(&data->time);
            lv_label_set_text_fmt(data->lora, ":%02i:%02i:%02i", data->time.tm_hour, data->time.tm_min,
                                  data->time.tm_sec);
            lv_label_set_text_fmt(data->ldata, ":%02i/%02i/%02i", data->time.tm_mday, data->time.tm_mon + 1,
                                  data->time.tm_year - 100);
            lv_label_set_text(data->lday, get_string_day(model, &data->time));

        } break;

        default:
            break;
    }

    return msg;
}

static view_t update_page(model_t *pmodel, void *args) {
    return 0;
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



const pman_page_t page_datetime = {
    .create        = create_page,
    .open          = open_page,
    .update        = update_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};


static const char *get_string_day(model_t *pmodel, struct tm *data) {
    switch (data->tm_wday) {
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
