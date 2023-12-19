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


#define KEYBOARD_CAPS  0
#define KEYBOARD_LOWER 1
#define KEYBOARD_NUM   2
#define NUM_KEYBOARDS  3

enum {
    TIMEOUT_TIMER_ID,
};


struct page_data {
    size_t    cindex;
    size_t    rindex;
    lv_obj_t *labels[4][10];
    lv_obj_t *lbl_name;
    uint16_t  language;
    uint16_t  keyboard;

    int flag;

    lv_task_t *task;
};


static void labels_set_text(struct page_data *data);
static void update_page(model_t *pmodel, struct page_data *pdata);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->task             = view_register_periodic_task(500UL, LV_TASK_PRIO_OFF, TIMEOUT_TIMER_ID);
    pdata->language         = model_get_language(pmodel);
    pdata->keyboard         = KEYBOARD_CAPS;
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *data = args;

    lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(cont, 110, 20);
    lv_obj_align(cont, NULL, LV_ALIGN_IN_TOP_MID, 0, 1);
    lv_obj_set_style(cont, &style_container_bordered);

    lv_obj_t *lbl = lv_label_create(cont, NULL);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(lbl, 108);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 2, 1);
    lv_obj_set_style(lbl, &style_label_6x8);
    data->lbl_name = lbl;

    lv_task_set_prio(data->task, LV_TASK_PRIO_MID);

    data->rindex = 0;
    data->cindex = 0;
    data->flag   = 1;

    lv_obj_t *labels[4][10];

    for (size_t i = 0; i < 4; i++) {
        labels[i][0] = lv_label_create(lv_scr_act(), NULL);
        for (size_t j = 1; j < 10; j++) {
            if ((i == 0) || (i == 1 && j != 9) || (i == 2 && j <= 6) || (i == 3 && j <= 3)) {
                labels[i][j] = lv_label_create(lv_scr_act(), NULL);
            } else {
                labels[i][j] = NULL;
            }
        }
    }

    for (size_t i = 0; i < 4; i++) {
        lv_obj_set_style(labels[3][i], &style_label_6x8);
    }

    for (size_t i = 0; i < 4; i++) {
        for (size_t j = 0; j < 10; j++) {
            data->labels[i][j] = labels[i][j];
        }
    }

    labels_set_text(data);
    lv_obj_align(data->labels[2][0], NULL, LV_ALIGN_CENTER, -32, 15);
    lv_obj_align(data->labels[1][0], NULL, LV_ALIGN_CENTER, -40, 5);
    lv_obj_align(data->labels[0][0], NULL, LV_ALIGN_CENTER, -50, -5);
    lv_obj_align(data->labels[3][0], NULL, LV_ALIGN_IN_BOTTOM_LEFT, 12, -1);
    lv_label_set_body_draw(labels[0][0], 1);
    lv_label_set_body_draw(labels[1][0], 1);
    lv_label_set_body_draw(labels[2][0], 1);
    lv_label_set_body_draw(labels[3][0], 1);

    for (size_t i = 0; i < 4; i++) {
        for (size_t j = 1; j < 10; j++) {
            const int spacing = i == 3 ? 6 : 3;
            if (labels[i][j] == NULL) {
                continue;
            }
            lv_obj_align(data->labels[i][j], data->labels[i][j - 1], LV_ALIGN_OUT_RIGHT_MID, spacing, 0);
            lv_label_set_body_draw(labels[i][j], 1);
        }
    }

    update_page(pmodel, data);
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
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_SAVE_PARMAC;
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
                        break;
                    }

                    case BUTTON_MENU:
                        break;

                    case BUTTON_DESTRA: {
                        switch (data->rindex) {
                            case (0):
                                data->cindex = (data->cindex + 1) % 10;
                                break;
                            case (1):
                                data->cindex = (data->cindex + 1) % 9;
                                break;
                            case (2):
                                data->cindex = (data->cindex + 1) % 7;
                                break;
                            case (3):
                                data->cindex = (data->cindex + 1) % 4;
                                break;
                        }
                        update_page(pmodel, data);
                        break;
                    }

                    case BUTTON_SINISTRA: {
                        switch (data->rindex) {
                            case (0):
                                if (data->cindex > 0) {
                                    data->cindex--;
                                } else {
                                    data->cindex = 9;
                                }
                                break;
                            case (1):
                                if (data->cindex > 0) {
                                    data->cindex--;
                                } else if (data->cindex == 0) {
                                    data->cindex = 8;
                                }
                                break;
                            case (2):
                                if (data->cindex > 0) {
                                    data->cindex--;
                                } else if (data->cindex == 0) {
                                    data->cindex = 6;
                                }
                                break;
                            case (3):
                                if (data->cindex > 0) {
                                    data->cindex--;
                                } else if (data->cindex == 0) {
                                    data->cindex = 3;
                                }
                                break;

                            default:
                                break;
                        }
                        update_page(pmodel, data);
                        break;
                    }

                    case BUTTON_PIU: {
                        if (data->rindex < 3) {
                            data->rindex++;
                            if (data->labels[data->rindex][data->cindex] == NULL) {
                                do {
                                    data->cindex--;
                                } while (data->labels[data->rindex][data->cindex] == NULL);
                            }
                        } else {
                            data->rindex = 0;
                        }
                        update_page(pmodel, data);
                        break;
                    }

                    case BUTTON_MENO: {
                        if (data->rindex > 0) {
                            data->rindex--;
                            if (data->labels[data->rindex][data->cindex] == NULL) {
                                do {
                                    data->cindex--;
                                } while (data->labels[data->rindex][data->cindex] == NULL);
                            }
                        } else {
                            data->rindex = 3;
                        }
                        update_page(pmodel, data);
                        break;
                    }

                    case BUTTON_START:
                        data->keyboard = (data->keyboard + 1) % NUM_KEYBOARDS;
                        labels_set_text(data);
                        break;

                    case BUTTON_LINGUA: {
                        char  *name     = pmodel->prog.parmac.nome;
                        size_t name_len = strlen(name);

                        if (data->rindex == 3) {
                            // Cambio lingua
                            if (data->cindex == 0) {
                                data->language = (data->language + 1) % NUM_LINGUE;
                            }
                            // Spazio
                            else if (data->cindex == 1) {
                                if (name_len < MAX_NAME_LENGTH) {
                                    strcat(name, " ");
                                }
                            }
                            // Cancella
                            else if (data->cindex == 2) {
                                name[name_len - 1] = '\0';

                            }
                            // Azzera
                            else if (data->cindex == 3) {
                                name[0] = '\0';
                            }
                            data->flag = 1;
                            update_page(pmodel, data);
                        } else if (name_len < MAX_NAME_LENGTH) {
                            strcat(name, lv_label_get_text(data->labels[data->rindex][data->cindex]));
                            data->flag = 1;
                            update_page(pmodel, data);
                        }
                        break;
                    }
                }
            }
            break;
        }
        case VIEW_EVENT_CODE_TIMER: {
            lv_task_reset(data->task);
            data->flag = !data->flag;
            update_page(pmodel, data);
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


static void update_page(model_t *pmodel, struct page_data *pdata) {
    for (size_t i = 0; i < 4; i++) {
        for (size_t j = 0; j < 10; j++) {
            if (pdata->labels[i][j] == NULL) {
                continue;
            }
            if (i == pdata->rindex && j == pdata->cindex) {
                lv_obj_set_style(pdata->labels[i][j], i == 3 ? &style_label_6x8_reverse : &style_label_reverse);
            } else {
                lv_obj_set_style(pdata->labels[i][j], i == 3 ? &style_label_6x8 : &style_label_normal);
            }
        }
    }

    char  *name     = pmodel->prog.parmac.nome;
    size_t name_len = strlen(name);
    lv_label_set_text_fmt(pdata->lbl_name, "%s%c", name, (pdata->flag && name_len < MAX_NAME_LENGTH) ? '_' : ' ');

    switch (pdata->language) {
        case 0:
            lv_label_set_text(pdata->labels[3][0], "[IT]");
            break;

        case 1:
            lv_label_set_text(pdata->labels[3][0], "[EN]");
            break;

        default:
            assert(0);
    }
}


const pman_page_t page_machine_name = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};

static void labels_set_text(struct page_data *data) {
    switch (data->keyboard) {
        case KEYBOARD_CAPS:
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
            break;

        case KEYBOARD_LOWER:
            lv_label_set_text(data->labels[0][0], "q");
            lv_label_set_text(data->labels[0][1], "w");
            lv_label_set_text(data->labels[0][2], "e");
            lv_label_set_text(data->labels[0][3], "r");
            lv_label_set_text(data->labels[0][4], "t");
            lv_label_set_text(data->labels[0][5], "y");
            lv_label_set_text(data->labels[0][6], "u");
            lv_label_set_text(data->labels[0][7], "i");
            lv_label_set_text(data->labels[0][8], "o");
            lv_label_set_text(data->labels[0][9], "p");

            lv_label_set_text(data->labels[1][0], "a");
            lv_label_set_text(data->labels[1][1], "s");
            lv_label_set_text(data->labels[1][2], "d");
            lv_label_set_text(data->labels[1][3], "f");
            lv_label_set_text(data->labels[1][4], "g");
            lv_label_set_text(data->labels[1][5], "h");
            lv_label_set_text(data->labels[1][6], "j");
            lv_label_set_text(data->labels[1][7], "k");
            lv_label_set_text(data->labels[1][8], "l");

            lv_label_set_text(data->labels[2][0], "z");
            lv_label_set_text(data->labels[2][1], "x");
            lv_label_set_text(data->labels[2][2], "c");
            lv_label_set_text(data->labels[2][3], "v");
            lv_label_set_text(data->labels[2][4], "b");
            lv_label_set_text(data->labels[2][5], "n");
            lv_label_set_text(data->labels[2][6], "m");
            break;

        case KEYBOARD_NUM:
            lv_label_set_text(data->labels[0][0], "1");
            lv_label_set_text(data->labels[0][1], "2");
            lv_label_set_text(data->labels[0][2], "3");
            lv_label_set_text(data->labels[0][3], "4");
            lv_label_set_text(data->labels[0][4], "5");
            lv_label_set_text(data->labels[0][5], "6");
            lv_label_set_text(data->labels[0][6], "7");
            lv_label_set_text(data->labels[0][7], "8");
            lv_label_set_text(data->labels[0][8], "9");
            lv_label_set_text(data->labels[0][9], "0");

            lv_label_set_text(data->labels[1][0], ";");
            lv_label_set_text(data->labels[1][1], ":");
            lv_label_set_text(data->labels[1][2], "'");
            lv_label_set_text(data->labels[1][3], "\"");
            lv_label_set_text(data->labels[1][4], ",");
            lv_label_set_text(data->labels[1][5], ".");
            lv_label_set_text(data->labels[1][6], "/");
            lv_label_set_text(data->labels[1][7], "?");
            lv_label_set_text(data->labels[1][8], "!");

            lv_label_set_text(data->labels[2][0], "@");
            lv_label_set_text(data->labels[2][1], "#");
            lv_label_set_text(data->labels[2][2], "%");
            lv_label_set_text(data->labels[2][3], "&");
            lv_label_set_text(data->labels[2][4], "*");
            lv_label_set_text(data->labels[2][5], "(");
            lv_label_set_text(data->labels[2][6], ")");
            break;

        default:
            assert(0);
    }

    lv_label_set_text(data->labels[3][1], "space");
    lv_label_set_text(data->labels[3][2], "del");
    lv_label_set_text(data->labels[3][3], "azz");
}
