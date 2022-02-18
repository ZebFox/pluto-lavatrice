#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "lvgl/lvgl.h"
#include "src/lv_core/lv_obj.h"
#include "src/lv_misc/lv_task.h"
#include "src/lv_objx/lv_label.h"
#include "view/styles.h"
#include "view/view.h"
#include "view/images/legacy.h"
#include "view/view_types.h"
#include "view/widgets/custom_lv_img.h"
#include "gel/pagemanager/page_manager.h"
#include "peripherals/keyboard.h"
#include "view/common.h"
#include "view/intl/intl.h"
#include "utils/utils.h"
#include "esp_log.h"


// enum { };

enum {
    TIMEOUT_TIMER_ID,
};


struct page_data {
    size_t    cindex;
    size_t    rindex;
    lv_obj_t *labels[4][10];

    int flag;

    lv_task_t *task;
    name_t     names[MAX_LINGUE];
};

static void labels_set_text(struct page_data *data);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->task             = view_register_periodic_task(500UL, LV_TASK_PRIO_OFF, TIMEOUT_TIMER_ID);
    return pdata;
}



static void open_page(model_t *pmodel, void *args) {
    struct page_data *data = args;

    lv_task_set_prio(data->task, LV_TASK_PRIO_MID);

    data->rindex = 0;
    data->cindex = 0;

    data->flag = 1;

    lv_obj_t *labels[4][10];

    for (int i = 0; i < 4; i++) {
        labels[i][0] = lv_label_create(lv_scr_act(), NULL);
        for (int j = 1; j < 10; j++) {
            if ((i == 0) || (i == 1 && j != 9) || (i == 2 && j <= 6) || (i == 3 && j <= 3)) {
                labels[i][j] = lv_label_create(lv_scr_act(), NULL);
            } else {
                labels[i][j] = NULL;
            }
        }
    }


    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 10; j++) {
            data->labels[i][j] = labels[i][j];
        }
    }

    labels_set_text(data);
    lv_obj_align(data->labels[2][0], NULL, LV_ALIGN_CENTER, -30, 10);
    lv_obj_align(data->labels[1][0], NULL, LV_ALIGN_CENTER, -37, 0);
    lv_obj_align(data->labels[0][0], NULL, LV_ALIGN_CENTER, -47, -10);
    lv_obj_align(data->labels[3][0], NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    lv_label_set_body_draw(labels[0][0], 1);
    lv_label_set_body_draw(labels[1][0], 1);
    lv_label_set_body_draw(labels[2][0], 1);
    lv_label_set_body_draw(labels[3][0], 1);

    for (int i = 0; i < 4; i++) {
        for (int j = 1; j < 10; j++) {
            if (labels[i][j] == NULL) {
                continue;
            }
            lv_obj_align(data->labels[i][j], data->labels[i][j - 1], LV_ALIGN_OUT_RIGHT_MID, 3, 0);
            lv_label_set_body_draw(labels[i][j], 1);
        }
    }
}


static view_message_t process_page_event(model_t *pmodel, void *arg, view_event_t event) {
    view_message_t    msg  = VIEW_EMPTY_MSG;
    struct page_data *data = arg;
    (void)data;

    switch (event.code) {
        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {

                switch (event.key_event.code) {
                    case BUTTON_STOP: {
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
                        break;
                    }
                    case BUTTON_DESTRA: {
                        switch (data->rindex) {
                            case (0):
                                if (data->cindex < 9) {
                                    data->cindex++;
                                } else if (data->cindex == 9) {
                                    data->cindex = 0;
                                    data->rindex++;
                                }
                                break;
                            case (1):
                                if (data->cindex < 8) {
                                    data->cindex++;
                                } else if (data->cindex == 8) {
                                    data->cindex = 0;
                                    data->rindex++;
                                }
                                break;
                            case (2):
                                if (data->cindex < 6) {
                                    data->cindex++;
                                } else if (data->cindex == 6) {
                                    data->cindex = 0;
                                    data->rindex++;
                                }
                                break;
                            case (3):
                                if (data->cindex < 3) {
                                    data->cindex++;
                                }
                                break;
                        }
                        break;
                    }

                    case BUTTON_CALDO: {
                        switch (data->rindex) {
                            case (0):
                                if (data->cindex > 0) {
                                    data->cindex--;
                                }
                                break;
                            case (1):
                                if (data->cindex > 0) {
                                    data->cindex--;
                                } else if (data->cindex == 0) {
                                    data->cindex = 9;
                                    data->rindex--;
                                }
                                break;
                            case (2):
                                if (data->cindex > 0) {
                                    data->cindex--;
                                } else if (data->cindex == 0) {
                                    data->cindex = 8;
                                    data->rindex--;
                                }
                                break;
                            case (3):
                                if (data->cindex > 0) {
                                    data->cindex--;
                                } else if (data->cindex == 0) {
                                    data->cindex = 6;
                                    data->rindex--;
                                }
                                break;

                            default:
                                break;
                        }
                        break;
                    } break;
                    case BUTTON_FREDDO: {
                        if (data->rindex < 3) {
                            data->rindex++;
                            if (data->labels[data->rindex][data->cindex] == NULL) {
                                do {
                                    data->cindex--;
                                } while (data->labels[data->rindex][data->cindex] == NULL);
                            }
                        }
                    } break;
                    case BUTTON_PIU: {
                        if (data->rindex > 0) {
                            data->rindex--;
                            if (data->labels[data->rindex][data->cindex] == NULL) {
                                do {
                                    data->cindex--;
                                } while (data->labels[data->rindex][data->cindex] == NULL);
                            }
                        }
                    } break;
                }
            }
        }
        case VIEW_EVENT_CODE_TIMER: {
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 10; j++) {
                    if (data->labels[i][j] == NULL) {
                        continue;
                    }
                    if (i == data->rindex && j == data->cindex) {
                        lv_obj_set_style(data->labels[i][j], &style_label_reverse);
                    } else {
                        lv_obj_set_style(data->labels[i][j], &style_label_normal);
                    }
                }
            }
            lv_task_reset(data->task);
        }

        break;

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


const pman_page_t page_program_name = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};

static void labels_set_text(struct page_data *data) {
    lv_label_set_text(data->labels[0][0], "Q");
    lv_label_set_text(data->labels[0][1], "W");
    lv_label_set_text(data->labels[0][2], "E");
    lv_label_set_text(data->labels[0][3], "R");
    lv_label_set_text(data->labels[0][4], "T");
    lv_label_set_text(data->labels[0][5], "Y");
    lv_label_set_text(data->labels[0][6], "U");
    lv_label_set_text(data->labels[0][7], "I");
    lv_label_set_text(data->labels[0][8], "O");
    lv_label_set_text(data->labels[0][9], "P");

    lv_label_set_text(data->labels[1][0], "A");
    lv_label_set_text(data->labels[1][1], "S");
    lv_label_set_text(data->labels[1][2], "D");
    lv_label_set_text(data->labels[1][3], "F");
    lv_label_set_text(data->labels[1][4], "G");
    lv_label_set_text(data->labels[1][5], "H");
    lv_label_set_text(data->labels[1][6], "J");
    lv_label_set_text(data->labels[1][7], "K");
    lv_label_set_text(data->labels[1][8], "L");

    lv_label_set_text(data->labels[2][0], "Z");
    lv_label_set_text(data->labels[2][1], "X");
    lv_label_set_text(data->labels[2][2], "C");
    lv_label_set_text(data->labels[2][3], "V");
    lv_label_set_text(data->labels[2][4], "B");
    lv_label_set_text(data->labels[2][5], "N");
    lv_label_set_text(data->labels[2][6], "M");

    lv_label_set_text(data->labels[3][0], "[EN]");
    lv_label_set_text(data->labels[3][1], "space");
    lv_label_set_text(data->labels[3][2], "del");
    lv_label_set_text(data->labels[3][3], "azz");
}