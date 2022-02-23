#include <stdio.h>
#include "view/images/legacy.h"
#include "view/view.h"
#include "gel/pagemanager/page_manager.h"
#include "model/model.h"
#include "lvgl/lvgl.h"
#include "view/fonts/legacy_fonts.h"
#include "view/common.h"
#include "view/widgets/custom_lv_img.h"
#include "view/intl/intl.h"
#include "peripherals/keyboard.h"


struct page_data {
    lv_task_t *task;
    lv_obj_t * label;
    lv_obj_t * lbl_stat;
    size_t     index;
};


static void update_page(model_t *pmodel, struct page_data *data) {
    char string[64] = {0};

    switch (data->index) {
        case 0: {
            unsigned long time = pmodel->stats.tempo_accensione;
            sprintf(string, "%06luh %02lum %02lus", time / 3600, (time / 60) % 60, time % 60);
            lv_label_set_text(data->label, view_intl_get_string(pmodel, STRINGS_TEMPO_ACCENSIONE));
            lv_label_set_text(data->lbl_stat, string);
            break;
        }

        case 1: {
            unsigned long time = pmodel->stats.tempo_lavoro;
            sprintf(string, "%06luh %02lum %02lus", time / 3600, (time / 60) % 60, time % 60);
            lv_label_set_text(data->label, view_intl_get_string(pmodel, STRINGS_TEMPO_LAVORO));
            lv_label_set_text(data->lbl_stat, string);
            break;
        }

        case 2: {
            unsigned long time = pmodel->stats.tempo_moto;
            sprintf(string, "%06luh %02lum %02lus", time / 3600, (time / 60) % 60, time % 60);
            lv_label_set_text(data->label, view_intl_get_string(pmodel, STRINGS_TEMPO_MOTO));
            lv_label_set_text(data->lbl_stat, string);
            break;
        }

        case 3: {
            unsigned long time = pmodel->stats.tempo_riscaldamento;
            sprintf(string, "%06luh %02lum %02lus", time / 3600, (time / 60) % 60, time % 60);
            lv_label_set_text(data->label, view_intl_get_string(pmodel, STRINGS_TEMPO_RISCALDAMENTO));
            lv_label_set_text(data->lbl_stat, string);
            break;
        }

        case 4: {
            sprintf(string, "%06u", pmodel->stats.cicli_interrotti);
            lv_label_set_text(data->label, view_intl_get_string(pmodel, STRINGS_CICLI_PARZIALI));
            lv_label_set_text(data->lbl_stat, string);
            break;
        }

        case 6: {
            sprintf(string, "%06u", pmodel->stats.cicli_eseguiti);
            lv_label_set_text(data->label, view_intl_get_string(pmodel, STRINGS_CICLI_TOTALI));
            lv_label_set_text(data->lbl_stat, string);
            break;
        }

        case 7: {
            sprintf(string, "%06u", pmodel->stats.cicli_loop);
            lv_label_set_text(data->label, view_intl_get_string(pmodel, STRINGS_GETTONI_TOTALI));
            lv_label_set_text(data->lbl_stat, string);
            break;
        }

        default:
            break;
    }
}


static void *create_page(model_t *model, void *extra) {
    return malloc(sizeof(struct page_data));
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *data = args;

    data->index = 0;

    view_common_title(lv_scr_act(), view_intl_get_string(pmodel, STRINGS_STATISTICHE));
    data->task = view_register_periodic_task(500, LV_TASK_PRIO_HIGH, 0);

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lbl, 1);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);
    data->label = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lbl, 1);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(lbl, data->label, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
    data->lbl_stat = lbl;

    update_page(pmodel, data);
}


static view_message_t process_page_event(model_t *pmodel, void *arg, pman_event_t event) {
    view_message_t    msg  = VIEW_EMPTY_MSG;
    struct page_data *data = arg;

    switch (event.code) {
        case VIEW_EVENT_CODE_TIMER:
            update_page(pmodel, data);
            break;

        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                switch (event.key_event.code) {
                    case BUTTON_STOP:
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
                        break;

                    case BUTTON_SINISTRA:
                        data->index = (data->index + 1) % 8;
                        update_page(pmodel, data);
                        break;

                    case BUTTON_DESTRA:
                        if (data->index > 0) {
                            data->index--;
                        } else {
                            data->index = 7;
                        }
                        update_page(pmodel, data);
                        break;

                    case BUTTON_LINGUA:
                        msg.cmsg.code  = VIEW_CONTROLLER_COMMAND_CODE_UPDATE_STATISTICS;
                        break;

                    default:
                        break;
                }
            }
            break;
        }

        default:
            break;
    }

    return msg;
}


static void close_page(void *arg) {
    struct page_data *data = arg;
    lv_obj_clean(lv_scr_act());
    lv_task_del(data->task);
}

const pman_page_t page_stats = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = view_destroy_all,
};