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
#include "configuration/configuration.h"


static const char *TAG = "Controller";

static void           pwoff_callback(void *arg);
static rtc_time_t     rtc_time;
static struct timeval tv;
static struct tm      tm;
static int            pending_state_change = 0;


void controller_init(model_t *pmodel) {
    (void)TAG;
    machine_init();
    configuration_init();
    configuration_load_all_data(pmodel);

    view_change_page(pmodel, &page_splash);
    power_off_init();
    power_off_register_callback(pwoff_callback, pmodel);
    // M41T81_init(rtc_driver);
    // rtc_time.sec   = 0;
    // rtc_time.min   = 30;
    // rtc_time.hour  = 10;
    // rtc_time.day   = 17;
    // rtc_time.wday  = 4;
    // rtc_time.month = 1;
    // rtc_time.year  = 2022 - 1900;
    //  m41t81_set_time(rtc_driver, &rtc_time);
    ESP_LOGI(TAG, "impostato rtc %i %i %i", rtc_time.day, rtc_time.month, rtc_time.year);
    tm.tm_sec  = rtc_time.sec;
    tm.tm_min  = rtc_time.min;
    tm.tm_hour = rtc_time.hour;
    tm.tm_mday = rtc_time.day;
    tm.tm_mon  = rtc_time.month;
    tm.tm_year = rtc_time.year;
    tv.tv_usec = 0;
    tv.tv_sec  = mktime(&tm);
    settimeofday(&tv, NULL);
}


void controller_process_msg(view_controller_command_t *msg, model_t *pmodel) {
    switch (msg->code) {
        case VIEW_CONTROLLER_COMMAND_CODE_START_PROGRAM:
            if (pending_state_change) {
                break;
            }
            if (model_get_current_step_number(pmodel) > 0) {
                // Se eri fermo assicurati di ripartire da 0, in pausa potrei aver
                // selezionato un nuovo step
                if (model_macchina_in_stop(pmodel)) {
                    model_azzera_lavaggio(pmodel);
                }

                pending_state_change = 1;
                machine_start(pmodel->run.num_prog_corrente);

                // Se nel frattempo ho scelto un altro step devo mandare quello nuovo
                if (model_macchina_in_pausa(pmodel) &&
                    (int)pmodel->run.macchina.numero_step != pmodel->run.num_step_corrente) {
                    machine_esegui_step(model_get_current_step(pmodel), model_get_current_step_number(pmodel));
                    model_avanza_step(pmodel);
                } else {     // Altrimenti l'ho cominciato e devo mandare i parametri macchina
                    machine_invia_parmac(&pmodel->prog.parmac);
                }
            }
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_PAUSE:
            if (!pending_state_change) {
                pending_state_change = 1;
                machine_pause();
            }
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_STOP:
            if (!pending_state_change) {
                pending_state_change = 1;
                machine_stop();
            }
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_CREATE_PROGRAM:
            configuration_create_empty_program(pmodel);
            pmodel->prog.num_programmi = configuration_load_programs_preview(pmodel->prog.preview_programmi,
                                                                             MAX_PROGRAMMI, model_get_language(pmodel));
            configuration_clear_orphan_programs(pmodel->prog.preview_programmi, pmodel->prog.num_programmi);
            view_event((view_event_t){.code = VIEW_EVENT_CODE_DATA_REFRESH});
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_LOAD_PROGRAM:
            if (configuration_load_program(pmodel, msg->num)) {
                break;
            }
            view_event((view_event_t){.code = VIEW_EVENT_CODE_PROGRAM_LOADED, .num = msg->num});
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_UPDATE_PROGRAM:
            configuration_update_program(model_get_program(pmodel));
            model_sync_program_preview(pmodel);
            view_event((view_event_t){.code = VIEW_EVENT_CODE_PROGRAM_SAVED});
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_TEST:
            machine_test(msg->test);
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_TEST_REFRESH:
            machine_richiedi_dati_test();
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
    static int           initial_level_check = 0;
    static unsigned long stato_ts            = 0;
    machine_response_t   risposta;

    if (is_expired(stato_ts, get_millis(), 250)) {
        machine_richiedi_stato();
        stato_ts = get_millis();
    }

    if (machine_ricevi_risposta(&risposta)) {
        switch (risposta.code) {
            case MACHINE_RESPONSE_CODE_ERRORE_COMUNICAZIONE:
                // TODO: segnala errore di comunicazione
                break;

            case MACHINE_RESPONSE_CODE_PRESENTAZIONI: {
                //  Invia i parametri macchina
                machine_invia_parmac(&pmodel->prog.parmac);

                if (risposta.presentazioni.n_all == 0 || risposta.presentazioni.n_all == 2) {
                    // allarme poweroff; se l'autoavvio e' configurato devo ripartire
                    if (risposta.presentazioni.n_all == 0x02 && pmodel->prog.parmac.autoavvio) {
                        machine_azzera_allarmi();
                    }

                    if (risposta.presentazioni.stato != STATO_MACCHINA_STOP) {
                        uint8_t lavaggio = risposta.presentazioni.nro_programma;
                        int     step     = risposta.presentazioni.nro_step;

                        if (lavaggio < model_get_num_programs(pmodel)) {
                            configuration_load_program(pmodel, lavaggio);
                            if (model_select_program_step(pmodel, lavaggio, step) >= 0) {
                                ESP_LOGI(TAG, "Macchina spenta in azione: lavaggio %i e step %i", lavaggio + 1,
                                         step + 1);
                                pending_state_change = 1;
                                machine_start(lavaggio);
                            }
                        }
                    }
                }
                break;
            }

            case MACHINE_RESPONSE_CODE_TEST:
                pmodel->test = risposta.test;
                view_event((view_event_t){.code = VIEW_EVENT_CODE_MODEL_UPDATE});
                break;

            case MACHINE_RESPONSE_CODE_STATO:
                machine_read_state(pmodel);

                if (model_get_livello_centimetri(pmodel) > 0 && !initial_level_check) {
                    pmodel->run.f_richiedi_scarico = 1;
                }
                initial_level_check = 1;

                if (pmodel->run.macchina.richiesto_aggiornamento_tempo) {
                    // TODO: controller_send_time(request_pipe);
                    pmodel->run.macchina.richiesto_aggiornamento_tempo = 0;
                }

                view_event((view_event_t){.code = VIEW_EVENT_CODE_MODEL_UPDATE});

                if (pending_state_change) {
                    break;
                }

                // Passaggio allo step successivo se sono sincronizzato con il quadro, in marcia e senza step
                if (model_macchina_in_marcia(pmodel) && model_step_finito(pmodel)) {
                    if (model_lavaggio_finito(pmodel)) {
                        pending_state_change = 1;
                        machine_stop();
                    } else {
                        machine_esegui_step(model_get_current_step(pmodel), model_get_current_step_number(pmodel));
                    }
                }

                if (model_macchina_in_scarico_forzato(pmodel)) {
                    pmodel->run.f_richiedi_scarico = 0;
                }
                break;
        }
    }
}

static void pwoff_callback(void *arg) {
    ESP_LOGI(TAG, "chiamata cb PWoff");
}