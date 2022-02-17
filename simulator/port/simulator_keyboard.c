#include <SDL2/SDL.h>
#include "peripherals/keyboard.h"

const uint8_t *keystates     = NULL;
static int     ignore_events = 0;


static keypad_key_t keyboard[] = {
    KEYPAD_KEY(0x01, BUTTON_MEDIO), KEYPAD_KEY(0x02, BUTTON_TIEPIDO), KEYPAD_KEY(0x04, BUTTON_CALDO),
    KEYPAD_KEY(0x08, BUTTON_LANA),  KEYPAD_KEY(0x10, BUTTON_FREDDO),  KEYPAD_KEY(0x40, BUTTON_LINGUA),
    KEYPAD_KEY(0x80, BUTTON_MENU),  KEYPAD_KEY(0x100, BUTTON_STOP),   KEYPAD_NULL_KEY,
};


void keyboard_reset(void) {
    ignore_events = 1;
}


void keyboard_init(void) {
    keystates = SDL_GetKeyboardState(NULL);
}


unsigned int keyboard_read(void) {
    static unsigned int input = 0;

    SDL_PumpEvents();
    if (keystates[SDL_SCANCODE_1] || keystates[SDL_SCANCODE_KP_1]) {
        input |= 0x100;
    } else {
        input &= ~0x100;
    }
    if (keystates[SDL_SCANCODE_3] || keystates[SDL_SCANCODE_KP_3]) {
        input |= 0x40;
    } else {
        input &= ~0x40;
    }
    if (keystates[SDL_SCANCODE_4] || keystates[SDL_SCANCODE_KP_4]) {
        input |= 0x20;
    } else {
        input &= ~0x20;
    }
    if (keystates[SDL_SCANCODE_5] || keystates[SDL_SCANCODE_KP_5]) {
        input |= 0x10;
    } else {
        input &= ~0x10;
    }
    if (keystates[SDL_SCANCODE_6] || keystates[SDL_SCANCODE_KP_6]) {
        input |= 0x8;
    } else {
        input &= ~0x8;
    }
    if (keystates[SDL_SCANCODE_7] || keystates[SDL_SCANCODE_KP_7]) {
        input |= 0x4;
    } else {
        input &= ~0x4;
    }
    if (keystates[SDL_SCANCODE_8] || keystates[SDL_SCANCODE_KP_8]) {
        input |= 0x2;
    } else {
        input &= ~0x2;
    }
    if (keystates[SDL_SCANCODE_9] || keystates[SDL_SCANCODE_KP_9]) {
        input |= 0x1;
    } else {
        input &= ~0x1;
    }

    return input;
}


keypad_update_t keyboard_manage(unsigned long ts) {
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
