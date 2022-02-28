#include <string.h>
#include "packet.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "config/app_config.h"
#include "peripherals/machine_serial.h"
#include "esp_log.h"
#include "packet.h"
#include "machine.h"
#include "gel/serializer/serializer.h"
#include "model/model.h"


#define TENTATIVI_TOTALI 5
#define RITARDO_MS       50


typedef enum {
    MACHINE_REQUEST_CODE_PRESENTAZIONI,
    MACHINE_REQUEST_CODE_TEST,
    MACHINE_REQUEST_CODE_TEST_DATA,
    MACHINE_REQUEST_CODE_STATO,
    MACHINE_REQUEST_CODE_STATS,
    MACHINE_REQUEST_CODE_RIAVVIA_COMUNICAZIONE,
    MACHINE_REQUEST_CODE_IMPOSTA_USCITA,
    MACHINE_REQUEST_CODE_SCRIVI_PARAMETRI_MACCHINA,
    MACHINE_REQUEST_CODE_AZZERA_ALLARMI,
    MACHINE_REQUEST_CODE_START,
    MACHINE_REQUEST_CODE_PAUSA,
    MACHINE_REQUEST_CODE_STOP,
    MACHINE_REQUEST_CODE_ESEGUI_STEP,
    MACHINE_REQUEST_CODE_FORZA_SCARICO,
} machine_request_code_t;


typedef struct {
    machine_request_code_t code;
    union {
        struct {
            int    singola;
            size_t uscita;
            int    valore;
        };
        int       test;
        parmac_t *parmac;
        uint8_t   num;
        struct {
            parametri_step_t pars;
            uint8_t          num;
        } step;
    };
} machine_request_t;


static void communication_task(void *args);
static int  invia_pacchetto_semplice(int comando, packet_t *risposta, int lunghezza_dati_prevista);
static int  invia_pacchetto(int comando, uint8_t *dati, uint16_t lunghezza_dati, packet_t *risposta,
                            int lunghezza_dati_prevista);
static int  task_gestisci_richiesta(machine_request_t request);
static void free_packet(packet_t *packet);


static const char       *TAG       = "Machine";
static QueueHandle_t     requestq  = NULL;
static QueueHandle_t     responseq = NULL;
static SemaphoreHandle_t sem       = NULL;
static stato_macchina_t  stato     = {0};
static int               communication_ab;

void machine_init(void) {
    static StaticQueue_t static_requestq_queue;
    static uint8_t       requestq_queue_buffer[sizeof(machine_request_t) * 8] = {0};
    requestq = xQueueCreateStatic(sizeof(requestq_queue_buffer) / sizeof(machine_request_t), sizeof(machine_request_t),
                                  requestq_queue_buffer, &static_requestq_queue);

    communication_ab = 1;
    static StaticQueue_t static_responseq_queue;
    static uint8_t       responseq_queue_buffer[sizeof(machine_response_t) * 4] = {0};
    responseq = xQueueCreateStatic(sizeof(responseq_queue_buffer) / sizeof(machine_response_t),
                                   sizeof(machine_response_t), responseq_queue_buffer, &static_responseq_queue);

    static StaticTask_t static_task;
    static StackType_t  task_stack[BASE_TASK_STACK_SIZE * 8] = {0};
    xTaskCreateStatic(communication_task, TAG, sizeof(task_stack), NULL, 1, task_stack, &static_task);

    static StaticSemaphore_t static_semaphore;
    sem = xSemaphoreCreateMutexStatic(&static_semaphore);
}


int machine_read_state(model_t *pmodel) {
    xSemaphoreTake(sem, portMAX_DELAY);
    int res = stato.stato != pmodel->run.macchina.stato;
    memcpy(&pmodel->run.macchina, &stato, sizeof(stato_macchina_t));
    xSemaphoreGive(sem);
    return res;
}


void machine_read_stats(model_t *pmodel) {
    machine_request_t richiesta = {.code = MACHINE_REQUEST_CODE_STATS};
    xQueueSend(requestq, &richiesta, portMAX_DELAY);
}


void machine_invia_parmac(parmac_t *parmac) {
    machine_request_t richiesta = {.code = MACHINE_REQUEST_CODE_SCRIVI_PARAMETRI_MACCHINA};
    richiesta.parmac            = malloc(sizeof(parmac_t));
    memcpy(richiesta.parmac, parmac, sizeof(parmac_t));
    xQueueSend(requestq, &richiesta, portMAX_DELAY);
}


void machine_esegui_step(parametri_step_t *step, uint8_t num) {
    machine_request_t richiesta = {.code = MACHINE_REQUEST_CODE_ESEGUI_STEP, .step = {.pars = *step, .num = num}};
    xQueueSend(requestq, &richiesta, portMAX_DELAY);
}


void machine_forza_scarico(void) {
    machine_request_t richiesta = {.code = MACHINE_REQUEST_CODE_FORZA_SCARICO};
    xQueueSend(requestq, &richiesta, portMAX_DELAY);
}


void machine_azzera_allarmi(void) {
    machine_request_t richiesta = {.code = MACHINE_REQUEST_CODE_AZZERA_ALLARMI};
    xQueueSend(requestq, &richiesta, portMAX_DELAY);
}


void machine_start(uint8_t num_program) {
    machine_request_t richiesta = {.code = MACHINE_REQUEST_CODE_START, .num = num_program};
    xQueueSend(requestq, &richiesta, portMAX_DELAY);
}


void machine_stop(void) {
    machine_request_t richiesta = {.code = MACHINE_REQUEST_CODE_STOP};
    xQueueSend(requestq, &richiesta, portMAX_DELAY);
}


void machine_pause(void) {
    machine_request_t richiesta = {.code = MACHINE_REQUEST_CODE_PAUSA};
    xQueueSend(requestq, &richiesta, portMAX_DELAY);
}


int machine_ricevi_risposta(machine_response_t *risposta) {
    return xQueueReceive(responseq, risposta, 0);
}


void machine_riavvia_comunicazione(void) {
    machine_request_t richiesta = {.code = MACHINE_REQUEST_CODE_RIAVVIA_COMUNICAZIONE};
    xQueueSend(requestq, &richiesta, portMAX_DELAY);
}


void machine_richiedi_stato(void) {
    machine_request_t richiesta = {.code = MACHINE_REQUEST_CODE_STATO};
    xQueueSend(requestq, &richiesta, portMAX_DELAY);
}


void machine_test(int test) {
    machine_request_t richiesta = {.code = MACHINE_REQUEST_CODE_TEST, .test = test};
    xQueueSend(requestq, &richiesta, portMAX_DELAY);
}


void machine_richiedi_dati_test(void) {
    machine_request_t richiesta = {.code = MACHINE_REQUEST_CODE_TEST_DATA};
    xQueueSend(requestq, &richiesta, portMAX_DELAY);
}


void machine_invia_presentazioni(void) {
    machine_request_t richiesta = {.code = MACHINE_REQUEST_CODE_PRESENTAZIONI};
    xQueueSend(requestq, &richiesta, portMAX_DELAY);
}


void machine_imposta_uscita_singola(size_t uscita, int valore) {
    machine_request_t richiesta = {
        .code = MACHINE_REQUEST_CODE_IMPOSTA_USCITA, .uscita = uscita, .singola = 1, .valore = valore};
    xQueueSend(requestq, &richiesta, portMAX_DELAY);
}


static void communication_task(void *args) {
    (void)args;
    const machine_response_t risposta_errore          = {.code = MACHINE_RESPONSE_CODE_ERRORE_COMUNICAZIONE};
    machine_request_t        ultima_richiesta_fallita = {0};
    int                      errore_comunicazione     = 0;
    vTaskDelay(pdMS_TO_TICKS(4000));
    ESP_LOGI(TAG, "Inizio task di comunicazione");

    for (;;) {
        machine_request_t request = {0};
        xSemaphoreTake(sem, portMAX_DELAY);
        int ab_local = communication_ab;
        xSemaphoreGive(sem);

        if (xQueueReceive(requestq, &request, portMAX_DELAY)) {
            if (errore_comunicazione && request.code == MACHINE_REQUEST_CODE_RIAVVIA_COMUNICAZIONE) {
                ESP_LOGI(TAG, "Retrying communication, request %i", ultima_richiesta_fallita.code);
                errore_comunicazione = task_gestisci_richiesta(ultima_richiesta_fallita);
                if (errore_comunicazione) {
                    ESP_LOGW(TAG, "Risposta assente o non valida (durante il riavvio)!");
                    xQueueSend(responseq, &risposta_errore, portMAX_DELAY);
                    continue;
                }
            } else if (!errore_comunicazione && ab_local) {
                errore_comunicazione = task_gestisci_richiesta(request);
                if (errore_comunicazione) {
                    ESP_LOGW(TAG, "Risposta assente o non valida!");
                    ultima_richiesta_fallita = request;
                    xQueueSend(responseq, &risposta_errore, portMAX_DELAY);
                }
            }

            vTaskDelay(pdMS_TO_TICKS(RITARDO_MS));
        }
    }

    vTaskDelete(NULL);
}

void machine_abilita_comunicazione(size_t en) {
    xSemaphoreTake(sem, portMAX_DELAY);
    communication_ab = en;
    xSemaphoreGive(sem);
}

static int task_gestisci_richiesta(machine_request_t request) {
    packet_t           risposta_pacchetto = {0};
    int                res                = 0;
    machine_response_t risposta_task      = {0};

    switch (request.code) {
        case MACHINE_REQUEST_CODE_FORZA_SCARICO:
            res = invia_pacchetto_semplice(COMANDO_FORZA_SCARICO, &risposta_pacchetto, -1);
            break;

        case MACHINE_REQUEST_CODE_PRESENTAZIONI: {
            res = invia_pacchetto_semplice(COMANDO_PRESENTAZIONI, &risposta_pacchetto, 38);
            if (res) {
                break;
            }

            risposta_task.code = MACHINE_RESPONSE_CODE_PRESENTAZIONI;
            deserialize_uint8(&risposta_task.presentazioni.n_all, &risposta_pacchetto.data[1]);
            deserialize_uint8(&risposta_task.presentazioni.stato, &risposta_pacchetto.data[3]);
            deserialize_uint8(&risposta_task.presentazioni.nro_programma, &risposta_pacchetto.data[4]);
            deserialize_uint8(&risposta_task.presentazioni.nro_step, &risposta_pacchetto.data[5]);

            // Versione di firmware della macchina
            char string[64];
            strcpy(string, (char *)&risposta_pacchetto.data[6]);
            char *dash = strchr(string, '-');
            if (dash) {
                *dash = '\0';
                dash++;
                memset(risposta_task.presentazioni.machine_fw_date, 0, STRING_NAME_SIZE);
                strncpy(risposta_task.presentazioni.machine_fw_date, dash, STRING_NAME_SIZE);
            }
            memset(risposta_task.presentazioni.machine_fw_version, 0, STRING_NAME_SIZE);
            strncpy(risposta_task.presentazioni.machine_fw_version, string, MAX_NAME_LENGTH);

            xQueueSend(responseq, &risposta_task, portMAX_DELAY);
            break;
        }

        case MACHINE_REQUEST_CODE_SCRIVI_PARAMETRI_MACCHINA: {
            uint8_t buffer[116] = {0};
            int     len         = model_pack_parametri_macchina(buffer, request.parmac);
            assert(len == sizeof(buffer));
            res = invia_pacchetto(COMANDO_SCRIVI_PARMAC, buffer, len, &risposta_pacchetto, 1);

            if (res) {
                break;
            }
            free(request.parmac);
            break;
        }

        case MACHINE_REQUEST_CODE_STATO:
            res = invia_pacchetto_semplice(COMANDO_LEGGI_STATO, &risposta_pacchetto, 159);
            if (res) {
                break;
            }

            risposta_task.code = MACHINE_RESPONSE_CODE_STATO;
            xSemaphoreTake(sem, portMAX_DELAY);
            model_unpack_stato_macchina(&stato, &risposta_pacchetto.data[1]);
            xSemaphoreGive(sem);
            xQueueSend(responseq, &risposta_task, portMAX_DELAY);
            break;

        case MACHINE_REQUEST_CODE_ESEGUI_STEP: {
            uint8_t buffer[256];
            ESP_LOGI(TAG, "Executing step %i", request.step.num);
            size_t len = pack_step(buffer, &request.step.pars, request.step.num);
            res        = invia_pacchetto(ESEGUI_STEP, buffer, len, &risposta_pacchetto, -1);
            break;
        }

        case MACHINE_REQUEST_CODE_AZZERA_ALLARMI:
            res = invia_pacchetto_semplice(COMANDO_AZZERA_ALLARMI, &risposta_pacchetto, -1);
            break;

        case MACHINE_REQUEST_CODE_START:
            res = invia_pacchetto(START_LAVAGGIO, &request.num, 1, &risposta_pacchetto, -1);
            if (res) {
                break;
            }

            if (risposta_pacchetto.command == COMANDO_NACK && risposta_pacchetto.data[1] == IMPOSSIBLE) {
                risposta_task.code = MACHINE_RESPONSE_CODE_REFUSED;
                xQueueSend(responseq, &risposta_task, portMAX_DELAY);
            }
            break;

        case MACHINE_REQUEST_CODE_STOP:
            res = invia_pacchetto_semplice(STOP_LAVAGGIO, &risposta_pacchetto, -1);
            if (res) {
                break;
            }

            if (risposta_pacchetto.command == COMANDO_NACK) {
                if (risposta_pacchetto.data_length == 0) {
                    risposta_task.code = MACHINE_RESPONSE_CODE_REFUSED;
                } else {
                    if (risposta_pacchetto.data[1] == NACK_SCARICO_NECESSARIO) {
                        risposta_task.code = MACHINE_RESPONSE_CODE_DRAIN_REQUIRED;
                    } else if (risposta_pacchetto.data[1] == IMPOSSIBLE) {
                        risposta_task.code = MACHINE_RESPONSE_CODE_REFUSED;
                    }
                }
                xQueueSend(responseq, &risposta_task, portMAX_DELAY);
            }
            break;

        case MACHINE_REQUEST_CODE_PAUSA:
            res = invia_pacchetto_semplice(PAUSA_LAVAGGIO, &risposta_pacchetto, -1);
            if (res) {
                break;
            }

            if (risposta_pacchetto.command == COMANDO_NACK && risposta_pacchetto.data[1] == IMPOSSIBLE) {
                risposta_task.code = MACHINE_RESPONSE_CODE_REFUSED;
                xQueueSend(responseq, &risposta_task, portMAX_DELAY);
            }
            break;

        case MACHINE_REQUEST_CODE_STATS:
            res = invia_pacchetto_semplice(COMANDO_LEGGI_STATISTICHE, &risposta_pacchetto, -1);
            if (res) {
                break;
            }

            risposta_task.code  = MACHINE_RESPONSE_CODE_STATS;
            risposta_task.stats = malloc(sizeof(statistics_t));
            assert(risposta_task.stats != NULL);
            model_deserialize_statistics(risposta_task.stats, &risposta_pacchetto.data[1]);
            xQueueSend(responseq, &risposta_task, portMAX_DELAY);
            break;

        case MACHINE_REQUEST_CODE_TEST:
            res = invia_pacchetto_semplice(request.test ? ENTRA_IN_TEST : ESCI_DAL_TEST, &risposta_pacchetto, 1);
            break;

        case MACHINE_REQUEST_CODE_TEST_DATA:
            res = invia_pacchetto_semplice(COMANDO_LEGGI_TEST, &risposta_pacchetto, -1);
            if (res) {
                break;
            }

            risposta_task.code = MACHINE_RESPONSE_CODE_TEST;
            model_unpack_test(&risposta_task.test, &risposta_pacchetto.data[1]);
            xQueueSend(responseq, &risposta_task, portMAX_DELAY);
            break;

        case MACHINE_REQUEST_CODE_IMPOSTA_USCITA: {
            uint8_t dati[3] = {(uint8_t)request.singola, (uint8_t)request.uscita, (uint8_t)request.valore};
            res             = invia_pacchetto(COMANDO_IMPOSTA_USCITA, dati, sizeof(dati), &risposta_pacchetto, 1);
            break;
        }

        case MACHINE_REQUEST_CODE_RIAVVIA_COMUNICAZIONE:
            // Gestito sopra
            break;
    }

    free_packet(&risposta_pacchetto);
    return res;
}


static int invia_pacchetto_semplice(int comando, packet_t *risposta, int lunghezza_dati_prevista) {
    return invia_pacchetto(comando, NULL, 0, risposta, lunghezza_dati_prevista);
}


static int invia_pacchetto(int comando, uint8_t *dati, uint16_t lunghezza_dati, packet_t *risposta,
                           int lunghezza_dati_prevista) {
    size_t tentativi = 0;
    int    err       = 0;

    size_t   lunghezza_prevista = lunghezza_dati_prevista < 0 ? PACKET_SIZE(512) : PACKET_SIZE(lunghezza_dati_prevista);
    uint8_t *write_buffer       = malloc(PACKET_SIZE(lunghezza_dati));
    uint8_t *read_buffer        = malloc(lunghezza_prevista);
    int      write_len          = build_packet(write_buffer, comando, dati, lunghezza_dati);
    assert(write_len == PACKET_SIZE(lunghezza_dati));

    do {
        machine_serial_write(write_buffer, write_len);

        int len = machine_serial_read(read_buffer, lunghezza_prevista);
        err     = elaborate_data(read_buffer, len, risposta, NULL);
        if (err) {
            ESP_LOGW(TAG, "Risposta non valida al comando 0x%02X: %i (%i)", comando, err, len);
            tentativi++;
        } else if (lunghezza_dati_prevista == -1) {
            ESP_LOGI(TAG, "Len cmd 0x%02X %i", comando, len - PREAMBLE_LEN - HEADER_LEN - CRC_SIZE - 1);
        }
    } while (tentativi++ < TENTATIVI_TOTALI && err != 0);

    free(write_buffer);
    free(read_buffer);

    if (risposta->command != COMANDO_ACK) {
        if (risposta->command == COMANDO_NACK) {
            ESP_LOGW(TAG, "Nack: 0x%02X code 0x%02X", risposta->command, risposta->data[1]);
            return err;
        } else {
            ESP_LOGW(TAG, "Unexpected: 0x%02X", risposta->command);
            return -1;
        }
    }

    if (risposta->data[0] != comando) {
        ESP_LOGW(TAG, "Risposta a comando diverso: mi aspettavo %i, ho ricevuto %i", comando, risposta->data[0]);
        return -1;
    }

    return err;
}


static void free_packet(packet_t *packet) {
    if (packet->data_length > 0) {
        free(packet->data);
    }
}