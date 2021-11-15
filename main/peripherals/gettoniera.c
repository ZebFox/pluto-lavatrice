
#include "gettoniera.h"
#include "freertos/portmacro.h"
#include "peripherals/hardwareprofile.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "freertos/timers.h"
#include "gel/debounce/pulsecounter.h"

#define GPIO_INPUT_PIN_SEL                                                                                             \
    ((1ULL << HAL_GETT_1) | (1ULL << HAL_GETT_2) | (1ULL << HAL_GETT_3) | (1ULL << HAL_GETT_4) | (1ULL << HAL_GETT_5))
#define GPIO_OUTPUT_PIN_SEL (1ULL << HAL_GETT_INHIBIT)

static SemaphoreHandle_t sem;
static void              gettoniera_periodic_read(TimerHandle_t timer);
static pulse_filter_t    filter;

void gettoniera_init(void) {
    // output abilitazione
    // zero-initialize the config structure.
    gpio_config_t io_conf_output = {};
    // disable interrupt
    io_conf_output.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf_output.mode = GPIO_MODE_INPUT_OUTPUT;
    // bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf_output.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    // disable pull-down mode
    io_conf_output.pull_down_en = 0;
    // disable pull-up mode
    io_conf_output.pull_up_en = 0;
    // configure GPIO with the given settings
    gpio_config(&io_conf_output);

    gpio_set_level(HAL_GETT_INHIBIT, 1);

    // input
    // zero-initialize the config structure.
    gpio_config_t io_conf_input = {};
    // disable interrupt
    io_conf_input.intr_type = GPIO_INTR_DISABLE;
    // bit mask of the pins, use GPIO4/5 here
    io_conf_input.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    // set as input mode
    io_conf_input.mode = GPIO_MODE_INPUT;
    // disable pull-down mode
    io_conf_input.pull_down_en = 0;
    // disable pull-up mode
    io_conf_input.pull_up_en = 0;
    gpio_config(&io_conf_input);

    sem                 = xSemaphoreCreateMutex();
    TimerHandle_t timer = xTimerCreate("timerInput", pdMS_TO_TICKS(5), pdTRUE, NULL, gettoniera_periodic_read);
    xTimerStart(timer, portMAX_DELAY);
}

void gettoniera_reset_count(gett_t i) {
    xSemaphoreTake(sem, portMAX_DELAY);
    pulse_clear(&filter, i);
    xSemaphoreGive(sem);
}

void gettoniera_reset_all_count(void) {
    gettoniera_reset_count(GETT1);
    gettoniera_reset_count(GETT2);
    gettoniera_reset_count(GETT3);
    gettoniera_reset_count(GETT4);
    gettoniera_reset_count(GETT5);
}

int gettoniera_take_insert(void) {
    unsigned int input = 0;
    input |= gpio_get_level(HAL_GETT_1);
    input |= gpio_get_level(HAL_GETT_2) << 1;
    input |= gpio_get_level(HAL_GETT_3) << 2;
    input |= gpio_get_level(HAL_GETT_4) << 3;
    input |= gpio_get_level(HAL_GETT_5) << 4;
    return pulse_filter(&filter, input, 4);
}

unsigned int gettoniera_get_count(gett_t i) {
    xSemaphoreTake(sem, portMAX_DELAY);
    unsigned int res =  pulse_count(&filter, i);
    xSemaphoreGive(sem);
    return res;
}


static void gettoniera_periodic_read(TimerHandle_t timer) {
    (void)timer;
    xSemaphoreTake(sem, portMAX_DELAY);
    gettoniera_take_insert();
    xSemaphoreGive(sem);
}