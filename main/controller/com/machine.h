#ifndef MACHINE_H_INCLUDED
#define MACHINE_H_INCLUDED


#include <stdint.h>
#include <stdlib.h>


typedef enum {
    MACHINE_RESPONSE_CODE_ERRORE_COMUNICAZIONE,
    MACHINE_RESPONSE_CODE_PRESENTAZIONI,
    MACHINE_RESPONSE_CODE_STATO,
} machine_response_code_t;


typedef struct {
    machine_response_code_t code;
    union {
        struct {
            uint8_t n_all;
            uint8_t stato;
            uint8_t nro_programma;
            uint8_t nro_step;
        } presentazioni;
    };
} machine_response_t;


void machine_init(void);
void machine_riavvia_comunicazione(void);
void machine_invia_presentazioni(void);
int  machine_ricevi_risposta(machine_response_t *risposta);
void machine_imposta_uscita_singola(size_t uscita, int valore);
void machine_test(int test);
void machine_richiedi_stato(void);


#endif