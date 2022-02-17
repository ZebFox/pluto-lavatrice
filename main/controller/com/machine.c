#include "packet.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
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
    MACHINE_REQUEST_CODE_STATO,
    MACHINE_REQUEST_CODE_RIAVVIA_COMUNICAZIONE,
    MACHINE_REQUEST_CODE_IMPOSTA_USCITA,
} machine_request_code_t;


typedef struct {
    machine_request_code_t code;
    union {
        struct {
            int    singola;
            size_t uscita;
            int    valore;
        };
        int test;
    };
} machine_request_t;


static void communication_task(void *args);
static int  invia_pacchetto_semplice(int comando, packet_t *risposta, int lunghezza_dati_prevista);
static int  invia_pacchetto(int comando, uint8_t *dati, uint8_t lunghezza_dati, packet_t *risposta,
                            int lunghezza_dati_prevista);
static int  task_gestisci_richiesta(machine_request_t request);


static const char      *TAG       = "Machine";
static QueueHandle_t    requestq  = NULL;
static QueueHandle_t    responseq = NULL;
static stato_macchina_t stato     = {0};


void machine_init(void) {
    static StaticQueue_t static_requestq_queue;
    static uint8_t       requestq_queue_buffer[sizeof(machine_request_t) * 8] = {0};
    requestq = xQueueCreateStatic(sizeof(requestq_queue_buffer) / sizeof(machine_request_t), sizeof(machine_request_t),
                                  requestq_queue_buffer, &static_requestq_queue);

    static StaticQueue_t static_responseq_queue;
    static uint8_t       responseq_queue_buffer[sizeof(machine_response_t) * 4] = {0};
    responseq = xQueueCreateStatic(sizeof(responseq_queue_buffer) / sizeof(machine_response_t),
                                   sizeof(machine_response_t), responseq_queue_buffer, &static_responseq_queue);

    static StaticTask_t static_task;
    static StackType_t  task_stack[BASE_TASK_STACK_SIZE * 5] = {0};
    xTaskCreateStatic(communication_task, TAG, sizeof(task_stack), NULL, 1, task_stack, &static_task);
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

    for (;;) {
        machine_request_t request = {0};

        if (xQueueReceive(requestq, &request, portMAX_DELAY)) {
            if (errore_comunicazione && request.code == MACHINE_REQUEST_CODE_RIAVVIA_COMUNICAZIONE) {
                errore_comunicazione = task_gestisci_richiesta(ultima_richiesta_fallita);
                if (errore_comunicazione) {
                    ESP_LOGW(TAG, "Risposta assente o non valida (durante il riavvio)!");
                    xQueueSend(responseq, &risposta_errore, portMAX_DELAY);
                    continue;
                }
            } else if (!errore_comunicazione) {
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


static int task_gestisci_richiesta(machine_request_t request) {
    packet_t           risposta_pacchetto = {0};
    int                res                = 0;
    machine_response_t risposta_task      = {0};

    switch (request.code) {
        case MACHINE_REQUEST_CODE_PRESENTAZIONI: {
            res = invia_pacchetto_semplice(COMANDO_PRESENTAZIONI, &risposta_pacchetto, 6);
            if (res) {
                break;
            }

            risposta_task.code = MACHINE_RESPONSE_CODE_PRESENTAZIONI;
            deserialize_uint8(&risposta_task.presentazioni.n_all, &risposta_pacchetto.data[1]);
            deserialize_uint8(&risposta_task.presentazioni.stato, &risposta_pacchetto.data[3]);
            deserialize_uint8(&risposta_task.presentazioni.nro_programma, &risposta_pacchetto.data[4]);
            deserialize_uint8(&risposta_task.presentazioni.nro_step, &risposta_pacchetto.data[5]);
            free(risposta_pacchetto.data);

            xQueueSend(responseq, &risposta_task, portMAX_DELAY);
            break;
        }

        case MACHINE_REQUEST_CODE_STATO:
            res = invia_pacchetto_semplice(COMANDO_LEGGI_STATO, &risposta_pacchetto, -1);
            if (res) {
                break;
            }

            risposta_task.code = MACHINE_RESPONSE_CODE_STATO;
            // model_unpack_stato_macchina(&, uint8_t *buffer)
            //  TODO: spacchetta lo stato
            //  Attenzione: lo stato macchina comporta una grossa quantita' di dati, ed e' inopportuno inserirli nel
            //  messaggio di risposta onde evitare che la coda esploda in dimensione. Piuttosto e' meglio avere
            //  un'area di memoria coperta da semaforo che viene usata come casella postale quando arriva la
            //  risposta.
            free(risposta_pacchetto.data);
            xQueueSend(responseq, &risposta_task, portMAX_DELAY);
            break;

        case MACHINE_REQUEST_CODE_TEST:
            res = invia_pacchetto_semplice(request.test ? ENTRA_IN_TEST : ESCI_DAL_TEST, &risposta_pacchetto, 1);
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

    return res;
}


static int invia_pacchetto_semplice(int comando, packet_t *risposta, int lunghezza_dati_prevista) {
    return invia_pacchetto(comando, NULL, 0, risposta, lunghezza_dati_prevista);
}


static int invia_pacchetto(int comando, uint8_t *dati, uint8_t lunghezza_dati, packet_t *risposta,
                           int lunghezza_dati_prevista) {
    uint8_t buffer[MAX_PACKET] = {0};
    size_t  tentativi          = 0;
    int     err                = 0;

    size_t lunghezza_prevista = lunghezza_dati_prevista < 0
                                    ? sizeof(buffer)
                                    : PREAMBLE_LEN + HEADER_LEN + CRC_SIZE + (size_t)lunghezza_dati_prevista + 1;

    do {
        int len = build_packet(buffer, comando, dati, lunghezza_dati);
        machine_serial_write(buffer, len);

        len = machine_serial_read(buffer, lunghezza_prevista);
        err = elaborate_data(buffer, len, risposta, NULL);
        if (err) {
            ESP_LOGW(TAG, "Risposta non valida al comando 0x%02X: %i (%i)", comando, err, len);
            tentativi++;
        }
    } while (tentativi++ < TENTATIVI_TOTALI && err != 0);

    if (risposta->command != COMANDO_ACK) {
        ESP_LOGW(TAG, "Nack: %i", risposta->command);
        return -1;
    }

    if (risposta->data[0] != comando) {
        ESP_LOGW(TAG, "Risposta a comando diverso: mi aspettavo %i, ho ricevuto %i", comando, risposta->data[0]);
        return -1;
    }

    return err;
}