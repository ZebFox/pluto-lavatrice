
#ifndef KEYBOARD_H_INCLUDED
#define KEYBOARD_H_INCLUDED

#include <stdint.h>
#include "gel/keypad/keypad.h"


typedef enum {
    BUTTON_NONE  = 0,
    BUTTON_SINISTRA,
    BUTTON_KEY,
    BUTTON_DESTRA,
    BUTTON_PIU,
    BUTTON_START,
    BUTTON_MENO,
    BUTTON_STOP,
    BUTTON_MENU,
    BUTTON_LINGUA,
    BUTTON_STOP_PIU,
    BUTTON_STOP_MENO,
    BUTTON_STOP_START,
    BUTTON_STOP_KEY,
} button_t;

void            keyboard_init(void);
unsigned int    keyboard_read(void);
keypad_update_t keyboard_manage(unsigned long ts);
void            keyboard_reset(void);

#endif
