/******************************************************************************/
/*                                                                            */
/*  HSW snc - Casalecchio di Reno (BO) ITALY                                  */
/*  ----------------------------------------                                  */
/*                                                                            */
/*  modulo: parmac.c                                                          */
/*                                                                            */
/*      definizione e gestione parametri macchina                             */
/*                                                                            */
/*  Autore: Maldus (Mattia MALDINI) & Virginia NEGRI & Massimo ZANNA          */
/*                                                                            */
/*  Data  : 19/07/2021      REV  : 00.0                                       */
/*                                                                            */
/*  U.mod.: 02/08/2021      REV  : 01.0                                       */
/*                                                                            */
/******************************************************************************/

#include <assert.h>
#include <stdio.h>
#include "config/parameter_conf.h"
#include "gel/parameter/parameter.h"

#include "model.h"
#include "parmac.h"
#include "descriptions/AUTOGEN_FILE_parmac.h"

#define NUM_PARAMETERS 1

#define AL_USER 0x01
#define AL_TECH 0x02

#define FINT(i) ((parameter_user_data_t){parmac_descriptions[i], formatta, NULL, NULL})

enum {
    LIVELLO_ACCESSO_ESTESI  = 0,
    LIVELLO_ACCESSO_RIDOTTI = 1,
    NUM_LIVELLI_ACCESSO,
};

parameter_handle_t parameters[NUM_PARAMETERS];

static void                formatta(char *string, const void *arg);
static parameter_handle_t *get_actual_parameter(model_t *pmodel, size_t parameter, uint8_t al);
static uint8_t             get_livello_accesso(uint8_t parametri_ridotti);


void parmac_init(model_t *p, int reset) {
    size_t i = 0;

    parameters[i++] =
        PARAMETER(&p->prog.parmac.lingua, 0, 1, 0, FINT(PARMAC_DESCRIPTIONS_LINGUA), AL_USER);

    parameter_check_ranges(parameters, NUM_PARAMETERS);
    if (reset) {
        parameter_reset_to_defaults(parameters, NUM_PARAMETERS);
    }
}

void parmac_operation(model_t *pmodel, size_t parameter, int op, uint8_t al) {
    parameter_operator(get_actual_parameter(pmodel, parameter, al), op);
}

const char *parmac_get_description(model_t *pmodel, size_t parameter, uint8_t al) {
    parameter_user_data_t data = parameter_get_user_data(get_actual_parameter(pmodel, parameter, al));

    return data.descrizione[pmodel->prog.parmac.lingua];
}

void parmac_format_value(model_t *pmodel, char *string, size_t parameter, uint8_t al) {
    parameter_handle_t   *par  = get_actual_parameter(pmodel, parameter, al);
    parameter_user_data_t data = parameter_get_user_data(par);

    data.format(string, par);
}

size_t parmac_get_tot_parameters(uint8_t al) {
    return parameter_get_count(parameters, NUM_PARAMETERS, al);
}


static void formatta(char *string, const void *arg) {
    const parameter_handle_t *par = arg;

    switch (par->type) {
        case PARAMETER_TYPE_UINT8:
            sprintf(string, "%i", *(uint8_t *)par->pointer);
            break;
        case PARAMETER_TYPE_UINT16:
            sprintf(string, "%i", *(uint16_t *)par->pointer);
            break;
        default:
            sprintf(string, "Errore!");
            break;
    }
}


static uint8_t get_livello_accesso(uint8_t parametri_ridotti) {
    if (parametri_ridotti == 1)
        return 0b01;
    else
        return 0b11;
}


static parameter_handle_t *get_actual_parameter(model_t *pmodel, size_t parameter, uint8_t al) {
    return parameter_get_handle(parameters, NUM_PARAMETERS, parameter, get_livello_accesso(al));
}


const char *parmac_commissioning_language_get_description(model_t *pmodel) {
    parameter_user_data_t data = parameter_get_user_data(&parameters[PARMAC_COMMISSIONING_LINGUA]);
    return data.descrizione[pmodel->prog.parmac.lingua];
}



void parmac_commissioning_operation(model_t *pmodel, parmac_commissioning_t parameter, int op) {
    parameter_operator(&parameters[parameter], op);
    // pmodel->lingua_temporanea = pmodel->prog.parmac.lingua;
}