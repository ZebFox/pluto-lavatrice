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


#define NUM_LINES 3

#define OP_NONE 0
#define OP_ADD  1
#define OP_DEL  2


struct page_data {
    lv_obj_t *lbl_index;
    lv_obj_t *lines[NUM_LINES];
    lv_obj_t *lbl_addition;

    int      operation;
    uint16_t step_code;

    size_t start;
    size_t index;
};


static void update_page(model_t *pmodel, struct page_data *pdata);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->index            = 0;
    pdata->start            = 0;
    pdata->operation        = OP_NONE;
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;

    view_common_title(lv_scr_act(), view_intl_get_string(pmodel, STRINGS_PASSI_PROGRAMMA));

    lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_MID, 0, 15);
    pdata->lbl_index = lbl;

    lv_obj_t *line = view_common_horizontal_line();
    lv_obj_align(line, NULL, LV_ALIGN_IN_TOP_MID, 0, 24);

    for (size_t i = 0; i < NUM_LINES; i++) {
        lv_obj_t *lbl = lv_label_create(lv_scr_act(), NULL);
        lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 28 + 8 * i);
        lv_label_set_body_draw(lbl, 1);
        pdata->lines[i] = lbl;
    }

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(lbl, &style_label_6x8_reverse);
    lv_label_set_body_draw(lbl, 1);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    pdata->lbl_addition = lbl;

    update_page(pmodel, pdata);
}


static view_message_t process_page_event(model_t *pmodel, void *arg, view_event_t event) {
    view_message_t    msg   = VIEW_EMPTY_MSG;
    struct page_data *pdata = arg;

    switch (event.code) {
        case VIEW_EVENT_CODE_PROGRAM_SAVED:
            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
            break;

        case VIEW_EVENT_CODE_KEYPAD: {
            if (event.key_event.event == KEY_CLICK) {
                switch (event.key_event.code) {
                    case BUTTON_MENU:
                        break;

                    case BUTTON_STOP: {
                        if (pdata->operation) {
                            pdata->operation = OP_NONE;
                            update_page(pmodel, pdata);
                        } else {
                            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_UPDATE_PROGRAM;
                        }
                        break;
                    }

                    case BUTTON_DESTRA: {
                        if (model_get_program(pmodel)->num_steps == 0) {
                            break;
                        }

                        pdata->index = (pdata->index + 1) % (model_get_program(pmodel)->num_steps + 1);
                        update_page(pmodel, pdata);
                        break;
                    }

                    case BUTTON_SINISTRA: {
                        if (model_get_program(pmodel)->num_steps == 0) {
                            break;
                        }

                        if (pdata->index > 0) {
                            pdata->index--;
                        } else {
                            pdata->index = model_get_program(pmodel)->num_steps;
                        }
                        update_page(pmodel, pdata);
                        break;
                    }

                    case BUTTON_MENO: {
                        if (pdata->operation == OP_ADD) {
                            if (pdata->step_code > 0) {
                                pdata->step_code--;
                            } else {
                                pdata->step_code = NUM_STEPS - 1;
                            }
                            update_page(pmodel, pdata);
                        } else if (pdata->operation == OP_NONE && pdata->index > 0) {
                            pdata->operation = OP_DEL;
                            update_page(pmodel, pdata);
                        }
                        break;
                    }

                    case BUTTON_PIU: {
                        if (pdata->operation == OP_ADD) {
                            pdata->step_code = (pdata->step_code + 1) % NUM_STEPS;
                            update_page(pmodel, pdata);
                        } else if (pdata->operation == OP_NONE) {
                            if (model_get_program(pmodel)->num_steps < MAX_STEPS) {
                                pdata->operation = OP_ADD;
                                pdata->step_code = 0;
                                update_page(pmodel, pdata);
                            }
                        }
                        break;
                    }

                    case BUTTON_START:
                        break;

                    case BUTTON_LINGUA:
                        if (pdata->operation == OP_ADD) {
                            program_insert_step(model_get_program(pmodel), pdata->step_code + 1, pdata->index + 1);
                            pdata->index     = (pdata->index + 1) % (model_get_program(pmodel)->num_steps + 1);
                            pdata->operation = OP_NONE;
                            update_page(pmodel, pdata);
                        } else if (pdata->operation == OP_DEL) {
                            pdata->operation = OP_NONE;
                            programs_remove_step(model_get_program(pmodel), pdata->index - 1);
                            if (pdata->index >= model_get_program(pmodel)->num_steps) {
                                if (pdata->index > 0) {
                                    pdata->index--;
                                }
                            }
                            update_page(pmodel, pdata);
                        } else if (pdata->index > 0 && pdata->index < model_get_program(pmodel)->num_steps + 1) {
                            msg.vmsg.code  = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.page  = (void *)&page_parlav;
                            msg.vmsg.extra = &model_get_program(pmodel)->steps[pdata->index - 1];
                        }
                        break;
                }
            }
            break;
        }

        case VIEW_EVENT_CODE_TIMER:
            break;

        default:
            break;
    }
    return msg;
}


static void update_page(model_t *pmodel, struct page_data *pdata) {
    size_t num_steps = model_get_program(pmodel)->num_steps;

    if (pdata->index >= pdata->start + 3) {
        pdata->start = pdata->index - 2;
    } else if (pdata->index < pdata->start) {
        pdata->start = pdata->index;
    }

    if (num_steps == 0) {
        lv_label_set_text(pdata->lbl_index, view_intl_get_string(pmodel, STRINGS_PROGRAMMA_VUOTO));
        lv_obj_set_style(pdata->lines[0], &style_label_6x8_reverse);
        lv_obj_set_hidden(pdata->lines[0], 0);
        lv_label_set_text(pdata->lines[0], view_intl_get_string(pmodel, STRINGS_INIZIO_PROGRAMMA));
        for (size_t i = 1; i < NUM_LINES; i++) {
            lv_obj_set_hidden(pdata->lines[i], 1);
        }
    } else {
        lv_label_set_text_fmt(pdata->lbl_index, "%s: %02i/%02i", view_intl_get_string(pmodel, STRINGS_PASSO),
                              pdata->index, num_steps);
        for (size_t i = 0; i < NUM_LINES; i++) {
            size_t position = pdata->start + i;
            if (position == pdata->index) {
                lv_obj_set_style(pdata->lines[i], &style_label_6x8_reverse);
            } else {
                lv_obj_set_style(pdata->lines[i], &style_label_6x8);
            }
            if (position == 0) {
                lv_label_set_text(pdata->lines[i], view_intl_get_string(pmodel, STRINGS_INIZIO_PROGRAMMA));
            } else {
                parametri_step_t *step = model_get_program_step(pmodel, position - 1);
                if (step != NULL) {
                    lv_label_set_text(pdata->lines[i], view_common_step2str(pmodel, step->tipo));
                    lv_obj_set_hidden(pdata->lines[i], 0);
                } else {
                    lv_obj_set_hidden(pdata->lines[i], 1);
                }
            }
        }
    }

    if (pdata->operation == OP_ADD) {
        lv_obj_set_hidden(pdata->lbl_addition, 0);
        lv_label_set_text_fmt(pdata->lbl_addition, "+ %-18s ", view_common_step2str(pmodel, pdata->step_code + 1));
    } else if (pdata->operation == OP_DEL) {
        lv_obj_set_hidden(pdata->lbl_addition, 0);
        lv_label_set_text_fmt(pdata->lbl_addition, "- %-18s ",
                              view_common_step2str(pmodel, model_get_program_step(pmodel, pdata->index - 1)->tipo));
    } else {
        lv_obj_set_hidden(pdata->lbl_addition, 1);
    }
}


const pman_page_t page_program_steps = {
    .create        = create_page,
    .open          = open_page,
    .process_event = process_page_event,
    .close         = view_close_all,
    .destroy       = view_destroy_all,
};