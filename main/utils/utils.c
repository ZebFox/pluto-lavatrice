#include "utils.h"
#include <time.h>
#include "esp_log.h"

static const char *TAG = "Utils";


void utils_get_sys_time(struct tm *systm) {
    time_t t = time(NULL);
    *systm=*localtime(&t);
    //ESP_LOGI(TAG, "tempo del sistema %i %i %i %i %i", systm->tm_mday,systm->tm_mon, systm->tm_year, systm->tm_hour, systm->tm_min);
}
