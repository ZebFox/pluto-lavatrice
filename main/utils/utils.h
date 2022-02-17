#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include <sys/time.h>
#include <time.h>


#define get_millis() (xTaskGetTickCount() * portTICK_PERIOD_MS)

void utils_get_sys_time(struct tm *systm);

#endif
