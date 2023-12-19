#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include <time.h>
#include <stdint.h>
#include "programs.h"
#include "./event.h"


#define NUM_MAX_SAPONI 10

#define LIVELLO_CENTIMETRI 0
#define LIVELLO_LITRI      1

#define BIT_NESSUNO      0
#define BIT_UTENTE       0x1
#define LVL_UTENTE       BIT_UTENTE
#define BIT_TECNICO      0x2
#define LVL_TECNICO      (BIT_UTENTE | BIT_TECNICO)
#define BIT_DISTRIBUTORE 0x4
#define LVL_DISTRIBUTORE (LVL_TECNICO | BIT_DISTRIBUTORE)
#define BIT_COSTRUTTORE  0x8
#define LVL_COSTRUTTORE  (LVL_DISTRIBUTORE | BIT_COSTRUTTORE)

#define CODICE_LVL_COSTRUTTORE 3

#define STATO_MACCHINA_STOP            0
#define STATO_MACCHINA_MARCIA          1
#define STATO_MACCHINA_PAUSA           2
#define STATO_MACCHINA_SCARICO_FORZATO 3
#define STATO_MACCHINA_FRENATA         6

#define LINEE_PAGAMENTO_GETTONIERA 9
#define LINEE_GETTONIERA_DIGITALE  5
#define LINEE_GETTONIERA_MECCANICA 2

#define LINEA_GETTONIERA_SINGOLA    0
#define LINEA_1_GETTONIERA_DIGITALE 1
#define LINEA_1_GETTONIERA_IMPULSI  6
#define LINEA_GETTONIERA_CASSA      8

#define MAX_LOG_ACCELEROMETRO 40

#define LIMITE_SALVATAGGIO 5
#define PARMAC_SIZE        279

#define EVENT_LOG_CHUNK 50

#define NUM_LINGUE 2

#if NUM_LINGUE > MAX_LINGUE
#error "Too many languages!"
#endif


typedef enum {
    REMOVABLE_DRIVE_STATE_MISSING,
    REMOVABLE_DRIVE_STATE_MOUNTED,
    REMOVABLE_DRIVE_STATE_INVALID,
} removable_drive_state_t;


enum {
    PAGAMENTO_NESSUNO = 0,
    PAGAMENTO_1_LINEA_NA,
    PAGAMENTO_1_LINEA_NC,
    PAGAMENTO_2_LINEA_NA,
    PAGAMENTO_2_LINEA_NC,
    PAGAMENTO_DIGITALE,
    PAGAMENTO_DIGITALE_LINEA_SINGOLA,
    PAGAMENTO_CASSA_NA,
    PAGAMENTO_CASSA_NC,
};

enum {
    CONDIZIONE_MACCHINA_FERMO = 0,
    CONDIZIONE_MACCHINA_LAVORO,
    CONDIZIONE_MACCHINA_PAUSA,
    CONDIZIONE_MACCHINA_ATTESA,
    CONDIZIONE_MACCHINA_AVVISO,
    CONDIZIONE_MACCHINA_ALLARME,
    NUM_CONDIZIONI_MACCHINA,
};

typedef enum {
    NETWORK_CONNECTED,
    NETWORK_CONNECTING,
    NETWORK_SCANNING,
    NETWORK_INACTIVE,
} network_status_t;

typedef struct {
    name_t   nome;
    uint16_t lingua;
    uint16_t lingua_max_bandiera;
    uint16_t logo;
    uint16_t modello_macchina;
    uint16_t submodello_macchina;
    uint16_t codice_nodo_macchina;
    uint16_t diametro_cesto;
    uint16_t profondita_cesto;
    uint16_t altezza_trappola;
    uint16_t correzione_contagiri;
    uint16_t numero_massimo_programmi;
    uint16_t numero_massimo_programmi_utente;
    uint16_t visualizzazione_opl;
    uint16_t livello_accesso;
    uint16_t visualizzazione_stop;
    uint16_t visualizzazione_start;
    uint16_t visualizzazione_menu;
    uint16_t visualizzazione_menu_saponi;
    uint16_t visualizzazione_tot_cicli;
    uint16_t abilitazione_lavaggio_programmato;
    uint16_t visualizzazione_kg;
    uint16_t visualizzazione_prezzo_macchina;
    uint16_t visualizzazione_help_1;
    uint16_t visualizzazione_help_2;
    uint16_t visualizzazione_help_3;
    uint16_t visualizzazione_help_4;
    uint16_t visualizzazione_help_5;
    uint16_t secondi_pausa;
    uint16_t secondi_stop;
    uint16_t tempo_out_pagine;
    uint16_t tempo_allarme_livello;
    uint16_t tempo_allarme_temperatura;
    uint16_t tempo_allarme_scarico;
    uint16_t tempo_ritardo_micro_oblo;
    uint16_t tipo_out_aux_1;
    uint16_t tipo_out_aux_2;
    uint16_t tipo_out_aux_3;
    uint16_t tipo_out_aux_4;
    uint16_t f_scarico_recupero;
    uint16_t abilitazione_macchina_libera;
    uint16_t f_macchina_libera;
    uint16_t abilitazione_espansione_io;
    uint16_t tipo_gettoniera;
    uint16_t prezzo_unico;
    uint16_t valore_impulso;
    uint16_t richiesta_pagamento;
    uint16_t cifre_prezzo;
    uint16_t cifre_decimali_prezzo;
    uint16_t modo_vis_prezzo;
    uint16_t isteresi_temperatura;
    uint16_t temperatura_sicurezza;
    uint16_t temperatura_termodegradazione;
    uint16_t tipo_livello;
    uint16_t impulsi_litro;
    uint16_t tempo_isteresi_livello;
    uint16_t centimetri_max_livello;
    uint16_t centimetri_minimo_scarico;
    uint16_t centimetri_minimo_riscaldo;
    uint16_t litri_massimi;
    uint16_t livello_sfioro;
    uint16_t litri_minimi_riscaldo;
    uint16_t tempo_minimo_scarico;
    uint16_t tempo_scarico_servizio;
    uint16_t tempo_colpo_aperto_scarico;
    uint16_t tipo_inverter;
    uint16_t velocita_minima_lavaggio;
    uint16_t velocita_massima_lavaggio;
    uint16_t velocita_servizio;
    uint16_t abilitazione_preparazione_rotazione;
    uint16_t tempo_marcia_preparazione_rotazione;
    uint16_t tempo_sosta_preparazione_rotazione;
    uint16_t velocita_minima_preparazione;
    uint16_t velocita_massima_preparazione;
    uint16_t velocita_minima_centrifuga_1;
    uint16_t velocita_massima_centrifuga_1;
    uint16_t velocita_minima_centrifuga_2;
    uint16_t velocita_massima_centrifuga_2;
    uint16_t velocita_minima_centrifuga_3;
    uint16_t velocita_massima_centrifuga_3;
    uint16_t tempo_minimo_rampa;
    uint16_t tempo_massimo_rampa;
    uint16_t nro_max_sbilanciamenti;
    uint16_t f_proximity;
    uint16_t tempo_minimo_frenata;
    uint16_t numero_raggi;
    uint16_t abilitazione_accelerometro;
    uint16_t scala_accelerometro;
    uint16_t soglia_x_accelerometro;
    uint16_t soglia_y_accelerometro;
    uint16_t soglia_z_accelerometro;
    uint16_t soglia_x_accelerometro_h;
    uint16_t soglia_y_accelerometro_h;
    uint16_t soglia_z_accelerometro_h;
    uint16_t giri_accelerometro;
    uint16_t giri_accelerometro_2;
    uint16_t delta_val_accelerometro;
    uint16_t tempo_attesa_accelerometro;
    uint16_t tempo_scarico_accelerometro;
    uint16_t numero_saponi_utilizzabili;
    uint16_t tempo_h2o_pulizia_saponi;
    uint16_t tempo_precarica;
    uint16_t tempo_tasto_carico_saponi;
    uint16_t abilitazione_min_sec;
    uint16_t abilitazione_sblocco_get;
    uint16_t tipo_serratura;
    uint16_t durata_impulso_serratura;
    uint16_t inibizione_allarmi;
    uint16_t autoavvio;
    uint16_t abilitazione_loop_prog;
    uint16_t temperatura_massima;
    uint16_t valore_prezzo_unico;
    uint16_t percentuale_livello_carico_ridotto;
    uint16_t percentuale_sapone_carico_ridotto;

    uint8_t  funzioni_rgb[NUM_CONDIZIONI_MACCHINA];
    uint16_t visualizzazione_data_ora;
    uint16_t visualizzazione_esclusione_sapone;
    uint16_t visualizzazione_pedante;
    uint16_t esclusione_sapone;
    uint16_t tipo_frontale;
} parmac_t;

typedef struct {
    uint8_t stato;
    uint8_t sottostato;
    uint8_t codice_step;
    uint8_t sottostato_step;
    uint8_t codice_allarme;
    uint8_t credito[LINEE_PAGAMENTO_GETTONIERA];
    uint8_t pagato;

    uint8_t allarme_oblo_aperto;
    uint8_t allarme_oblo_sbloccato;
    uint8_t allarme_chiavistello;
    uint8_t allarme_errore_ram;
    uint8_t allarme_power_off;
    uint8_t allarme_emergenza;
    uint8_t allarme_inverter_ko;
    uint8_t allarme_sbilanciamento;
    uint8_t allarme_emergenza_marcia;
    uint8_t allarme_no_riempimento;
    uint8_t allarme_no_scarico;
    uint8_t allarme_sonda_temperatura;
    uint8_t allarme_sovratemperatura;
    uint8_t allarme_no_riscaldamento;

    uint8_t chiavistello_aperto, chiavistello_chiuso;
    uint8_t livello_riscaldamento_ok;
    uint8_t livello_scarico_ok;
    uint8_t livello_ok;
    uint8_t temperatura_ok;
    uint8_t acqua_fredda;
    uint8_t acqua_calda;
    uint8_t out_scarico;
    uint8_t out_riscaldamento;
    uint8_t abilitazione_riscaldamento;
    uint8_t abilitazione_moto_fermo_riempimento;
    uint8_t abilitazione_inversione_riempimento;
    uint8_t abilitazione_inversione_lavaggio;
    uint8_t out_motore_avanti;
    uint8_t out_motore_indietro;
    uint8_t condizionamento_livello;
    uint8_t condizionamento_temperatura;
    uint8_t condizionamento_livello_saponi;
    uint8_t condizionamento_temperatura_saponi;
    uint8_t precarica;
    uint8_t alt_tempo_durata;
    uint8_t alt_tempo_durata_saponi;
    uint8_t out_flusso;
    uint8_t allarme_sblocco_oblo;
    uint8_t allarme_oblo_h2o;
    uint8_t termodegradazione;
    uint8_t preparazione_centrifuga;
    uint8_t vis_popup_frenata;
    uint8_t sbilanciamento;
    uint8_t cicli_sbilanciamento;
    uint8_t cicli_sbilanciamento_massimi;
    uint8_t tipo_sbilanciamento;
    uint8_t abilitazione_preparazione;
    uint8_t cicli_preparazione;

    uint16_t tempo_precarica;

    uint8_t  out_saponi[10];
    uint16_t tempo_saponi[10];
    uint16_t ritardo_saponi[10];

    uint16_t tempo_flusso;
    uint16_t tempo_giro_riempimento;
    uint16_t tempo_giro_lavaggio;
    uint16_t tempo_pausa_riempimento;
    uint16_t tempo_pausa_lavaggio;
    uint16_t tempo_moto_cesto;
    uint16_t velocita_rpm;
    uint16_t velocita_volt;
    uint16_t velocita_rilevata;
    uint16_t frenata;

    uint16_t tempo_allarme_livello;
    uint16_t tempo_allarme_scarico;
    uint16_t tempo_allarme_temperatura;

    uint16_t numero_programma;
    uint16_t numero_step;
    uint16_t rimanente;
    char     oblo_aperto_chiuso;     // 0 - aperto; 1 - chiuso
    uint16_t livello, livello_litri, temperatura;
    uint16_t livello_impostato, temperatura_impostata;

    uint16_t descrizione_pedante;
    uint8_t  richiesto_aggiornamento_tempo;
} stato_macchina_t;


typedef struct {
    uint32_t cicli_eseguiti;
    uint32_t cicli_interrotti;
    uint32_t cicli_loop;
    uint32_t tempo_accensione;
    uint32_t tempo_lavoro;
    uint32_t tempo_moto;
    uint32_t tempo_riscaldamento;
    uint32_t tempo_h2o_fredda;
    uint32_t tempo_h2o_calda;
    uint32_t tempo_h2o_rec_dep;
    uint32_t tempo_h2o_flusso;
    uint32_t tempo_saponi[NUM_MAX_SAPONI];
    uint32_t chiusure_oblo;
    uint32_t aperture_oblo;
} statistics_t;


typedef struct {
    uint16_t     inputs;
    uint8_t      inputs_exp;
    uint16_t     adc_temp, adc_press;
    uint16_t     offset_press;
    uint32_t     pmin, pmax;
    uint8_t      gettoniera_impulsi_abilitata;
    uint32_t     minp[3], maxp[3];
    uint8_t      accelerometro_ok;
    unsigned int log_accelerometro[3];
} test_data_t;


typedef struct {
    statistics_t stats;
    test_data_t  test;     // Informazioni relative alle schermate di test

    struct {
        parmac_t            parmac;
        size_t              num_programmi;
        programma_preview_t preview_programmi[MAX_PROGRAMMI];
        uint8_t             contrast;
    } prog;     // Programmazione (parametri e lavaggi)

    struct {
        uint16_t num_step_corrente, num_step_successivo;

        int                   maybe_programma;
        uint16_t              num_programma_caricato;
        programma_lavatrice_t programma_caricato;

        int model_lavaggio_finito;
        int f_richiedi_scarico;
        int lingua;

        uint16_t         credito;
        stato_macchina_t macchina;

        struct {
            int    attivo;
            size_t lavaggio;
            time_t start;
        } lavaggio_programmato;

        size_t  event_log_number;
        size_t  total_event_log_number;
        event_t event_log_chunk[EVENT_LOG_CHUNK];

        uint8_t done;
        int     livello_accesso_temporaneo;
    } run;     // Informazioni relative all'esecuzione attuale (sia della scheda quadro che dell'applicazione)

    struct {
        char wifi_ipaddr[16];
        char machine_fw_version[STRING_NAME_SIZE];
        char machine_fw_date[STRING_NAME_SIZE];
        char ssid[STRING_NAME_SIZE];

        int f_connected;
        int f_versioni_diverse;

        size_t  num_archivi;
        name_t *archivi;
        int     removable_drive_state;

        int errore_comunicazione;
        int comunicazione_abilitata;

        unsigned int debug_code;
    } system;     // Dati del sistema Linux sottostante
} model_t;


/* Inizializzazione */

void model_init(model_t *model);


uint16_t                   model_get_language(model_t *pmodel);
uint16_t                   model_get_temporary_language(model_t *pmodel);
size_t                     model_deserialize_parmac(parmac_t *p, uint8_t *buffer);
size_t                     model_serialize_parmac(uint8_t *buffer, parmac_t *p);
void                       model_unpack_stato_macchina(stato_macchina_t *stato, uint8_t *buffer);
size_t                     model_get_num_programs(model_t *pmodel);
const programma_preview_t *model_get_preview(model_t *pmodel, size_t i);
void                       model_unpack_test(test_data_t *test, uint8_t *buffer);
size_t                     model_pack_parametri_macchina(uint8_t *buffer, parmac_t *p);
char                      *model_new_unique_filename(model_t *model, char *filename, unsigned long seed);
programma_lavatrice_t     *model_get_program(model_t *pmodel);
void                       model_sync_program_preview(model_t *pmodel);
int                        model_select_program_step(model_t *model, size_t i, size_t step);
uint16_t                   model_get_preparation_time(model_t *pmodel);
void                       model_formatta_prezzo(char *string, model_t *model, unsigned int prezzo);
unsigned int               model_get_credito(model_t *pmodel);
parametri_step_t          *model_get_current_step(model_t *pmodel);
uint16_t                   model_get_current_step_number(model_t *pmodel);
int                        model_macchina_in_pausa(model_t *model);
int                        model_macchina_in_frenata(model_t *model);
int                        model_macchina_in_scarico_forzato(model_t *model);
int                        model_macchina_in_stop(model_t *model);
void                       model_azzera_lavaggio(model_t *pmodel);
int                        model_macchina_in_marcia(model_t *model);
int                        model_step_finito(model_t *model);
int                        model_lavaggio_finito(model_t *model);
uint16_t                   model_get_livello_centimetri(model_t *pmodel);
int                        model_lavaggio_pagato(model_t *pmodel, size_t num_prog);
parametri_step_t          *model_get_program_step(model_t *pmodel, size_t num);
void                       model_deserialize_statistics(statistics_t *stats, uint8_t *buffer);
int                        model_is_communication_ok(model_t *pmodel);
uint8_t                    model_get_bit_accesso(uint8_t al);
uint16_t                   model_get_program_num(model_t *pmodel);
int                        model_can_work(model_t *pmodel);
void                       model_update_preview(model_t *pmodel);
void                       model_reset_temporary_language(model_t *pmodel);
int                        model_oblo_serrato(model_t *pmodel);
int                        model_oblo_libero(model_t *pmodel);
int                        model_oblo_chiuso(model_t *pmodel);
uint16_t                   model_get_livello(model_t *pmodel);
void                       model_arretra_step(model_t *model);
void                       model_avanza_step(model_t *model);
uint16_t                   model_alarm_code(model_t *pmodel);
int                        model_requested_time(model_t *pmodel);
uint16_t                   model_program_remaining(model_t *pmodel);
int                        model_is_level_in_cm(parmac_t *parmac);
size_t                     model_get_num_user_programs(model_t *pmodel);
void                       model_set_drive_mounted(model_t *pmodel, removable_drive_state_t drive_mounted);
int                        model_get_minimo_livello_riscaldo(model_t *model);
int                        model_get_velocita_corretta(model_t *model);
void program_deserialize_preview(model_t *pmodel, programma_preview_t *p, uint8_t *buffer, uint16_t lingua);


#endif
