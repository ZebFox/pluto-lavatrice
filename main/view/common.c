#include <assert.h>
#include "common.h"
#include "gel/timer/timecheck.h"
#include "styles.h"


static void timer_task(lv_task_t *task);
static int  find_password_start(view_common_password_t *password);


static const button_t preamble[3] = {BUTTON_STOP, BUTTON_STOP, BUTTON_STOP};


lv_obj_t *view_common_title(lv_obj_t *root, const char *str) {
    lv_obj_t *cont = lv_cont_create(root, NULL);

    static lv_style_t style;
    lv_style_copy(&style, lv_obj_get_style(cont));
    style.body.border.part    = LV_BORDER_BOTTOM | LV_BORDER_TOP;
    style.body.border.width   = 1;
    style.body.border.color   = LV_COLOR_BLACK;
    style.body.padding.inner  = 0;
    style.body.padding.top    = 2;
    style.body.padding.bottom = 2;
    style.text.letter_space   = 0;
    lv_obj_set_style(cont, &style);

    lv_obj_t *title = lv_label_create(cont, NULL);
    lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
    lv_label_set_long_mode(title, LV_LABEL_LONG_CROP);
    lv_label_set_text(title, str);
    lv_obj_set_width(title, LV_HOR_RES_MAX);

    lv_cont_set_fit2(cont, LV_FIT_NONE, LV_FIT_TIGHT);
    lv_obj_set_width(cont, LV_HOR_RES_MAX);
    lv_obj_align(cont, NULL, LV_ALIGN_IN_TOP_MID, 0, 2);
    lv_obj_align(title, NULL, LV_ALIGN_CENTER, 0, 0);

    return title;
}


void view_common_password_add_key(view_common_password_t *inserted, button_t new, unsigned long timestamp) {
    if (is_expired(inserted->last_timestamp, timestamp, VIEW_PASSWORD_TIMEOUT)) {
        view_common_password_reset(inserted, timestamp);
    }
    inserted->password[inserted->index] = new;
    inserted->index                     = (inserted->index + 1) % VIEW_PASSWORD_MAX_SIZE;
    inserted->last_timestamp            = timestamp;
}


void view_common_password_reset(view_common_password_t *inserted, unsigned long timestamp) {
    size_t i = 0;
    for (i = 0; i < VIEW_PASSWORD_MAX_SIZE; i++) {
        inserted->password[i] = BUTTON_NONE;
    }
    inserted->index          = 0;
    inserted->last_timestamp = timestamp;
}


int view_common_check_password(view_common_password_t *inserted, button_t *password, size_t length,
                               unsigned long timestamp) {
    if (is_expired(inserted->last_timestamp, timestamp, VIEW_PASSWORD_TIMEOUT)) {
        view_common_password_reset(inserted, timestamp);
        return 0;
    } else if (length + sizeof(preamble) / sizeof(preamble[0]) > VIEW_PASSWORD_MAX_SIZE) {
        return 0;
    }


    size_t i = 0;

    int res = find_password_start(inserted);
    if (res < 0) {
        return 0;
    }

    size_t start = (size_t)res;
    for (i = 0; i < length; i++) {
        if (inserted->password[(start + i) % VIEW_PASSWORD_MAX_SIZE] != password[i]) {
            return 0;
        }
    }

    return 1;
}


int view_common_check_password_started(view_common_password_t *inserted) {
    int res = find_password_start(inserted);
    return res >= 0;
}


lv_task_t *view_common_register_timer(unsigned long period) {
    return lv_task_create(timer_task, period, LV_TASK_PRIO_OFF, NULL);
}


lv_obj_t *view_common_horizontal_line(void) {
    /*Create an array for the points of the line*/
    static lv_point_t line_points[] = {{0, 0}, {128, 0}};

    /*Create new style (thick dark blue)*/
    static lv_style_t style_line;
    lv_style_copy(&style_line, &lv_style_plain);
    style_line.line.color   = LV_COLOR_BLACK;
    style_line.line.width   = 1;
    style_line.line.rounded = 0;

    /*Copy the previous line and apply the new style*/
    lv_obj_t *line1;
    line1 = lv_line_create(lv_scr_act(), NULL);
    lv_line_set_points(line1, line_points, 2); /*Set the points*/
    lv_line_set_style(line1, LV_LINE_STYLE_MAIN, &style_line);
    return line1;
}


static void timer_task(lv_task_t *task) {
    (void)task;
    view_event((view_event_t){.code = VIEW_EVENT_CODE_TIMER});
}


static int find_password_start(view_common_password_t *password) {
    int    result = -1;
    size_t i = 0, start = 0;

    for (start = 0; start < VIEW_PASSWORD_MAX_SIZE; start++) {
        int found = 1;

        for (i = 0; i < sizeof(preamble) / sizeof(preamble[0]); i++) {
            if (password->password[(start + i) % VIEW_PASSWORD_MAX_SIZE] != preamble[i]) {
                found = 0;
                break;
            }
        }

        if (found) {
            result = start + 3;
            // Do not break this cycle; we consider the latest possible password
        }
    }

    return result;
}