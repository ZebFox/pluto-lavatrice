#include "utils.h"
#include <time.h>
#include "esp_log.h"
#include "model/model.h"
#include "peripherals/i2c_devices.h"
#include "i2c_devices/rtc/M41T81/m41t81.h"


static const char *TAG = "Utils";


void utils_get_sys_time(struct tm *systm) {
    time_t t = time(NULL);
    *systm   = *localtime(&t);
    // ESP_LOGI(TAG, "tempo del sistema %i %i %i %i %i", systm->tm_mday,systm->tm_mon, systm->tm_year, systm->tm_hour,
    // systm->tm_min);
}


void utils_set_system_time(struct tm systm) {
    struct timeval timeval;
    timeval.tv_sec  = mktime(&systm);
    timeval.tv_usec = 0;
    settimeofday(&timeval, NULL);
}


void utils_set_rtc_time(struct tm systm) {
    rtc_time_t rtc_time;
    rtc_time.sec   = systm.tm_sec;
    rtc_time.min   = systm.tm_min;
    rtc_time.hour  = systm.tm_hour;
    rtc_time.day   = systm.tm_mday;
    rtc_time.month = systm.tm_mon;
    rtc_time.year  = systm.tm_year;
    m41t81_set_time(rtc_driver, rtc_time);
}


void utils_dump_state(stato_macchina_t *stato) {
    ESP_LOGI(TAG, "State dump follows:");
    printf("s %i; ss %i; sss %i; a %i\n", stato->stato, stato->sottostato, stato->sottostato_step,
           stato->codice_allarme);
    printf("p %02i; s %02i; cs %02i; t %6i\n", stato->numero_programma, stato->numero_step, stato->codice_step,
           stato->rimanente);
    printf("ped %i\n", stato->descrizione_pedante);
    printf("\n");
}
