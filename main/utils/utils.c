#include "utils.h"
#include <stdio.h>
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
    printf("p %2i; s %2i; cs %2i; t %6i\n", stato->numero_programma, stato->numero_step, stato->codice_step,
           stato->rimanente);
    printf("ped %i\n", stato->descrizione_pedante);
    printf("oblo %i %i %i %i\n", stato->oblo_aperto_chiuso, stato->chiavistello_chiuso, stato->chiavistello_aperto,
           stato->allarme_oblo_aperto);
    printf("dati: t %3i (%3i); l %3i %3i (%3i)", stato->temperatura, stato->temperatura_impostata, stato->livello,
           stato->livello_litri, stato->livello_impostato);
    printf("\n");
}


const char *utils_get_build_date(void) {
    static char date[32]     = {0};
    char        month_str[4] = {0};
    int         day          = 0;
    int         month        = 0;
    int         year         = 0;
    sscanf(__DATE__, "%s %i %i", month_str, &day, &year);

    char *months[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    };

    for (size_t i = 0; i < sizeof(months) / sizeof(months[0]); i++) {
        if (strcmp(months[i], month_str) == 0) {
            month = i + 1;
            break;
        }
    }

    snprintf(date, sizeof(date), "%02i/%02i/%i", day, month, year);
    return (const char *)date;
}


size_t utils_circular_decrease(size_t num, size_t max) {
    if (num > 0) {
        return num - 1;
    } else {
        return max - 1;
    }
}