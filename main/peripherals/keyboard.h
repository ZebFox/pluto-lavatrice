
#ifndef KEYBOARD_H_INCLUDED
#define KEYBOARD_H_INCLUDED

#include <stdint.h>
#include "gel/keypad/keypad.h"

#define BUTTON_MENO BUTTON_FREDDO
#define BUTTON_PIU BUTTON_MEDIO

typedef enum {
    BUTTON_NONE  = 0,
    BUTTON_CALDO = 1,
    BUTTON_TIEPIDO,
    BUTTON_MEDIO,
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
