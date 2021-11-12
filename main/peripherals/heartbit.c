#include <driver/gpio.h>
#include "peripherals/hardwareprofile.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"


static void timer_heartbit(TimerHandle_t timer);


void heartbit_init(void) {
    gpio_set_direction(HAL_LED_RUN, GPIO_MODE_OUTPUT);

    TimerHandle_t timer = xTimerCreate("heartbit", pdMS_TO_TICKS(100), 1, NULL, timer_heartbit);
    xTimerStart(timer, portMAX_DELAY);
}


static void timer_heartbit(TimerHandle_t timer) {
    static int blink = 0;
    gpio_set_level(HAL_LED_RUN, blink);
    blink = !blink;
}