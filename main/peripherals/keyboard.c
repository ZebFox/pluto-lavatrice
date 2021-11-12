/*
 * File:   keypad.c
 * Author: Virginia
 *
 * Created on 17 aprile 2021, 12.38
 */

#include "peripherals/keyboard.h"
#include "peripherals/hardwareprofile.h"
#include "gel/debounce/debounce.h"

static debounce_filter_t filter;
static int               ignore_events = 0;


static keypad_key_t keyboard[] = {
    KEYPAD_KEY(0x01, BUTTON_MEDIO),
    KEYPAD_KEY(0x02, BUTTON_PADLOCK),
    KEYPAD_KEY(0x04, BUTTON_CALDO),
    KEYPAD_KEY(0x08, BUTTON_FREDDO),
    KEYPAD_KEY(0x10, BUTTON_PLAY),
    KEYPAD_KEY(0x20, BUTTON_PLUS),
    KEYPAD_KEY(0x40, BUTTON_LINGUA),
    KEYPAD_KEY(0x80, BUTTON_MENU),
    KEYPAD_KEY(0x100, BUTTON_STOP),

    KEYPAD_KEY(0x60, BUTTON_LINGUA_TIEPIDO),
    KEYPAD_KEY(0x180, BUTTON_STOP_MENU),
    KEYPAD_KEY(0x110, BUTTON_STOP_LANA),
    KEYPAD_KEY(0x108, BUTTON_STOP_FREDDO),

    KEYPAD_NULL_KEY,
};

void keyboard_init(void) {
    /*KEYBOARD_RIGA1_TRIS = TRIS_OUTPUT;
    KEYBOARD_RIGA2_TRIS = TRIS_OUTPUT;
    KEYBOARD_RIGA3_TRIS = TRIS_OUTPUT;
    KEYBOARD_COL1_TRIS  = TRIS_INPUT;
    KEYBOARD_COL2_TRIS  = TRIS_INPUT;
    KEYBOARD_COL3_TRIS  = TRIS_INPUT;

    KEYBOARD_RIGA1_LAT = 0;
    KEYBOARD_RIGA2_LAT = 0;
    KEYBOARD_RIGA3_LAT = 0;*/

    debounce_filter_init(&filter);
}


void keyboard_reset(void) {
    ignore_events = 1;
}


unsigned int keyboard_read(void) {

    unsigned int res = 0;

    /*KEYBOARD_RIGA1_LAT = 1;
    __delay_us(1);
    res |= KEYBOARD_COL1_PORT;
    res |= KEYBOARD_COL2_PORT << 1;
    res |= KEYBOARD_COL3_PORT << 2;
    KEYBOARD_RIGA1_LAT = 0;

    KEYBOARD_RIGA2_LAT = 1;
    __delay_us(1);
    res |= KEYBOARD_COL1_PORT << 3;
    res |= KEYBOARD_COL2_PORT << 4;
    res |= KEYBOARD_COL3_PORT << 5;
    KEYBOARD_RIGA2_LAT = 0;

    KEYBOARD_RIGA3_LAT = 1;
    __delay_us(1);
    res |= KEYBOARD_COL1_PORT << 6;
    res |= KEYBOARD_COL2_PORT << 7;
    res |= KEYBOARD_COL3_PORT << 8;
    KEYBOARD_RIGA3_LAT = 0;*/

    return res;
}


keypad_update_t keyboard_manage(unsigned long ts) {
    // unsigned int input=0;
    // input=keyboard_read();
    // debounce_filter(&filter, input, 2);

    unsigned int keymap = keyboard_read();
    if (ignore_events) {
        if (keymap == 0) {
            ignore_events = 0;
            keypad_reset_keys(keyboard);
        }
        return (keypad_update_t){.event = KEY_NOTHING};
    } else {
        return keypad_routine(keyboard, 40, 1500, 100, ts, keymap);
    }
}
