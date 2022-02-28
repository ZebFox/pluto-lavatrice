/*
 * File:   keypad.c
 * Author: Virginia
 *
 * Created on 17 aprile 2021, 12.38
 */

#include "peripherals/keyboard.h"
#include "peripherals/hardwareprofile.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "freertos/timers.h"

static int               ignore_events = 0;

#define GPIO_INPUT_PIN_SEL  ((1ULL<<HAL_BUTTON_IN_1) | (1ULL<<HAL_BUTTON_IN_2) | (1ULL<<HAL_BUTTON_IN_3) | (1ULL<<HAL_BUTTON_IN_4) )
#define GPIO_OUTPUT_PIN_SEL ((1ULL<<HAL_BUTTON_OUT_1) | (1ULL<<HAL_BUTTON_OUT_2) | (1ULL<<HAL_BUTTON_OUT_3) | (1ULL<<HAL_BUTTON_OUT_4) )


static const char *TAG = "keyboard";

static keypad_key_t keyboard[] = {
    KEYPAD_KEY(0x6000, BUTTON_STOP_MENU),
    KEYPAD_KEY(0x8000, BUTTON_LINGUA),
    KEYPAD_KEY(0x800, BUTTON_MENO), //oK
    KEYPAD_KEY(0x80, BUTTON_DESTRA), //OK
    KEYPAD_KEY(0x4000, BUTTON_MENU), //OK
    KEYPAD_KEY(0x400, BUTTON_START),
    KEYPAD_KEY(0x40, BUTTON_KEY),
    KEYPAD_KEY(0x2000, BUTTON_STOP), //OK
    KEYPAD_KEY(0x200, BUTTON_PIU), //OK
    KEYPAD_KEY(0x20, BUTTON_SINISTRA), //OK
    KEYPAD_NULL_KEY,
};

void keyboard_init(void) {
    (void)TAG;
//zero-initialize the config structure.
    gpio_config_t io_conf_input = {};
    //disable interrupt
    io_conf_input.intr_type = GPIO_INTR_DISABLE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf_input.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf_input.mode = GPIO_MODE_INPUT;
    //disable pull-down mode
    io_conf_input.pull_down_en = 0;
    //disable pull-up mode
    io_conf_input.pull_up_en = 0;
    gpio_config(&io_conf_input);

    //zero-initialize the config structure.
    gpio_config_t io_conf_output = {};
    //disable interrupt
    io_conf_output.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf_output.mode = GPIO_MODE_INPUT_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf_output.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf_output.pull_down_en = 0;
    //disable pull-up mode
    io_conf_output.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf_output);
}

void keyboard_reset(void) {
    ignore_events = 1;
}

unsigned int keyboard_read(void) {

    unsigned int res = 0;

    gpio_set_level(HAL_BUTTON_OUT_1, 1);
    res |= gpio_get_level(HAL_BUTTON_IN_1);
    res |= gpio_get_level(HAL_BUTTON_IN_2)<<1;
    res |= gpio_get_level(HAL_BUTTON_IN_3)<<2;
    res |= gpio_get_level(HAL_BUTTON_IN_4)<<3;
    gpio_set_level(HAL_BUTTON_OUT_1, 0);

    gpio_set_level(HAL_BUTTON_OUT_2, 1);
    res |= gpio_get_level(HAL_BUTTON_IN_1)<<4;
    res |= gpio_get_level(HAL_BUTTON_IN_2)<<5;
    res |= gpio_get_level(HAL_BUTTON_IN_3)<<6;
    res |= gpio_get_level(HAL_BUTTON_IN_4)<<7;
    gpio_set_level(HAL_BUTTON_OUT_2, 0);

    gpio_set_level(HAL_BUTTON_OUT_3, 1);
    res |= gpio_get_level(HAL_BUTTON_IN_1)<<8;
    res |= gpio_get_level(HAL_BUTTON_IN_2)<<9;
    res |= gpio_get_level(HAL_BUTTON_IN_3)<<10;
    res |= gpio_get_level(HAL_BUTTON_IN_4)<<11;
    gpio_set_level(HAL_BUTTON_OUT_3, 0);

        gpio_set_level(HAL_BUTTON_OUT_4, 1);
    res |= gpio_get_level(HAL_BUTTON_IN_1)<<12;
    res |= gpio_get_level(HAL_BUTTON_IN_2)<<13;
    res |= gpio_get_level(HAL_BUTTON_IN_3)<<14;
    res |= gpio_get_level(HAL_BUTTON_IN_4)<<15;
    gpio_set_level(HAL_BUTTON_OUT_4, 0);

    return res;
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
