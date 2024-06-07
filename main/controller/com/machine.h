#ifndef MACHINE_H_INCLUDED
#define MACHINE_H_INCLUDED


#include <stdint.h>
#include <stdlib.h>
#include "model/model.h"


typedef enum {
    MACHINE_RESPONSE_CODE_ERRORE_COMUNICAZIONE,
    MACHINE_RESPONSE_CODE_PRESENTAZIONI,
    MACHINE_RESPONSE_CODE_STATO,
    MACHINE_RESPONSE_CODE_STATS,
    MACHINE_RESPONSE_CODE_TEST,
    MACHINE_RESPONSE_CODE_REFUSED,
    MACHINE_RESPONSE_CODE_DRAIN_REQUIRED,
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
        test_data_t   test;
        statistics_t *stats;
    };
} machine_response_t;


void machine_init(void);
void machine_riavvia_comunicazione(void);
void machine_invia_presentazioni(void);
int  machine_ricevi_risposta(machine_response_t *risposta);
void machine_imposta_uscita_singola(size_t uscita, int valore);
void machine_test(int test);
void machine_richiedi_stato(void);
int  machine_read_state(model_t *pmodel);
void machine_richiedi_dati_test(void);
void machine_invia_parmac(parmac_t *parmac);
void machine_azzera_allarmi(void);
void machine_start(uint8_t num_program);
void machine_esegui_step(parametri_step_t *step, uint8_t num);
void machine_stop(void);
void machine_pause(void);
void machine_forza_scarico(void);
void machine_offset_pressione(void);
void machine_azzera_litri(void);
void machine_apri_oblo(int forza);
void machine_abilita_comunicazione(size_t en);
void machine_read_stats(model_t *pmodel);
void machine_chiudi_oblo(void);
void machine_send_debug_code(uint8_t debug_code);
void machine_send_time(void);
void machine_modify_cycle_parameters(uint8_t step, uint16_t duration, uint16_t speed, uint16_t temperature,
                                     uint16_t level);
void machine_control_detergent(uint8_t detergent, uint8_t value);
void machine_activate_detergent(uint8_t detergent);
void machine_imposta_uscita_multipla(size_t uscita, int valore);
void machine_imposta_dac(uint8_t dac);
void machine_imposta_led(uint8_t led);
void machine_azzera_credito(void);
void machine_payment_state(uint8_t payment_state);
void machine_enable_digital_coin_reader(uint8_t enable);

#endif
