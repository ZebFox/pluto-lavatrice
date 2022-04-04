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
#include "view/styles.h"
#include "peripherals/keyboard.h"
#include "esp_log.h"


#define ELEMENTS    5
#define TOTAL_STATS 22


struct page_data {
    lv_task_t *task;
    size_t     index;
    size_t     start;

    lv_obj_t *lines[ELEMENTS];
};


struct stat {
    uint32_t *ptr;
    strings_t desc;
    int       time;
};


static const char *TAG = "PageStats";


static void update_page(model_t *pmodel, struct page_data *data) {
    statistics_t *stats = &pmodel->stats;

    if (data->index >= data->start + ELEMENTS) {
        data->start = data->index - (ELEMENTS - 1);
    } else if (data->index < data->start) {
        data->start = data->index;
    }

    const struct stat stat_list[TOTAL_STATS] = {
        {&stats->tempo_accensione, STRINGS_TEMPO_ACCENSIONE, 1},
        {&stats->tempo_lavoro, STRINGS_TEMPO_LAVORO, 1},
        {&stats->cicli_eseguiti, STRINGS_CICLI_TOTALI, 0},
        {&stats->cicli_interrotti, STRINGS_CICLI_PARZIALI, 0},
        {&stats->tempo_moto, STRINGS_TEMPO_MOTO, 1},
        {&stats->tempo_riscaldamento, STRINGS_TEMPO_RISCALDAMENTO, 1},
        {&stats->tempo_h2o_calda, STRINGS_TEMPO_H2O_CALDA, 1},
        {&stats->tempo_h2o_fredda, STRINGS_TEMPO_H2O_FREDDA, 1},
        {&stats->tempo_h2o_flusso, STRINGS_TEMPO_H2O_FLUSSO, 1},
        {&stats->tempo_h2o_rec_dep, STRINGS_TEMPO_H2O_REC_DEP, 1},
        {&stats->chiusure_oblo, STRINGS_CHIUSURE_OBLO, 0},
        {&stats->aperture_oblo, STRINGS_APERTURE_OBLO, 0},
        {&stats->tempo_saponi[0], STRINGS_TEMPO_SAPONE_1, 1},
        {&stats->tempo_saponi[1], STRINGS_TEMPO_SAPONE_2, 1},
        {&stats->tempo_saponi[2], STRINGS_TEMPO_SAPONE_3, 1},
        {&stats->tempo_saponi[3], STRINGS_TEMPO_SAPONE_4, 1},
        {&stats->tempo_saponi[4], STRINGS_TEMPO_SAPONE_5, 1},
        {&stats->tempo_saponi[5], STRINGS_TEMPO_SAPONE_6, 1},
        {&stats->tempo_saponi[6], STRINGS_TEMPO_SAPONE_7, 1},
        {&stats->tempo_saponi[7], STRINGS_TEMPO_SAPONE_8, 1},
        {&stats->tempo_saponi[8], STRINGS_TEMPO_SAPONE_9, 1},
        {&stats->tempo_saponi[9], STRINGS_TEMPO_SAPONE_10, 1},
    };

    for (size_t i = 0; i < ELEMENTS; i++) {
        size_t pos = data->start + i;
        if (pos >= TOTAL_STATS) {
            lv_obj_set_hidden(data->lines[i], 1);
        } else {
            lv_obj_set_hidden(data->lines[i], 0);
            if (stat_list[pos].time) {
                uint32_t time = *stat_list[pos].ptr;
                lv_label_set_text_fmt(data->lines[i], "%c%-9s:%ih%02im%02is", VIEW_COMMON_CURSOR(data->index, pos),
                                      view_intl_get_string(pmodel, stat_list[pos].desc), time / 3600, (time / 60) % 60,
                                      time % 60);
            } else {
                lv_label_set_text_fmt(data->lines[i], "%c%-9s: %i", VIEW_COMMON_CURSOR(data->index, pos),
                                      view_intl_get_string(pmodel, stat_list[pos].desc), *stat_list[i].ptr);
            }
        }
    }
}


static void *create_page(model_t *model, void *extra) {
    (void)TAG;
    return malloc(sizeof(struct page_data));
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *data = args;
    data->index            = 0;
    data->start            = 0;

    view_common_title(lv_scr_act(), view_intl_get_string(pmodel, STRINGS_STATISTICHE));
    data->task = view_register_periodic_task(500, LV_TASK_PRIO_HIGH, 0);

    for (size_t i = 0; i < ELEMENTS; i++) {
        lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
        lv_obj_set_auto_realign(lbl, 1);
        lv_obj_set_style(lbl, &style_label_6x8);
        lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 20 + 8 * i);
        data->lines[i] = lbl;
    }

    update_page(pmodel, data);
}


static view_message_t process_page_event(model_t *pmodel, void *arg, pman_event_t event) {
    view_message_t    msg  = VIEW_EMPTY_MSG;
    struct page_data *data = arg;

    switch (event.code) {
        case VIEW_EVENT_CODE_OPEN:
            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_UPDATE_STATISTICS;
            break;

        case VIEW_EVENT_CODE_MODEL_UPDATE:
            update_page(pmodel, data);
            break;

        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                switch (event.key_event.code) {
                    case BUTTON_STOP:
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
                        break;

                    case BUTTON_DESTRA:
                        data->index = (data->index + 1) % TOTAL_STATS;
                        update_page(pmodel, data);
                        break;

                    case BUTTON_SINISTRA:
                        if (data->index > 0) {
                            data->index--;
                        } else {
                            data->index = TOTAL_STATS - 1;
                        }
                        update_page(pmodel, data);
                        break;

                    case BUTTON_LINGUA:
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_UPDATE_STATISTICS;
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