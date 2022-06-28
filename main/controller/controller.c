#include "esp_system.h"
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
#include "peripherals/display/NT7534.h"
#include "peripherals/buzzer.h"
#include "configuration/configuration.h"
#include "configuration/msc.h"


static const char *TAG = "Controller";

static void pwoff_callback(void *arg);
static int  pending_state_change = 0;


void controller_init(model_t *pmodel) {
    (void)TAG;
    machine_init();
    configuration_init();
    configuration_load_all_data(pmodel);
    nt7534_set_contrast(pmodel->prog.contrast);
    msc_init();

    view_change_page(pmodel, &page_splash);
    view_event((view_event_t){.code = VIEW_EVENT_CODE_OPEN});

    power_off_init();
    power_off_register_callback(pwoff_callback, pmodel);
    m41t81_init(rtc_driver);

    rtc_time_t rtc_time = {0};
    m41t81_get_time(rtc_driver, &rtc_time);

    struct tm tm = {0};
    tm.tm_sec    = rtc_time.sec;
    tm.tm_min    = rtc_time.min;
    tm.tm_hour   = rtc_time.hour;
    tm.tm_mday   = rtc_time.day;
    tm.tm_mon    = rtc_time.month;
    tm.tm_year   = rtc_time.year;
    utils_set_system_time(tm);

    machine_invia_presentazioni();
}


void controller_process_msg(view_controller_command_t *msg, model_t *pmodel) {
    switch (msg->code) {
        case VIEW_CONTROLLER_COMMAND_CODE_REMOVE_PROGRAM:
            configuration_remove_program(pmodel->prog.preview_programmi, pmodel->prog.num_programmi, msg->num);
            pmodel->prog.num_programmi--;
            view_event((view_event_t){.code = VIEW_EVENT_CODE_PROGRAM_CHANGE_DONE});
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_AGGIORNA_ORA_DATA:
            machine_send_time();
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_SAVE_CONTRAST:
            configuration_save_contrast(pmodel);
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_UPDATE_CONTRAST:
            nt7534_set_contrast(pmodel->prog.contrast);
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_RESET_RAM:
            configuration_delete_all();
            configuration_load_all_data(pmodel);
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_RESET:
            esp_restart();
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_LOAD_ARCHIVE:
            msc_extract_archive(msg->archive_name);
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_SEND_PARMAC:
            machine_invia_parmac(&pmodel->prog.parmac);
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_SEND_PARMAC_AND_TURNOFF:
            machine_invia_parmac(&pmodel->prog.parmac);
            machine_imposta_uscita_singola(0, 0);
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_TEST_LED_CONTROl:
            machine_imposta_led(msg->value);
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_TEST_CLEAR_CREDIT:
            machine_azzera_credito();
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_CLEAR_LITERS:
            machine_azzera_litri();
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_SAVE_PARMAC:
            configuration_save_parmac(&pmodel->prog.parmac);
            //  Invia i parametri macchina
            machine_invia_parmac(&pmodel->prog.parmac);
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_FORCE_DRAIN:
            machine_forza_scarico();
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_AZZERA_LITRI:
            ESP_LOGI(TAG, "Zeroing liters");
            machine_azzera_litri();
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_OFFSET_PRESSIONE:
            ESP_LOGI(TAG, "Resetting pressure offset");
            machine_offset_pressione();
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_COLPO_SAPONE:
            machine_activate_detergent(msg->output);
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_CONTROLLO_SAPONE:
            machine_control_detergent(msg->output, msg->value);
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_START_PROGRAM:
            if (pending_state_change) {
                break;
            }
            if (model_get_program(pmodel)->num_steps > 0) {
                // Se eri fermo assicurati di ripartire da 0, in pausa potrei aver
                // selezionato un nuovo step
                if (model_macchina_in_stop(pmodel)) {
                    model_azzera_lavaggio(pmodel);
                }

                pending_state_change = 1;
                machine_azzera_allarmi();
                machine_start(model_get_program_num(pmodel));

                // Se nel frattempo ho scelto un altro step devo mandare quello nuovo
                if (model_macchina_in_pausa(pmodel) &&
                    (int)pmodel->run.macchina.numero_step != pmodel->run.num_step_corrente) {
                    machine_esegui_step(model_get_current_step(pmodel), model_get_current_step_number(pmodel));
                } else {     // Altrimenti l'ho cominciato e devo mandare i parametri macchina
                    machine_invia_parmac(&pmodel->prog.parmac);
                }
            }
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_PAUSE:
            ESP_LOGI(TAG, "Pausing");
            pending_state_change = 1;
            machine_pause();
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_STOP:
            ESP_LOGI(TAG, "Stopping");
            pending_state_change = 1;
            machine_stop();
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_APRI_OBLO:
            machine_apri_oblo(0);
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_CHIUDI_OBLO:
            machine_chiudi_oblo();
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_FORZA_APERTURA_OBLO:
            machine_apri_oblo(1);
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_RELOAD_PREVIEWS:
            pmodel->prog.num_programmi = configuration_load_programs_preview(
                pmodel, pmodel->prog.preview_programmi, MAX_PROGRAMMI, model_get_temporary_language(pmodel));
            view_event((view_event_t){.code = VIEW_EVENT_CODE_PREVIEWS_LOADED});
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_CLONE_PROGRAM:
            if (configuration_load_program(pmodel, msg->source)) {
                break;
            }

            configuration_clone_program(pmodel, msg->destination);
            pmodel->prog.num_programmi = configuration_load_programs_preview(pmodel, pmodel->prog.preview_programmi,
                                                                             MAX_PROGRAMMI, model_get_language(pmodel));
            view_event((view_event_t){.code = VIEW_EVENT_CODE_PROGRAM_CHANGE_DONE});
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_CREATE_PROGRAM:
            configuration_create_empty_program(pmodel);
            pmodel->prog.num_programmi = configuration_load_programs_preview(pmodel, pmodel->prog.preview_programmi,
                                                                             MAX_PROGRAMMI, model_get_language(pmodel));
            configuration_clear_orphan_programs(pmodel->prog.preview_programmi, pmodel->prog.num_programmi);
            view_event((view_event_t){.code = VIEW_EVENT_CODE_DATA_REFRESH});
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_RETRY_COMMUNICATION:
            ESP_LOGI(TAG, "Requesting com retry");
            machine_riavvia_comunicazione();
            machine_invia_parmac(&pmodel->prog.parmac);
            pmodel->system.errore_comunicazione = 0;
            view_event((view_event_t){.code = VIEW_EVENT_CODE_MODEL_UPDATE});
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_MODIFICA_PARAMETRI_IN_LAVAGGIO:
            machine_modify_cycle_parameters(model_get_current_step_number(pmodel), msg->duration, msg->speed,
                                            msg->temperature, msg->level);
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_CHANGE_AB_COMMUNICATION:
            machine_abilita_comunicazione(msg->value);
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

        case VIEW_CONTROLLER_COMMAND_CODE_AZZERA_ALLARMI:
            machine_azzera_allarmi();
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_TEST_REFRESH:
            machine_richiedi_dati_test();
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_UPDATE_STATISTICS:
            machine_read_stats(pmodel);
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_DIGOUT_TURNOFF:
            machine_imposta_uscita_singola(0, 0);
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_TEST_DIGOUT:
            machine_imposta_uscita_singola(msg->output, msg->value);
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_TEST_DIGOUT_MULTI:
            machine_imposta_uscita_multipla(msg->output, msg->value);
            break;

        case VIEW_CONTROLLER_COMMAND_CODE_TEST_DAC_CONTROL:
            machine_imposta_dac(msg->value);
            break;


        case VIEW_CONTROLLER_COMMAND_CODE_SEND_DEBUG_CODE:
            machine_send_debug_code(msg->value);
            break;

        default:
            break;
    }
}


void controller_manage(model_t *pmodel) {
    static int           initial_level_check = 0;
    static unsigned long stato_ts            = 0;
    machine_response_t   machine_response;
    msc_response_t       msc_response;

    if (is_expired(stato_ts, get_millis(), 500)) {
        model_set_drive_mounted(pmodel, msc_is_device_mounted());
        msc_read_archives(pmodel);
        machine_richiedi_stato();
        view_event((view_event_t){.code = VIEW_EVENT_CODE_DEVICE_UPDATE});
        stato_ts = get_millis();
    }

    if (msc_get_response(&msc_response)) {
        switch (msc_response.code) {
            case MSC_RESPONSE_CODE_ARCHIVE_EXTRACTION_COMPLETE:
                configuration_load_all_data(pmodel);
                model_set_drive_mounted(pmodel, msc_is_device_mounted());
                view_event((view_event_t){.code = VIEW_EVENT_CODE_CONFIGURATION_LOADED, .error = msc_response.error});
                break;
        }
    }

    if (machine_ricevi_risposta(&machine_response)) {
        switch (machine_response.code) {
            case MACHINE_RESPONSE_CODE_DRAIN_REQUIRED:
                ESP_LOGI(TAG, "E' necessario uno scarico");
                if (model_lavaggio_finito(pmodel)) {     // Se sono alla fine devo segnalarlo all'utente
                    pending_state_change = 1;
                    machine_pause();
                    pmodel->run.f_richiedi_scarico = 1;
                } else {     // Altrimenti se l'utente ha richiesto lo stop devo passare automaticamente
                             // allo scarico
                    machine_forza_scarico();
                }
                break;

            case MACHINE_RESPONSE_CODE_REFUSED:
                pending_state_change = 0;
                buzzer_bip(2, 500, 500);
                break;

            case MACHINE_RESPONSE_CODE_ERRORE_COMUNICAZIONE:
                pmodel->system.errore_comunicazione = 1;
                view_event((view_event_t){.code = VIEW_EVENT_CODE_MODEL_UPDATE});
                break;

            case MACHINE_RESPONSE_CODE_PRESENTAZIONI: {
                //  Invia i parametri macchina
                machine_invia_parmac(&pmodel->prog.parmac);

                // allarme poweroff; se l'autoavvio e' configurato devo ripartire
                if (machine_response.presentazioni.n_all == 0x02 && pmodel->prog.parmac.autoavvio) {
                    machine_azzera_allarmi();
                }

                uint8_t allarme = machine_response.presentazioni.n_all;
                uint8_t stato   = machine_response.presentazioni.stato;

                ESP_LOGI(TAG, "Machine initial state %i (alarm %i)", machine_response.presentazioni.stato, allarme);
                if (allarme == 0 || allarme == 2) {
                    // allarme poweroff; se l'autoavvio e' configurato devo ripartire
                    if (allarme == 0x02 && pmodel->prog.parmac.autoavvio) {
                        machine_azzera_allarmi();
                    }

                    if (stato != STATO_MACCHINA_STOP) {
                        uint8_t lavaggio = machine_response.presentazioni.nro_programma;
                        int     step     = machine_response.presentazioni.nro_step;
                        ESP_LOGI(TAG, "Machine executing program %i, step %i", lavaggio, step);

                        if (lavaggio < model_get_num_programs(pmodel)) {
                            configuration_load_program(pmodel, lavaggio);
                            ESP_LOGI(TAG, "Macchina spenta in azione: lavaggio %i e step %i", lavaggio + 1, step + 1);
                            if (model_select_program_step(pmodel, lavaggio, step) >= 0) {
                                ESP_LOGI(TAG, "Caricato lavaggio %i e step %i", model_get_program_num(pmodel) + 1,
                                         pmodel->run.num_step_corrente + 1);
                                if (pmodel->prog.parmac.autoavvio) {
                                    pending_state_change = 1;
                                    machine_start(lavaggio);
                                }
                            }
                        } else {
                            ESP_LOGI(TAG, "Inconsistent state");
                            pending_state_change = 1;
                            machine_stop();
                        }
                    }
                }
                break;
            }

            case MACHINE_RESPONSE_CODE_TEST:
                pmodel->test = machine_response.test;
                view_event((view_event_t){.code = VIEW_EVENT_CODE_MODEL_UPDATE});
                break;

            case MACHINE_RESPONSE_CODE_STATO: {
                if (machine_read_state(pmodel)) {
                    pending_state_change = 0;
                }

                if (!model_macchina_in_stop(pmodel) &&
                    (model_get_program_num(pmodel) != pmodel->run.macchina.numero_programma ||
                     pmodel->run.maybe_programma == 0)) {
                    ESP_LOGI(TAG, "Loading current program");
                    configuration_load_program(pmodel, pmodel->run.macchina.numero_programma);
                }

                // utils_dump_state(&pmodel->run.macchina);

                if (model_get_livello_centimetri(pmodel) > 0 && !initial_level_check) {
                    pmodel->run.f_richiedi_scarico = 1;
                }
                initial_level_check = 1;

                view_event((view_event_t){.code = VIEW_EVENT_CODE_MODEL_UPDATE});

                if (pending_state_change) {
                    ESP_LOGI(TAG, "Pending state change, ignoring");
                    break;
                }

                // Passaggio allo step successivo se sono sincronizzato con il quadro, in marcia e senza step
                if (model_macchina_in_marcia(pmodel) && model_step_finito(pmodel)) {
                    if (model_lavaggio_finito(pmodel)) {
                        pending_state_change = 1;
                        ESP_LOGI(TAG, "Program done");
                        machine_stop();
                    } else {
                        model_avanza_step(pmodel);
                        ESP_LOGI(TAG, "Moving to step %i", model_get_current_step_number(pmodel));
                        machine_esegui_step(model_get_current_step(pmodel), model_get_current_step_number(pmodel));
                    }
                }

                if (model_macchina_in_scarico_forzato(pmodel)) {
                    pmodel->run.f_richiedi_scarico = 0;
                }

                if (model_requested_time(pmodel)) {
                    machine_send_time();
                    pmodel->run.macchina.richiesto_aggiornamento_tempo = 0;
                }
                break;
            }

            case MACHINE_RESPONSE_CODE_STATS:
                pmodel->stats = *machine_response.stats;
                free(machine_response.stats);
                view_event((view_event_t){.code = VIEW_EVENT_CODE_MODEL_UPDATE});
                break;
        }
    }
}

static void pwoff_callback(void *arg) {
    ESP_LOGI(TAG, "chiamata cb PWoff");
}