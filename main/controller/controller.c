#include "controller.h"
#include "model/model.h"
#include "model/parmac.h"
#include "view/view.h"
#include "com/packet.h"
#include "com/machine.h"
#include "peripherals/machine_serial.h"
#include "esp_log.h"
#include "gel/timer/timecheck.h"
#include "utils/utils.h"
#include "peripherals/pwoff.h"
#include "i2c_devices/rtc/M41T81/m41t81.h"
#include "peripherals/i2c_devices.h"


static const char *TAG = "Controller";

static void pwoff_callback(void *arg);
static rtc_time_t rtc_time;
static struct timeval tv;
static struct tm tm;


void controller_init(model_t *pmodel) {
    (void)TAG;
    machine_init();
    parmac_init(pmodel, 0);
    view_change_page(pmodel, &page_splash);
    power_off_init();
    power_off_register_callback(pwoff_callback, pmodel);
    M41T81_init(rtc_driver);
    rtc_time.sec = 0;
    rtc_time.min = 30;
    rtc_time.hour = 10;
    rtc_time.day = 17;
    rtc_time.wday = 4;
    rtc_time.month = 1;
    rtc_time.year = 2022-1900;
    //m41t81_set_time(rtc_driver, &rtc_time);
    ESP_LOGI(TAG, "impostato rtc %i %i %i", rtc_time.day, rtc_time.month, rtc_time.year);
    tm.tm_sec = rtc_time.sec;
    tm.tm_min = rtc_time.min;
    tm.tm_hour = rtc_time.hour;
    tm.tm_mday = rtc_time.day;
    tm.tm_mon = rtc_time.month;
    tm.tm_year = rtc_time.year;
    tv.tv_usec = 0;
    tv.tv_sec = mktime(&tm);
    settimeofday(&tv, NULL);

}


void controller_process_msg(view_controller_command_t *msg, model_t *pmodel) {
    switch (msg->code) {
        case VIEW_CONTROLLER_COMMAND_CODE_TEST:
            machine_test(msg->test);
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_DIGOUT_TURNOFF:
            machine_imposta_uscita_singola(0, 0);
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_TEST_DIGOUT:
            machine_imposta_uscita_singola(msg->output, msg->value);
            break;

        default:
            break;
    }
}


void controller_manage(model_t *pmodel) {
    static unsigned long timestamp = 0;
    machine_response_t   risposta;

    if (is_expired(timestamp, get_millis(), 250)) {
        machine_richiedi_stato();
        timestamp = get_millis();
    }

    if (machine_ricevi_risposta(&risposta)) {
        switch (risposta.code) {
            case MACHINE_RESPONSE_CODE_ERRORE_COMUNICAZIONE:
                // TODO: segnala errore di comunicazione
                break;

            case MACHINE_RESPONSE_CODE_PRESENTAZIONI:
                // TODO: ricevi le presentazioni
                break;

            case MACHINE_RESPONSE_CODE_STATO:
                // TODO: ricevi lo stato
                break;
        }
    }
}

static void pwoff_callback(void *arg) {
    ESP_LOGI(TAG, "chiamata cb PWoff");
}