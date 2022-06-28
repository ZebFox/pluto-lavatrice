#ifndef PROGRAMS_H_INCLUDED
#define PROGRAMS_H_INCLUDED

#include <stdint.h>
#include <stdlib.h>

#define MAX_LINGUE          10
#define MAX_PROGRAMMI       100
#define MAX_NAME_LENGTH     32
#define STRING_NAME_SIZE    (MAX_NAME_LENGTH + 1)
#define MAX_STEPS           36
#define STEP_SIZE           256
#define MAX_DETERGENTS      10
#define PROGRAM_SIZE(steps) ((size_t)(338 + STEP_SIZE * steps))
#define MAX_PROGRAM_SIZE    PROGRAM_SIZE(MAX_STEPS)

typedef char name_t[STRING_NAME_SIZE];

// codici FUNZIONI LAVAGGIO disponibili
#define STEP_AMMOLLO      1
#define STEP_PRELAVAGGIO  2
#define STEP_LAVAGGIO     3
#define STEP_RISCIACQUO   4
#define STEP_SCARICO      5
#define STEP_CENTRIFUGA   6
#define STEP_SROTOLAMENTO 7
#define STEP_ATTESA       8
#define NUM_STEPS         8

#define DELICATO 0
#define ENERGICO 1

enum {
    TIPO_PROG_MOLTO_SPORCHI_PRELAVAGGIO = 0,
    TIPO_PROG_SPORCHI_PRELAVAGGIO,
    TIPO_PROG_MOLTO_SPORCHI,
    TIPO_PROG_SPORCHI,
    TIPO_PROG_COLORATI,
    TIPO_PROG_SINTETICI,
    TIPO_PROG_PIUMONI,
    TIPO_PROG_DELICATI_FREDDO,
    TIPO_PROG_LANA,
    TIPO_PROG_LINO_TENDAGGI,
    TIPO_PROG_SOLO_CENTRIFUGA_1000,
    TIPO_PROG_SOLO_CENTRIFUGA_600,
    TIPO_PROG_SANIFICAZIONE,
    TIPO_PROG_AMMOLLO,
    TIPO_PROG_PRELAVAGGIO_CENTRIFUGA,
    TIPO_PROG_RISCIACQUO_CENTRIFUGA,
    NUM_TIPI_PROGRAMMA,
};

typedef struct {
    uint16_t tipo;

    // Almeno due byte
    uint16_t durata;
    uint16_t tempo_attivo;
    uint16_t velocita_riempimento;
    uint16_t giro_riempimento;
    uint16_t pausa_riempimento;
    uint16_t giro_lavaggio;
    uint16_t pausa_lavaggio;
    uint16_t temperatura;
    uint16_t livello;
    uint16_t tempo_attivo_sapone;
    uint16_t velocita_lavaggio;
    uint16_t tempo_avviso_attesa_on;
    uint16_t tempo_avviso_attesa_off;
    uint16_t tempo_preparazione;
    uint16_t velocita_preparazione;
    uint16_t tempo_scarico;
    uint16_t numero_rampe;
    uint16_t velocita_centrifuga_1;
    uint16_t velocita_centrifuga_2;
    uint16_t velocita_centrifuga_3;
    uint16_t tempo_rampa_1;
    uint16_t tempo_rampa_2;
    uint16_t tempo_rampa_3;
    uint16_t tempo_attesa_centrifuga_1;
    uint16_t tempo_attesa_centrifuga_2;
    uint16_t tempo_frenata;
    uint16_t tempo_sapone_1, tempo_sapone_2, tempo_sapone_3, tempo_sapone_4, tempo_sapone_5, tempo_sapone_6,
        tempo_sapone_7, tempo_sapone_8, tempo_sapone_9, tempo_sapone_10;
    uint16_t ritardo_sapone_1, ritardo_sapone_2, ritardo_sapone_3, ritardo_sapone_4, ritardo_sapone_5, ritardo_sapone_6,
        ritardo_sapone_7, ritardo_sapone_8, ritardo_sapone_9, ritardo_sapone_10;

    uint16_t tempo_attesa;

    // Un byte soltanto
    uint16_t riscaldamento_diretto_indiretto;
    uint16_t abilitazione_controllo_temperatura_continuo;
    uint16_t abilitazione_riscaldamento;
    uint16_t abilitazione_inversione_lavaggio;
    uint16_t abilitazione_inversione_riempimento;
    uint16_t abilitazione_moto_fermo_riempimento;
    uint16_t abilitazione_moto_fermo;
    uint16_t abilitazione_ricircolo;
    uint16_t abilitazione_acqua_fredda;
    uint16_t abilitazione_acqua_calda;
    uint16_t abilitazione_acqua_depurata;
    uint16_t abilitazione_recupero;
} parametri_step_t;

typedef struct {
    name_t   filename;
    name_t   nomi[MAX_LINGUE];
    uint32_t prezzo;
    uint8_t  tipo;

    size_t           num_steps;
    parametri_step_t steps[MAX_STEPS];
} programma_lavatrice_t;


typedef struct {
    uint32_t prezzo;
    uint16_t num_steps;
    uint16_t lavaggi;
    uint16_t velocita;
    uint16_t temperatura;
    uint16_t livello;
    uint16_t durata;
    uint8_t  tipo;
    name_t   filename;
    name_t   name;
} programma_preview_t;


void init_new_program(programma_lavatrice_t *p, int num);
void update_program_name(programma_lavatrice_t *p, const char *str, int lingua);
void update_program_price(programma_lavatrice_t *p, const char *string);
void update_program_type(programma_lavatrice_t *p, unsigned char type);
void program_add_step(programma_lavatrice_t *p, int tipo);
void swap_steps(programma_lavatrice_t *p, int first, int second);
void programs_remove_step(programma_lavatrice_t *p, int index);
void program_insert_step(programma_lavatrice_t *p, int tipo, size_t index);

parametri_step_t default_step(int tipo, int delicato_energico);
int              pack_step(uint8_t *buffer, const parametri_step_t *step, int num);
size_t           deserialize_program(programma_lavatrice_t *p, uint8_t *buffer);
size_t           serialize_program(uint8_t *buffer, programma_lavatrice_t *p);
size_t           program_serialize_empty(uint8_t *buffer, uint16_t num);

#endif
