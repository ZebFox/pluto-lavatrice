#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "peripherals/hardwareprofile.h"
#include "peripherals/buzzer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "freertos/timers.h"

static void          buzzer_periodic(TimerHandle_t timer);
static unsigned long time_on  = 0;
static unsigned long time_off = 0;
static size_t        repeat   = 0;
static int           is_set   = 0;
static TimerHandle_t timer;
static const char *TAG = "Buzzer";

#define GPIO_OUTPUT_PIN_SEL (1ULL << HAL_BUZZER)

void buzzer_init(void) {
    // zero-initialize the config structure.
    gpio_config_t io_conf = {};
    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
    // bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    // disable pull-down mode
    io_conf.pull_down_en = 0;
    // disable pull-up mode
    io_conf.pull_up_en = 0;
    // configure GPIO with the given settings
    gpio_config(&io_conf);

    static StaticTimer_t timer_buffer;
    timer = xTimerCreateStatic(TAG, 1, pdTRUE, NULL, buzzer_periodic, &timer_buffer);
}

void buzzer_bip(size_t r, unsigned long t_on, unsigned long t_off) {
    repeat   = r;
    time_on  = t_on;
    time_off = t_off;
    is_set   = 1;
    gpio_set_level(HAL_BUZZER, 1);
    xTimerStop(timer, portMAX_DELAY);
    xTimerChangePeriod(timer, pdMS_TO_TICKS(t_on), portMAX_DELAY);
    xTimerStart(timer, portMAX_DELAY);
}

void buzzer_check(void) {
    if (is_set && repeat > 0) {
        if (gpio_get_level(HAL_BUZZER)) {
            gpio_set_level(HAL_BUZZER, 0);
            xTimerChangePeriod(timer, pdMS_TO_TICKS(time_off), portMAX_DELAY);
            xTimerReset(timer, portMAX_DELAY);
            repeat--;
        } else if (!gpio_get_level(HAL_BUZZER)) {
            gpio_set_level(HAL_BUZZER, 1);
            xTimerChangePeriod(timer, pdMS_TO_TICKS(time_on), portMAX_DELAY);
            xTimerReset(timer, portMAX_DELAY);
        }
    }

    if (is_set && repeat == 0) {
        is_set = 0;
        gpio_set_level(HAL_BUZZER, 0);
        xTimerStop(timer, portMAX_DELAY);
    }
}

static void buzzer_periodic(TimerHandle_t timer) {
    (void)timer;
    buzzer_check();
}
