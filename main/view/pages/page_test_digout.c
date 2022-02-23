#include <stdio.h>
#include "view/view.h"
#include "gel/pagemanager/page_manager.h"
#include "model/model.h"
#include "peripherals/keyboard.h"
#include "lvgl/lvgl.h"
#include "view/fonts/legacy_fonts.h"
#include "view/common.h"

static char *string_status(uint8_t status);

static struct {
    lv_obj_t  *lbl_out;
    lv_obj_t  *lbl_status;
    lv_obj_t  *auto_off;
    int        digout_index;
    uint8_t    digout_state;
    lv_task_t *task;
    int        time;
} page_data;


static void update_page(model_t *model) {
    lv_label_set_text_fmt(page_data.lbl_out, "output: %i", page_data.digout_index);
    lv_label_set_text_fmt(page_data.lbl_status, "%s", string_status(page_data.digout_state));
    lv_label_set_text_fmt(page_data.auto_off, "T.AUTO-OFF[%isec]", page_data.time);
}


static void *create_page(model_t *model, void *extra) {
    page_data.task = view_common_register_timer(1000);
    return NULL;
}

static void open_page(model_t *model, void *data) {
    view_common_title(lv_scr_act(), "TEST USCITE");

    page_data.digout_index = 1;
    lv_obj_t *lblout       = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lblout, 1);
    lv_obj_align(lblout, NULL, LV_ALIGN_IN_TOP_LEFT, 2, 20);
    page_data.lbl_out = lblout;

    lv_obj_t *lblstato = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lblstato, 1);
    lv_obj_align(lblstato, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 20);
    page_data.lbl_status = lblstato;

    page_data.time   = 5;
    lv_obj_t *lbloff = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lbloff, 1);
    lv_obj_align(lbloff, NULL, LV_ALIGN_IN_LEFT_MID, 0, 20);
    page_data.auto_off = lbloff;

    update_page(model);
}


static view_message_t process_page_event(model_t *model, void *arg, pman_event_t event) {
    view_message_t msg = VIEW_EMPTY_MSG;

    switch (event.code) {
        case VIEW_EVENT_CODE_OPEN:
            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_TEST;
            msg.cmsg.test = 1;
            break;

        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                switch (event.key_event.code) {
                    case BUTTON_PIU: {
                        msg.vmsg.code          = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE;
                        msg.vmsg.page          = &page_main;
                        page_data.digout_state = 0;

                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_TEST;
                        msg.cmsg.test = 0;
                        break;
                    }
                    case BUTTON_SINISTRA: {
                        /*msg.vmsg.code          = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE;
                        page_data.digout_state = 0;
                        msg.vmsg.page          = &page_splash;

                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_TEST;
                        msg.cmsg.test = 0;*/
                        break;
                    }
                    case BUTTON_MENO: {
                        page_data.digout_state = 0;
                        if (page_data.digout_index < 20)
                            page_data.digout_index++;
                        else {
                            page_data.digout_index = 1;
                        }
                        lv_task_set_prio(page_data.task, LV_TASK_PRIO_OFF);
                        lv_task_reset(page_data.task);
                        page_data.time = 5;
                        update_page(model);

                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_DIGOUT_TURNOFF;
                        break;
                    }
                    case BUTTON_DESTRA: {
                        page_data.digout_state = 0;
                        if (page_data.digout_index > 1) {
                            page_data.digout_index--;

                        } else {
                            page_data.digout_index = 20;
                        }
                        lv_task_set_prio(page_data.task, LV_TASK_PRIO_OFF);
                        lv_task_reset(page_data.task);
                        page_data.time = 5;
                        update_page(model);

                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_DIGOUT_TURNOFF;
                        break;
                    }
                    case BUTTON_STOP: {
                        page_data.digout_state = 0;
                        lv_task_set_prio(page_data.task, LV_TASK_PRIO_OFF);
                        lv_task_reset(page_data.task);
                        page_data.time = 5;
                        update_page(model);

                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_DIGOUT_TURNOFF;
                        break;
                    }
                    case BUTTON_LINGUA: {
                        page_data.digout_state = 1;
                        lv_task_set_prio(page_data.task, LV_TASK_PRIO_MID);
                        update_page(model);

                        msg.cmsg.code   = VIEW_CONTROLLER_COMMAND_CODE_TEST_DIGOUT;
                        msg.cmsg.output = page_data.digout_index;
                        msg.cmsg.value  = 1;
                        break;
                    }

                    default:
                        break;
                }
                break;
            }

            default:
                break;
        }

        case VIEW_EVENT_CODE_MODEL_UPDATE:
            break;

        case VIEW_EVENT_CODE_TIMER: {
            page_data.time--;
            if (page_data.time == 0) {
                msg.cmsg.code          = VIEW_CONTROLLER_COMMAND_CODE_DIGOUT_TURNOFF;
                page_data.digout_state = 0;

                lv_task_set_prio(page_data.task, LV_TASK_PRIO_OFF);
                lv_task_reset(page_data.task);
                page_data.time = 5;
            }
            update_page(model);
            break;
        }
    }

    return msg;
}


static void view_close(void *data) {
    (void)data;
    lv_obj_clean(lv_scr_act());
    lv_task_set_prio(page_data.task, LV_TASK_PRIO_OFF);
}

static void page_destroy(void *data, void *extra) {
    lv_task_del(page_data.task);
}


const pman_page_t page_test_digout = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = view_close,
    .destroy       = page_destroy,
};

static char *string_status(uint8_t status) {
    if (status)
        return "ON";
    else
        return "OFF";
}