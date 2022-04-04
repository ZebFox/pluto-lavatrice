#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "model/model.h"
#include <sys/time.h>
#include <time.h>


#define get_millis() (xTaskGetTickCount() * portTICK_PERIOD_MS)

void        utils_get_sys_time(struct tm *systm);
void        utils_set_system_time(struct tm systm);
void        utils_set_rtc_time(struct tm systm);
void        utils_dump_state(stato_macchina_t *stato);
const char *utils_get_build_date(void);
size_t      utils_circular_decrease(size_t num, size_t max);

#endif
