#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "programs.h"
//#include "view/lingue.h"
#include "utils/utils.h"
#include "assert.h"
#include "gel/serializer/serializer.h"


#define PACK_BYTE(buffer, num, i)     PACK_UINT8(&buffer[i], num)
#define PACK_SHORT_BE(buffer, num, i) PACK_UINT16_BE(&buffer[i], num)


static void init_names(name_t *names, uint16_t num);


void update_program_name(programma_lavatrice_t *p, const char *str, int lingua) {
    if (strlen(str) > 0) {
        strcpy(p->nomi[lingua], str);
        p->modificato = 1;
    }
}

void update_program_price(programma_lavatrice_t *p, const char *string) {
    char *tmp = malloc(sizeof(string) + 1);
    strcpy(tmp, string);

    char *src, *dst;
    for (src = dst = tmp; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != '.')
            dst++;
    }
    *dst = '\0';

    p->prezzo     = atoi(tmp);
    p->modificato = 1;
    free(tmp);
}

void update_program_type(programma_lavatrice_t *p, unsigned char type) {
    p->tipo       = type;
    p->modificato = 1;
}

void add_step(programma_lavatrice_t *p, int tipo, int delicato_energetico) {
    insert_step(p, tipo, p->num_steps, delicato_energetico);
}

void insert_step(programma_lavatrice_t *p, int tipo, size_t index, int delicato_energetico) {
    if (p->num_steps < MAX_STEPS && index <= p->num_steps) {
        for (int i = (int)p->num_steps - 1; i >= (int)index; i--)
            p->steps[i + 1] = p->steps[i];

        p->steps[index] = default_step(tipo, delicato_energetico);
        p->num_steps++;
        p->modificato = 1;
    }
}

void remove_step(programma_lavatrice_t *p, int index) {
    int len = p->num_steps;

    for (int i = index; i < len - 1; i++) {
        p->steps[i] = p->steps[i + 1];
    }

    if (p->num_steps > 0) {
        p->num_steps--;
        p->modificato = 1;
    }
}

void swap_steps(programma_lavatrice_t *p, int first, int second) {
    if (first != second) {
        parametri_step_t s = p->steps[first];
        p->steps[first]    = p->steps[second];
        p->steps[second]   = s;
        p->modificato      = 1;
    }
}


parametri_step_t default_step(int tipo, int delicato_energico) {
    parametri_step_t step = {0};
    step.tipo             = tipo;

    switch (step.tipo) {
        case STEP_AMMOLLO:
        case STEP_PRELAVAGGIO:
            step.durata                                      = 180;
            step.tempo_attivo                                = 3;
            step.abilitazione_moto_fermo_riempimento         = delicato_energico == DELICATO ? 0 : 1;
            step.velocita_riempimento                        = delicato_energico == DELICATO ? 25 : 45;
            step.abilitazione_inversione_riempimento         = 1;
            step.giro_riempimento                            = delicato_energico == DELICATO ? 8 : 26;
            step.pausa_riempimento                           = delicato_energico == DELICATO ? 10 : 5;
            step.velocita_lavaggio                           = delicato_energico == DELICATO ? 25 : 45;
            step.abilitazione_inversione_lavaggio            = 1;
            step.giro_lavaggio                               = delicato_energico == DELICATO ? 8 : 26;
            step.pausa_lavaggio                              = delicato_energico == DELICATO ? 10 : 5;
            step.abilitazione_riscaldamento                  = 1;
            step.temperatura                                 = delicato_energico == DELICATO ? 30 : 40;
            step.riscaldamento_diretto_indiretto             = 0;
            step.abilitazione_controllo_temperatura_continuo = 0;
            step.livello                                     = 8;
            step.abilitazione_ricircolo                      = 0;
            step.abilitazione_acqua_fredda                   = 1;
            step.abilitazione_acqua_calda                    = 0;
            step.abilitazione_acqua_depurata                 = 0;
            step.tempo_attivo_sapone                         = 1;
            break;

        case STEP_LAVAGGIO:
            step.durata                                      = 600;
            step.tempo_attivo                                = 3;
            step.abilitazione_moto_fermo_riempimento         = delicato_energico == DELICATO ? 0 : 1;
            step.velocita_riempimento                        = delicato_energico == DELICATO ? 25 : 45;
            step.abilitazione_inversione_riempimento         = 1;
            step.giro_riempimento                            = delicato_energico == DELICATO ? 8 : 26;
            step.pausa_riempimento                           = delicato_energico == DELICATO ? 10 : 5;
            step.velocita_lavaggio                           = delicato_energico == DELICATO ? 25 : 45;
            step.abilitazione_inversione_lavaggio            = 1;
            step.giro_lavaggio                               = delicato_energico == DELICATO ? 8 : 26;
            step.pausa_lavaggio                              = delicato_energico == DELICATO ? 10 : 5;
            step.abilitazione_riscaldamento                  = 1;
            step.temperatura                                 = 30;
            step.riscaldamento_diretto_indiretto             = 0;
            step.abilitazione_controllo_temperatura_continuo = 0;
            step.livello                                     = 8;
            step.abilitazione_ricircolo                      = 0;
            step.abilitazione_acqua_fredda                   = 1;
            step.abilitazione_acqua_calda                    = 0;
            step.abilitazione_acqua_depurata                 = 0;
            step.tempo_attivo_sapone                         = 1;
            break;

        case STEP_RISCIACQUO:
            step.durata                              = 240;
            step.tempo_attivo                        = 1;
            step.abilitazione_moto_fermo_riempimento = 0;
            step.velocita_riempimento                = 25;
            step.abilitazione_inversione_riempimento = 1;
            step.giro_riempimento                    = 8;
            step.pausa_riempimento                   = 10;
            step.velocita_lavaggio                   = 25;
            step.abilitazione_inversione_lavaggio    = 1;
            step.giro_lavaggio                       = 8;
            step.pausa_lavaggio                      = 10;
            step.livello                             = 12;
            step.abilitazione_ricircolo              = 0;
            step.abilitazione_acqua_fredda           = 1;
            step.abilitazione_acqua_depurata         = 0;
            step.tempo_attivo_sapone                 = 1;
            break;

        case STEP_SROTOLAMENTO:
            step.durata            = 45;
            step.velocita_lavaggio = 45;
            step.giro_lavaggio     = 5;
            step.pausa_lavaggio    = 5;
            break;

        case STEP_SCARICO:
            step.durata                           = 30;
            step.abilitazione_moto_fermo          = delicato_energico == DELICATO ? 0 : 1;
            step.velocita_lavaggio                = delicato_energico == DELICATO ? 25 : 45;
            step.abilitazione_inversione_lavaggio = 1;
            step.giro_lavaggio                    = delicato_energico == DELICATO ? 8 : 10;
            step.pausa_lavaggio                   = delicato_energico == DELICATO ? 10 : 5;
            step.abilitazione_recupero            = 0;
            break;

        case STEP_CENTRIFUGA:
            step.durata                    = 60;
            step.tempo_preparazione        = 20;
            step.velocita_preparazione     = 60;
            step.tempo_scarico             = 0;
            step.abilitazione_recupero     = 0;
            step.numero_rampe              = delicato_energico == DELICATO ? 1 : 3;
            step.velocita_centrifuga_1     = 250;
            step.tempo_rampa_1             = 40;
            step.tempo_attesa_centrifuga_1 = 30;
            step.velocita_centrifuga_2     = 400;
            step.tempo_rampa_2             = 50;
            step.tempo_attesa_centrifuga_2 = 30;
            step.velocita_centrifuga_3     = delicato_energico == DELICATO ? 500 : 600;
            step.tempo_rampa_3             = 60;
            step.tempo_frenata             = 75;
            step.abilitazione_recupero     = 0;
            break;

        case STEP_ATTESA:
            step.durata                                      = 0;
            step.tempo_attivo                                = 0;
            step.tempo_avviso_attesa_on                      = 0;
            step.tempo_avviso_attesa_off                     = 0;
            step.abilitazione_moto_fermo_riempimento         = 0;
            step.velocita_lavaggio                           = delicato_energico == DELICATO ? 25 : 45;
            step.abilitazione_inversione_lavaggio            = 1;
            step.giro_lavaggio                               = delicato_energico == DELICATO ? 8 : 10;
            step.pausa_lavaggio                              = delicato_energico == DELICATO ? 10 : 5;
            step.abilitazione_riscaldamento                  = 0;
            step.temperatura                                 = 0;
            step.riscaldamento_diretto_indiretto             = 0;
            step.abilitazione_controllo_temperatura_continuo = 0;
            break;
    }
    return step;
}

int pack_step(uint8_t *buffer, const parametri_step_t *step, int num) {
    int     i      = 0;
    uint8_t bitmap = 0;

    PACK_BYTE(buffer, num, i);
    PACK_BYTE(buffer, step->tipo, i);

    switch (step->tipo) {
        case STEP_AMMOLLO:
        case STEP_PRELAVAGGIO:
        case STEP_LAVAGGIO:
            PACK_SHORT_BE(buffer, step->durata, i);
            PACK_BYTE(buffer, step->tempo_attivo, i);
            PACK_BYTE(buffer, step->velocita_riempimento, i);
            PACK_SHORT_BE(buffer, step->giro_riempimento, i);
            PACK_SHORT_BE(buffer, step->pausa_riempimento, i);
            PACK_BYTE(buffer, step->velocita_lavaggio, i);
            PACK_SHORT_BE(buffer, step->giro_lavaggio, i);
            PACK_SHORT_BE(buffer, step->pausa_lavaggio, i);

            PACK_BYTE(buffer, step->temperatura, i);
            PACK_BYTE(buffer, step->riscaldamento_diretto_indiretto, i);
            PACK_SHORT_BE(buffer, step->livello, i);

            PACK_BYTE(buffer, step->abilitazione_ricircolo, i);
            PACK_BYTE(buffer, step->tempo_attivo_sapone, i);
            PACK_BYTE(buffer, step->tempo_sapone_1, i);
            PACK_BYTE(buffer, step->tempo_sapone_2, i);
            PACK_BYTE(buffer, step->tempo_sapone_3, i);
            PACK_BYTE(buffer, step->tempo_sapone_4, i);
            PACK_BYTE(buffer, step->tempo_sapone_5, i);
            PACK_BYTE(buffer, step->tempo_sapone_6, i);
            PACK_BYTE(buffer, step->tempo_sapone_7, i);
            PACK_BYTE(buffer, step->tempo_sapone_8, i);
            PACK_BYTE(buffer, step->tempo_sapone_9, i);
            PACK_BYTE(buffer, step->tempo_sapone_10, i);

            PACK_SHORT_BE(buffer, step->ritardo_sapone_1, i);
            PACK_SHORT_BE(buffer, step->ritardo_sapone_2, i);
            PACK_SHORT_BE(buffer, step->ritardo_sapone_3, i);
            PACK_SHORT_BE(buffer, step->ritardo_sapone_4, i);
            PACK_SHORT_BE(buffer, step->ritardo_sapone_5, i);
            PACK_SHORT_BE(buffer, step->ritardo_sapone_6, i);
            PACK_SHORT_BE(buffer, step->ritardo_sapone_7, i);
            PACK_SHORT_BE(buffer, step->ritardo_sapone_8, i);
            PACK_SHORT_BE(buffer, step->ritardo_sapone_9, i);
            PACK_SHORT_BE(buffer, step->ritardo_sapone_10, i);

            bitmap = 0;
            bitmap |= (step->abilitazione_moto_fermo_riempimento > 0);
            bitmap |= (step->abilitazione_inversione_riempimento > 0) << 1;
            bitmap |= (step->abilitazione_inversione_lavaggio > 0) << 2;
            bitmap |= (step->abilitazione_riscaldamento > 0) << 3;
            bitmap |= (step->abilitazione_controllo_temperatura_continuo > 0) << 4;
            bitmap |= (step->abilitazione_acqua_fredda > 0) << 5;
            bitmap |= (step->abilitazione_acqua_calda > 0) << 6;
            bitmap |= (step->abilitazione_acqua_depurata > 0) << 7;
            PACK_BYTE(buffer, bitmap, i);

            break;

        case STEP_RISCIACQUO:
            PACK_SHORT_BE(buffer, step->durata, i);
            PACK_BYTE(buffer, step->tempo_attivo, i);
            PACK_BYTE(buffer, step->velocita_riempimento, i);
            PACK_SHORT_BE(buffer, step->giro_riempimento, i);
            PACK_SHORT_BE(buffer, step->pausa_riempimento, i);
            PACK_BYTE(buffer, step->velocita_lavaggio, i);
            PACK_SHORT_BE(buffer, step->giro_lavaggio, i);
            PACK_SHORT_BE(buffer, step->pausa_lavaggio, i);

            PACK_SHORT_BE(buffer, step->livello, i);

            PACK_BYTE(buffer, step->abilitazione_ricircolo, i);
            PACK_BYTE(buffer, step->tempo_attivo_sapone, i);
            PACK_BYTE(buffer, step->tempo_sapone_1, i);
            PACK_BYTE(buffer, step->tempo_sapone_2, i);
            PACK_BYTE(buffer, step->tempo_sapone_3, i);
            PACK_BYTE(buffer, step->tempo_sapone_4, i);
            PACK_BYTE(buffer, step->tempo_sapone_5, i);
            PACK_BYTE(buffer, step->tempo_sapone_6, i);
            PACK_BYTE(buffer, step->tempo_sapone_7, i);
            PACK_BYTE(buffer, step->tempo_sapone_8, i);
            PACK_BYTE(buffer, step->tempo_sapone_9, i);
            PACK_BYTE(buffer, step->tempo_sapone_10, i);

            PACK_SHORT_BE(buffer, step->ritardo_sapone_1, i);
            PACK_SHORT_BE(buffer, step->ritardo_sapone_2, i);
            PACK_SHORT_BE(buffer, step->ritardo_sapone_3, i);
            PACK_SHORT_BE(buffer, step->ritardo_sapone_4, i);
            PACK_SHORT_BE(buffer, step->ritardo_sapone_5, i);
            PACK_SHORT_BE(buffer, step->ritardo_sapone_6, i);
            PACK_SHORT_BE(buffer, step->ritardo_sapone_7, i);
            PACK_SHORT_BE(buffer, step->ritardo_sapone_8, i);
            PACK_SHORT_BE(buffer, step->ritardo_sapone_9, i);
            PACK_SHORT_BE(buffer, step->ritardo_sapone_10, i);

            bitmap = 0;
            bitmap |= (step->abilitazione_moto_fermo_riempimento > 0);
            bitmap |= (step->abilitazione_inversione_riempimento > 0) << 1;
            bitmap |= (step->abilitazione_inversione_lavaggio > 0) << 2;
            bitmap |= (step->abilitazione_riscaldamento > 0) << 3;
            bitmap |= (step->abilitazione_controllo_temperatura_continuo > 0) << 4;
            bitmap |= (step->abilitazione_acqua_fredda > 0) << 5;
            bitmap |= (step->abilitazione_acqua_calda > 0) << 6;
            bitmap |= (step->abilitazione_acqua_depurata > 0) << 7;
            PACK_BYTE(buffer, bitmap, i);
            break;

        case STEP_SCARICO:
            PACK_SHORT_BE(buffer, step->durata, i);
            PACK_BYTE(buffer, step->abilitazione_moto_fermo, i);
            PACK_BYTE(buffer, step->velocita_lavaggio, i);
            PACK_BYTE(buffer, step->abilitazione_inversione_lavaggio, i);
            PACK_SHORT_BE(buffer, step->giro_lavaggio, i);
            PACK_SHORT_BE(buffer, step->pausa_lavaggio, i);
            PACK_BYTE(buffer, step->abilitazione_recupero, i);
            break;

        case STEP_CENTRIFUGA:
            PACK_SHORT_BE(buffer, step->durata, i);
            PACK_SHORT_BE(buffer, step->tempo_preparazione, i);
            PACK_BYTE(buffer, step->velocita_preparazione, i);
            PACK_BYTE(buffer, step->tempo_scarico, i);
            PACK_BYTE(buffer, step->abilitazione_recupero, i);

            PACK_BYTE(buffer, step->numero_rampe, i);
            PACK_SHORT_BE(buffer, step->velocita_centrifuga_1, i);
            PACK_SHORT_BE(buffer, step->tempo_rampa_1, i);
            PACK_BYTE(buffer, step->tempo_attesa_centrifuga_1, i);
            PACK_SHORT_BE(buffer, step->velocita_centrifuga_2, i);
            PACK_SHORT_BE(buffer, step->tempo_rampa_2, i);
            PACK_BYTE(buffer, step->tempo_attesa_centrifuga_2, i);
            PACK_SHORT_BE(buffer, step->velocita_centrifuga_3, i);
            PACK_SHORT_BE(buffer, step->tempo_rampa_3, i);
            PACK_SHORT_BE(buffer, step->tempo_frenata, i);
            break;

        case STEP_SROTOLAMENTO:
            PACK_SHORT_BE(buffer, step->durata, i);
            PACK_BYTE(buffer, step->velocita_lavaggio, i);
            PACK_SHORT_BE(buffer, step->giro_lavaggio, i);
            PACK_SHORT_BE(buffer, step->pausa_lavaggio, i);
            break;

        case STEP_ATTESA:
            PACK_SHORT_BE(buffer, step->durata, i);
            PACK_BYTE(buffer, step->tempo_attesa, i);
            PACK_BYTE(buffer, step->tempo_avviso_attesa_on, i);
            PACK_BYTE(buffer, step->tempo_avviso_attesa_off, i);
            PACK_BYTE(buffer, step->abilitazione_moto_fermo, i);
            PACK_BYTE(buffer, step->velocita_lavaggio, i);
            PACK_BYTE(buffer, step->abilitazione_inversione_lavaggio, i);
            PACK_SHORT_BE(buffer, step->giro_lavaggio, i);
            PACK_SHORT_BE(buffer, step->pausa_lavaggio, i);
            PACK_BYTE(buffer, step->abilitazione_riscaldamento, i);
            PACK_BYTE(buffer, step->temperatura, i);
            PACK_BYTE(buffer, step->abilitazione_controllo_temperatura_continuo, i);
            break;
    }

    return i;
}

static int serialize_step(uint8_t *buffer, parametri_step_t *s) {
    int i = 0;
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tipo);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->durata);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_attivo);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->velocita_riempimento);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->giro_riempimento);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->pausa_riempimento);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->giro_lavaggio);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->pausa_lavaggio);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->temperatura);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->livello);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_attivo_sapone);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->velocita_lavaggio);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_avviso_attesa_on);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_avviso_attesa_off);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->velocita_preparazione);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_preparazione);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_scarico);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->numero_rampe);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->velocita_centrifuga_1);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->velocita_centrifuga_2);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->velocita_centrifuga_3);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_rampa_1);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_rampa_2);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_rampa_3);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_attesa_centrifuga_1);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_attesa_centrifuga_2);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_frenata);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_sapone_1);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_sapone_2);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_sapone_3);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_sapone_4);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_sapone_5);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_sapone_6);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_sapone_7);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_sapone_8);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_sapone_9);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_sapone_10);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->ritardo_sapone_1);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->ritardo_sapone_2);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->ritardo_sapone_3);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->ritardo_sapone_4);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->ritardo_sapone_5);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->ritardo_sapone_6);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->ritardo_sapone_7);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->ritardo_sapone_8);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->ritardo_sapone_9);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->ritardo_sapone_10);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->tempo_attesa);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->riscaldamento_diretto_indiretto);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->abilitazione_controllo_temperatura_continuo);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->abilitazione_riscaldamento);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->abilitazione_inversione_lavaggio);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->abilitazione_inversione_riempimento);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->abilitazione_moto_fermo_riempimento);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->abilitazione_moto_fermo);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->abilitazione_ricircolo);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->abilitazione_acqua_fredda);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->abilitazione_acqua_calda);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->abilitazione_acqua_depurata);
    i += serialize_uint16_be(&buffer[i], (uint16_t)s->abilitazione_recupero);

    assert(i <= STEP_SIZE);
    return STEP_SIZE;     // Allow for some margin
}


static int deserialize_step(parametri_step_t *s, uint8_t *buffer) {
    int i = 0;

    i += UNPACK_UINT16_BE(s->tipo, &buffer[i]);
    i += UNPACK_UINT16_BE(s->durata, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_attivo, &buffer[i]);
    i += UNPACK_UINT16_BE(s->velocita_riempimento, &buffer[i]);
    i += UNPACK_UINT16_BE(s->giro_riempimento, &buffer[i]);
    i += UNPACK_UINT16_BE(s->pausa_riempimento, &buffer[i]);
    i += UNPACK_UINT16_BE(s->giro_lavaggio, &buffer[i]);
    i += UNPACK_UINT16_BE(s->pausa_lavaggio, &buffer[i]);
    i += UNPACK_UINT16_BE(s->temperatura, &buffer[i]);
    i += UNPACK_UINT16_BE(s->livello, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_attivo_sapone, &buffer[i]);
    i += UNPACK_UINT16_BE(s->velocita_lavaggio, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_avviso_attesa_on, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_avviso_attesa_off, &buffer[i]);
    i += UNPACK_UINT16_BE(s->velocita_preparazione, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_preparazione, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_scarico, &buffer[i]);
    i += UNPACK_UINT16_BE(s->numero_rampe, &buffer[i]);
    i += UNPACK_UINT16_BE(s->velocita_centrifuga_1, &buffer[i]);
    i += UNPACK_UINT16_BE(s->velocita_centrifuga_2, &buffer[i]);
    i += UNPACK_UINT16_BE(s->velocita_centrifuga_3, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_rampa_1, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_rampa_2, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_rampa_3, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_attesa_centrifuga_1, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_attesa_centrifuga_2, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_frenata, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_sapone_1, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_sapone_2, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_sapone_3, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_sapone_4, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_sapone_5, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_sapone_6, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_sapone_7, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_sapone_8, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_sapone_9, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_sapone_10, &buffer[i]);
    i += UNPACK_UINT16_BE(s->ritardo_sapone_1, &buffer[i]);
    i += UNPACK_UINT16_BE(s->ritardo_sapone_2, &buffer[i]);
    i += UNPACK_UINT16_BE(s->ritardo_sapone_3, &buffer[i]);
    i += UNPACK_UINT16_BE(s->ritardo_sapone_4, &buffer[i]);
    i += UNPACK_UINT16_BE(s->ritardo_sapone_5, &buffer[i]);
    i += UNPACK_UINT16_BE(s->ritardo_sapone_6, &buffer[i]);
    i += UNPACK_UINT16_BE(s->ritardo_sapone_7, &buffer[i]);
    i += UNPACK_UINT16_BE(s->ritardo_sapone_8, &buffer[i]);
    i += UNPACK_UINT16_BE(s->ritardo_sapone_9, &buffer[i]);
    i += UNPACK_UINT16_BE(s->ritardo_sapone_10, &buffer[i]);
    i += UNPACK_UINT16_BE(s->tempo_attesa, &buffer[i]);
    i += UNPACK_UINT16_BE(s->riscaldamento_diretto_indiretto, &buffer[i]);
    i += UNPACK_UINT16_BE(s->abilitazione_controllo_temperatura_continuo, &buffer[i]);
    i += UNPACK_UINT16_BE(s->abilitazione_riscaldamento, &buffer[i]);
    i += UNPACK_UINT16_BE(s->abilitazione_inversione_lavaggio, &buffer[i]);
    i += UNPACK_UINT16_BE(s->abilitazione_inversione_riempimento, &buffer[i]);
    i += UNPACK_UINT16_BE(s->abilitazione_moto_fermo_riempimento, &buffer[i]);
    i += UNPACK_UINT16_BE(s->abilitazione_moto_fermo, &buffer[i]);
    i += UNPACK_UINT16_BE(s->abilitazione_ricircolo, &buffer[i]);
    i += UNPACK_UINT16_BE(s->abilitazione_acqua_fredda, &buffer[i]);
    i += UNPACK_UINT16_BE(s->abilitazione_acqua_calda, &buffer[i]);
    i += UNPACK_UINT16_BE(s->abilitazione_acqua_depurata, &buffer[i]);
    i += UNPACK_UINT16_BE(s->abilitazione_recupero, &buffer[i]);

    assert(i <= STEP_SIZE);
    return STEP_SIZE;     // Allow for some margin
}


void program_deserialize_preview(programma_preview_t *p, uint8_t *buffer, uint16_t lingua) {
    size_t i = lingua * STRING_NAME_SIZE;
    memcpy(p->name, &buffer[i], STRING_NAME_SIZE);

    i = MAX_LINGUE * STRING_NAME_SIZE;
    uint32_t prezzo;
    i += deserialize_uint32_be(&prezzo, &buffer[i]);
    p->prezzo = prezzo;
    i += UNPACK_UINT16_BE(p->tipo, &buffer[i]);
}


size_t deserialize_program(programma_lavatrice_t *p, uint8_t *buffer) {
    size_t   i = 0;
    uint32_t prezzo;

    for (int j = 0; j < MAX_LINGUE; j++) {
        memcpy(p->nomi[j], &buffer[i], STRING_NAME_SIZE);
        i += STRING_NAME_SIZE;
    }

    i += deserialize_uint32_be(&prezzo, &buffer[i]);
    p->prezzo = ((float)prezzo);
    i += UNPACK_UINT16_BE(p->tipo, &buffer[i]);
    i += UNPACK_UINT16_BE(p->num_steps, &buffer[i]);

    for (size_t j = 0; j < p->num_steps; j++)
        i += deserialize_step(&p->steps[j], &buffer[i]);

    assert(i == PROGRAM_SIZE(p->num_steps));
    return i;
}


size_t serialize_program(uint8_t *buffer, programma_lavatrice_t *p) {
    size_t i = 0;

    for (int j = 0; j < MAX_LINGUE; j++) {
        memcpy(&buffer[i], p->nomi[j], STRING_NAME_SIZE);
        i += STRING_NAME_SIZE;
    }

    i += serialize_uint32_be(&buffer[i], (uint32_t)p->prezzo);
    i += serialize_uint16_be(&buffer[i], p->tipo);
    i += serialize_uint16_be(&buffer[i], p->num_steps);

    for (size_t j = 0; j < p->num_steps; j++) {
        i += serialize_step(&buffer[i], &p->steps[j]);
    }

    assert(i == PROGRAM_SIZE(p->num_steps));
    return i;
}


size_t program_serialize_empty(uint8_t *buffer, uint16_t num) {
    size_t i = 0;
    name_t names[MAX_LINGUE];
    init_names(names, num);

    for (int j = 0; j < MAX_LINGUE; j++) {
        memcpy(&buffer[i], names[j], STRING_NAME_SIZE);
        i += STRING_NAME_SIZE;
    }

    i += serialize_uint32_be(&buffer[i], (uint32_t)0);
    i += serialize_uint16_be(&buffer[i], 0);
    i += serialize_uint16_be(&buffer[i], 0);

    assert(i == PROGRAM_SIZE(0));
    return i;
}


static void init_names(name_t *names, uint16_t num) {
    const char *nuovo_programma[MAX_LINGUE] = {"Nuovo programma", "New program", "New program", "New program",
                                               "New program",     "New program", "New program", "New program",
                                               "New program",     "New program"};

    for (int i = 0; i < MAX_LINGUE; i++) {
        snprintf(names[i], sizeof(names[i]), "%s %i", nuovo_programma[i], num + 1);
    }
}