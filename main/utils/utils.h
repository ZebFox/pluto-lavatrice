#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "model/model.h"
#include <sys/time.h>
#include <time.h>


#define get_millis()               (xTaskGetTickCount() * portTICK_PERIOD_MS)
#define WINDOWED_LIST(window_size) ((windowed_list_t){0, 0, window_size})


typedef struct {
    size_t start;
    size_t index;
    size_t window;
} windowed_list_t;


void        utils_get_sys_time(struct tm *systm);
void        utils_set_system_time(struct tm systm);
void        utils_set_rtc_time(struct tm systm);
void        utils_dump_state(stato_macchina_t *stato);
const char *utils_get_build_date(void);
size_t      utils_circular_decrease(size_t num, size_t max);
void        utils_free_string_list(char **strings, size_t len);
void        windowed_list_prev(windowed_list_t *list, size_t max);
void        windowed_list_next(windowed_list_t *list, size_t max);

#endif
