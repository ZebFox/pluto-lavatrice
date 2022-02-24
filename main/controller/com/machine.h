#ifndef MACHINE_H_INCLUDED
#define MACHINE_H_INCLUDED


#include <stdint.h>
#include <stdlib.h>
#include "model/model.h"


typedef enum {
    MACHINE_RESPONSE_CODE_ERRORE_COMUNICAZIONE,
    MACHINE_RESPONSE_CODE_PRESENTAZIONI,
    MACHINE_RESPONSE_CODE_STATO,
    MACHINE_RESPONSE_CODE_TEST,
} machine_response_code_t;


typedef struct {
    machine_response_code_t code;
    union {
        struct {
            uint8_t n_all;
            uint8_t stato;
            uint8_t nro_programma;
            uint8_t nro_step;
            name_t  machine_fw_version;
            name_t  machine_fw_date;
        } presentazioni;
        test_data_t test;
    };
} machine_response_t;


void machine_init(void);
void machine_riavvia_comunicazione(void);
void machine_invia_presentazioni(void);
int  machine_ricevi_risposta(machine_response_t *risposta);
void machine_imposta_uscita_singola(size_t uscita, int valore);
void machine_test(int test);
void machine_richiedi_stato(void);
void machine_read_state(model_t *pmodel);
void machine_richiedi_dati_test(void);
void machine_invia_parmac(parmac_t *parmac);
void machine_azzera_allarmi(void);
void machine_start(uint8_t num_program);
void machine_esegui_step(parametri_step_t *step, uint8_t num);
void machine_stop(void);
void machine_pause(void);


#endif