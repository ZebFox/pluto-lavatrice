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
    lv_obj_t *label_mem_free;
    lv_obj_t *label_low_watermark;

    lv_task_t *timer;
};


static const char *TAG = "PageInfo";


static void update_page(model_t *pmodel, struct page_data *pdata) {
    lv_label_set_text_fmt(pdata->label_mem_free, "Free  : %i", xPortGetFreeHeapSize());
    lv_label_set_text_fmt(pdata->label_low_watermark, "Low WM: %i", xPortGetMinimumEverFreeHeapSize());
}


static void *create_page(model_t *pmodel, void *extra) {
    (void)TAG;
    (void)pmodel;
    (void)extra;
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->timer            = view_register_periodic_task(500UL, LV_TASK_PRIO_OFF, 0);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;

    lv_task_set_prio(pdata->timer, LV_TASK_PRIO_MID);

    view_common_title(lv_scr_act(), view_intl_get_string(pmodel, STRINGS_INFORMAZIONI));

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_label_set_text_fmt(lbl, "V%i.%c.%i - %s", FIRMWARE_VERSION_MAJOR, FIRMWARE_VERSION_MINOR, FIRMWARE_VERSION_PATCH,
                          utils_get_build_date());
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 20);

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 32);
    pdata->label_mem_free = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_set_style(lbl, &style_label_6x8);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 40);
    pdata->label_low_watermark = lbl;

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
                        break;
                    }

                    case BUTTON_PIU: {
                        break;
                    }

                    case BUTTON_STOP: {
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
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


const pman_page_t page_info = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};