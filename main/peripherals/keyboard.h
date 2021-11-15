
#ifndef KEYBOARD_H_INCLUDED
#define KEYBOARD_H_INCLUDED

#include <stdint.h>
#include "gel/keypad/keypad.h"

typedef enum {
    BUTTON_NONE  = 0,
    BUTTON_CALDO4 = 1,
    BUTTON_CALDO3,
    BUTTON_CALDO2,
    BUTTON_LANA,
    BUTTON_FREDDO = 5,
    BUTTON_STOP,
    BUTTON_MENU,
    BUTTON_LINGUA,
    BUTTON_STOP_LANA = 9,
    BUTTON_STOP_FREDDO,
} button_t;

void            keyboard_init(void);
unsigned int    keyboard_read(void);
keypad_update_t keyboard_manage(unsigned long ts);
void            keyboard_reset(void);

#endif
