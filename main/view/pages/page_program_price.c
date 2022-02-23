#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
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


enum {
    TIMEOUT_TIMER_ID,
};


struct page_data {
    int        flag;
    lv_task_t *task;
    lv_obj_t * lprice;
    size_t     index;
};

static uint32_t take_int(uint8_t *digits);
static void     take_digits(uint8_t *res, uint32_t intero);
static void     price_update(model_t *pmodel, struct page_data *data);

static void update_page(model_t *pmodel, struct page_data *pdata);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->task             = view_register_periodic_task(500UL, LV_TASK_PRIO_OFF, TIMEOUT_TIMER_ID);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *data = args;

    view_common_title(lv_scr_act(), view_intl_get_string(pmodel, STRINGS_PREZZO));

    lv_task_set_prio(data->task, LV_TASK_PRIO_MID);
    data->flag  = 1;
    data->index = 0;

    lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(cont, 110, 20);
    lv_obj_align(cont, NULL, LV_ALIGN_CENTER, 0, 1);
    lv_obj_set_style(cont, &style_container_bordered);

    lv_obj_t *lprice = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lprice, 1);
    lv_obj_align(lprice, cont, LV_ALIGN_CENTER, 0, 0);
    data->lprice = lprice;

    uint8_t digits[6] = {0};
    take_digits(digits, pmodel->prog.programma_caricato.prezzo);
    lv_label_set_text_fmt(data->lprice, "%c%c%c%c%c%c", digits[5] + 48, digits[4] + 48, digits[3] + 48, digits[2] + 48,
                          digits[1] + 48, digits[0] + 48);
    update_page(pmodel, data);
}


static view_message_t process_page_event(model_t *pmodel, void *arg, view_event_t event) {
    view_message_t    msg  = VIEW_EMPTY_MSG;
    struct page_data *data = arg;
    (void)data;

    switch (event.code) {
        case VIEW_EVENT_CODE_PROGRAM_SAVED:
            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
            break;

        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                switch (event.key_event.code) {
                    case BUTTON_MENU:
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_SWAP_PAGE;
                        msg.vmsg.page = (void *)&page_program_type;
                        break;

                    case BUTTON_STOP: {
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_UPDATE_PROGRAM;
                        break;
                    }

                    case BUTTON_SINISTRA:     // skip sx
                        if (data->index < 5) {
                            data->index++;
                            data->flag = 1;
                            price_update(pmodel, data);
                        }
                        break;
                    case BUTTON_DESTRA:     // skip dx
                        if (data->index > 0) {
                            data->index--;
                            data->flag = 1;
                            price_update(pmodel, data);
                        }
                        break;
                    case BUTTON_PIU: {     // più
                        uint8_t digits[6] = {0};
                        take_digits(digits, pmodel->prog.programma_caricato.prezzo);
                        if (digits[data->index] < 9) {
                            digits[data->index]++;
                            pmodel->prog.programma_caricato.prezzo = take_int(digits);
                        } else if (digits[data->index] == 9) {
                            digits[data->index]                    = 0;
                            pmodel->prog.programma_caricato.prezzo = take_int(digits);
                        }
                        data->flag = 0;
                        price_update(pmodel, data);
                    }

                    break;
                    case BUTTON_MENO: {     // meno
                        uint8_t digits[6] = {0};
                        take_digits(digits, pmodel->prog.programma_caricato.prezzo);
                        if (digits[data->index] > 0) {
                            digits[data->index]--;
                            pmodel->prog.programma_caricato.prezzo = take_int(digits);
                        } else if (digits[data->index] == 0) {
                            digits[data->index]                    = 9;
                            pmodel->prog.programma_caricato.prezzo = take_int(digits);
                        }
                        data->flag = 0;
                        price_update(pmodel, data);

                    } break;
                    default:
                        break;
                }
            }
            break;
        }

        case VIEW_EVENT_CODE_TIMER: {
            price_update(pmodel, data);
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


static void update_page(model_t *pmodel, struct page_data *pdata) {}


const pman_page_t page_program_price = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = close_page,
    .destroy       = destroy_page,
};

static void take_digits(uint8_t *res, uint32_t intero) {
    for (size_t i = 0; i < 6; i++) {
        res[i] = ((int)intero % ((int)pow(10, (i + 1)))) / ((int)pow(10, i));
    }
}

static uint32_t take_int(uint8_t *digits) {
    uint32_t res = 0;
    for (size_t i = 0; i < 6; i++) {
        res += (digits[i]) * ((int)pow(10, i));
    }
    return res;
}

static void price_update(model_t *pmodel, struct page_data *data) {
    uint8_t digits[6] = {0};
    take_digits(digits, pmodel->prog.programma_caricato.prezzo);
    if (data->flag) {
        digits[data->index] = '_' - 48;
    }
    data->flag = !data->flag;

    lv_label_set_text_fmt(data->lprice, "%c%c%c%c%c%c", digits[5] + 48, digits[4] + 48, digits[3] + 48, digits[2] + 48,
                          digits[1] + 48, digits[0] + 48);
    lv_task_reset(data->task);
}