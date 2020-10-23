#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#define get_millis() ((uint32_t)xTaskGetTickCount() * portTICK_PERIOD_MS)

#endif