#ifndef VIEW_COMMON_H_INCLUDED
#define VIEW_COMMON_H_INCLUDED

#include "view.h"
#include "peripherals/keyboard.h"


#define VIEW_PASSWORD_TIMEOUT      4000UL
#define VIEW_PASSWORD_MAX_SIZE     7
#define VIEW_PASSWORD_MINUS        ((button_t[]){BUTTON_MENO, BUTTON_MENO})
#define VIEW_PASSWORD_LANA         ((button_t[]){BUTTON_START, BUTTON_START, BUTTON_START, BUTTON_START})
#define VIEW_PASSWORD_SET_DATETIME ((button_t[]){BUTTON_MENU, BUTTON_MENU})
#define VIEW_PASSWORD_PLUS         ((button_t[]){BUTTON_PIU, BUTTON_PIU})
#define VIEW_PASSWORD_RIGHT        ((button_t[]){BUTTON_DESTRA, BUTTON_DESTRA})
#define VIEW_PASSWORD_LEFT         ((button_t[]){BUTTON_SINISTRA, BUTTON_SINISTRA})
#define VIEW_PASSWORD_TIEPIDO      ((button_t[]){BUTTON_PIU, BUTTON_PIU})
#define VIEW_PASSWORD_RESET        ((button_t[]){BUTTON_PIU, BUTTON_DESTRA, BUTTON_SINISTRA, BUTTON_MENO})
#define VIEW_SHORT_PASSWORD_LEN    2
#define VIEW_LONG_PASSWORD_LEN     4
#define VIEW_COMMON_CURSOR(i, pos) (i == pos ? '>' : ' ')

typedef struct {
    button_t      password[VIEW_PASSWORD_MAX_SIZE];
    size_t        index;
    unsigned long last_timestamp;
} view_common_password_t;


lv_obj_t   *view_common_title(lv_obj_t *root, const char *str);
void        view_common_password_add_key(view_common_password_t *inserted, button_t new, unsigned long timestamp);
int         view_common_check_password(view_common_password_t *inserted, button_t *password, size_t length,
                                       unsigned long timestamp);
void        view_common_password_reset(view_common_password_t *password, unsigned long timestamp);
lv_task_t  *view_common_register_timer(unsigned long period);
int         view_common_check_password_started(view_common_password_t *inserted);
void       *view_common_malloc_page_data(size_t size);
lv_obj_t   *view_common_horizontal_line(void);
lv_obj_t   *view_common_line(lv_point_t *points, size_t len);
const char *view_common_step2str(model_t *pmodel, uint16_t step);
const char *view_common_pedantic_string(model_t *pmodel);
lv_obj_t   *view_common_popup(lv_obj_t *root, lv_obj_t **content);
void        view_common_program_type_image(lv_obj_t *img, uint8_t ptype);
const char *view_common_alarm_description(model_t *pmodel);
int         view_common_update_alarm_popup(model_t *pmodel, uint16_t *alarm, unsigned long *timestamp, lv_obj_t *popup,
                                           lv_obj_t *label, lv_obj_t *lbl_code);
lv_obj_t   *view_common_alarm_popup(lv_obj_t **label, lv_obj_t **code);
void        view_common_set_hidden(lv_obj_t *obj, int hidden);
void        view_common_program_type_name(model_t *pmodel, lv_obj_t *lbl, uint8_t type);
lv_obj_t   *view_common_braking_popup(lv_obj_t **label, uint16_t language);

#endif