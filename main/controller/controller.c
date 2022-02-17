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


static const char *TAG = "Controller";


void controller_init(model_t *pmodel) {
    (void)TAG;
    machine_init();
    parmac_init(pmodel, 0);
    view_change_page(pmodel, &page_splash);
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