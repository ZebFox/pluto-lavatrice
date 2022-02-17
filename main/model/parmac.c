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


void parmac_init(model_t *pmodel, int reset) {
    size_t    i = 0;
    parmac_t *p = &pmodel->prog.parmac;

    parameters[i++] = PARAMETER(&p->lingua, 0, 1, 0, FINT(PARMAC_DESCRIPTIONS_LINGUA), AL_USER);
    // parameters[i++] = PARAMETER(&p->lingua_max_bandiera, 0, 1, 1, FINT(PARMAC_DESCRIPTIONS_LINGUA), AL_USER);

#if 0
    parameter_data_t tmp[NUMP] = {
        // clang-format off
        {.t = unsigned_int, .d = {.uint = {0, NUM_LINGUE - 1, 0, &parmac->lingua}}, .display = {.string_value = (const char ***)lingue}, .lvl = BIT_UTENTE, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, NUM_LINGUE - 1, 1, &parmac->lingua_max_bandiera}}, .display = {.string_value = (const char ***)lingue}, .lvl = BIT_TECNICO, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 5, 1, &parmac->logo}}, .display = {.string_value = (const char ***)stringhe_loghi}, .lvl = BIT_TECNICO, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 255, 0, &parmac->codice_nodo_macchina}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 255, 255, &parmac->modello_macchina}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 255, 255, &parmac->submodello_macchina}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 3, 0, &parmac->livello_accesso}}, .display = {.string_value=(const char ***)stringhe_accesso}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 3, 0, &parmac->visualizzazione_stop}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 3, 0, &parmac->visualizzazione_start}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {1, 50, 20, &parmac->numero_massimo_programmi}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {1, 50, 20, &parmac->numero_massimo_programmi_utente}}, .lvl = BIT_UTENTE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 100, 0, &parmac->visualizzazione_kg}}, .display={.format=formato_kg}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 0xFFFF, 0, &parmac->visualizzazione_prezzo_macchina}}, .display={.special_format=fmt_price}, .lvl = BIT_UTENTE, .runtime = {.userdata = PRICE_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->visualizzazione_help_1}}, .display={.string_value=(const char ***)stringhe_abilitazione}, .lvl = BIT_DISTRIBUTORE, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->visualizzazione_help_2}}, .display={.string_value=(const char ***)stringhe_abilitazione}, .lvl = BIT_DISTRIBUTORE, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->visualizzazione_help_3}}, .display={.string_value=(const char ***)stringhe_abilitazione}, .lvl = BIT_DISTRIBUTORE, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->visualizzazione_help_4}}, .display={.string_value=(const char ***)stringhe_abilitazione}, .lvl = BIT_DISTRIBUTORE, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->visualizzazione_help_5}}, .display={.string_value=(const char ***)stringhe_abilitazione}, .lvl = BIT_DISTRIBUTORE, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->visualizzazione_opl}}, .display={.string_value=(const char ***)stringhe_abilitazione}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->visualizzazione_menu}}, .display={.string_value=(const char ***)stringhe_abilitazione}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->visualizzazione_menu_saponi}},  .display={.string_value=(const char ***)stringhe_abilitazione}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 3, 0, &parmac->visualizzazione_tot_cicli}}, .display = {.string_value = (const char ***)stringhe_totale_cicli}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->abilitazione_lavaggio_programmato}},  .display={.string_value=(const char ***)stringhe_abilitazione}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 8, 0, &parmac->tipo_gettoniera}}, .display={.string_value=(const char ***)stringhe_tipo_gettoniera}, .lvl = BIT_UTENTE, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {1, 0xFFFF, 10, &parmac->valore_impulso}}, .display={.special_format=fmt_price}, .lvl = BIT_UTENTE, .runtime = {.userdata = PRICE_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {1, 0xFFFF, 500, &parmac->valore_prezzo_unico}}, .display={.special_format=fmt_price}, .lvl = BIT_UTENTE, .runtime = {.userdata = PRICE_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->prezzo_unico}}, .display = {.string_value=(const char ***)stringhe_abilitazione}, .lvl = BIT_TECNICO, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {1, 6, 4, &parmac->cifre_prezzo}}, .lvl = BIT_TECNICO, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {.min = 0, .pmax = &parmac->cifre_prezzo, .def=2, .var=&parmac->cifre_decimali_prezzo}}, .lvl = BIT_TECNICO, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 4, 0, &parmac->modo_vis_prezzo}}, .display={.string_value=(const char ***)stringhe_tipo_pagamento}, .lvl = BIT_TECNICO, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 2, 0, &parmac->richiesta_pagamento}}, .display={.string_value=(const char ***)stringhe_richiesta_pagamento}, .lvl = BIT_TECNICO, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 3, 0, &parmac->abilitazione_sblocco_get}}, .display={.string_value=(const char ***)stringhe_abilitazione}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 10, 1, &parmac->secondi_pausa}}, .display = {.format = (const char **)secondi}, .lvl = BIT_UTENTE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 10, 3, &parmac->secondi_stop}}, .display = {.format = (const char **)secondi}, .lvl = BIT_UTENTE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {3, 60, 20, &parmac->tempo_out_pagine}}, .display = {.format = (const char **)secondi}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {1, 100, 30, &parmac->tempo_allarme_livello}}, .display = {.format = (const char **)stringhe_minuti}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {1, 100, 45, &parmac->tempo_allarme_temperatura}}, .display = {.format = (const char **)stringhe_minuti}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {1, 100, 5, &parmac->tempo_allarme_scarico}}, .display = {.format = (const char **)stringhe_minuti}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 240, 15, &parmac->tempo_ritardo_micro_oblo}}, .display = {.special_format = fmt_dec_sec}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 240, 10, &parmac->tempo_precarica}}, .display = {.special_format = fmt_min_sec}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = MIN_SEC_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 240, 10, &parmac->tempo_h2o_pulizia_saponi}}, .display = {.special_format = fmt_min_sec}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = MIN_SEC_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 240, 5, &parmac->tempo_tasto_carico_saponi}}, .display = {.special_format = fmt_min_sec}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = MIN_SEC_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {1, 240, 15, &parmac->tempo_scarico_servizio}}, .display = {.special_format = fmt_min_sec}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = MIN_SEC_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {1, 240, 24, &parmac->tempo_colpo_aperto_scarico}}, .display = {.special_format = fmt_dec_sec}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {1, 240, 10, &parmac->tempo_minimo_scarico}}, .display = {.special_format = fmt_min_sec}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = MIN_SEC_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {1, 250, 45, &parmac->tempo_minimo_frenata}}, .display = {.special_format = fmt_min_sec}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = MIN_SEC_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 10000, 0, &parmac->diametro_cesto}}, .display = {.format = (const char **)formato_cm}, .lvl = BIT_DISTRIBUTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 10000, 0, &parmac->profondita_cesto}}, .display = {.format = (const char **)formato_cm}, .lvl = BIT_DISTRIBUTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1000, 0, &parmac->altezza_trappola}}, .display = {.format = (const char **)formato_cm}, .lvl = BIT_DISTRIBUTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 1, &parmac->abilitazione_espansione_io}}, .display={.string_value=(const char ***)stringhe_abilitazione}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {3, 10, 5, &parmac->numero_saponi_utilizzabili}}, .lvl = BIT_TECNICO, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 10, 0, &parmac->esclusione_sapone}}, .display = {.string_value=(const char ***)esclusione_saponi}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 2, 2, &parmac->abilitazione_macchina_libera}}, .display = {.string_value=(const char ***)stringhe_macchina_libera},  .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->f_macchina_libera}}, .display = {.string_value=(const char***)stringhe_nc_na}, .lvl = BIT_UTENTE, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->tipo_out_aux_1}}, .display = {.string_value=(const char***) &stringhe_ausiliari[0]}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 1, &parmac->tipo_out_aux_2}}, .display = {.string_value=(const char***) &stringhe_ausiliari[2]}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 1, &parmac->tipo_out_aux_3}}, .display = {.string_value=(const char***) &stringhe_ausiliari[4]}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 1, &parmac->tipo_out_aux_4}}, .display = {.string_value=(const char***) &stringhe_ausiliari[6]}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->f_scarico_recupero}}, .display = {.string_value=(const char ***)stringhe_aperto_chiuso}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {10, 190, 100, &parmac->percentuale_livello_carico_ridotto}}, .display = {.format = (const char **)formato_percentuale}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {10, 190, 100, &parmac->percentuale_sapone_carico_ridotto}}, .display = {.format = (const char **)formato_percentuale}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 1, &parmac->autoavvio}}, .lvl = BIT_UTENTE, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->f_proximity}}, .display = {.string_value = (const char ***)stringhe_abilitazione}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {1, 12, 6, &parmac->numero_raggi}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 200, 111, &parmac->correzione_contagiri}}, .display = {.format = (const char **)formato_percentuale}, .lvl = BIT_DISTRIBUTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 3, 2, &parmac->abilitazione_accelerometro}}, .display = {.string_value=(const char ***)stringhe_abilitazione_accelerometro}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 5, 3, &parmac->scala_accelerometro}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 511, 100, &parmac->soglia_x_accelerometro}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 511, 90, &parmac->soglia_y_accelerometro}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 511, 110, &parmac->soglia_z_accelerometro}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 511, 155, &parmac->soglia_x_accelerometro_h}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 511, 135, &parmac->soglia_y_accelerometro_h}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 511, 110, &parmac->soglia_z_accelerometro_h}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1000, 200, &parmac->giri_accelerometro}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1000, 300, &parmac->giri_accelerometro_2}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 100, 50, &parmac->delta_val_accelerometro}}, .display = {.format = (const char **)formato_percentuale}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 60, 20, &parmac->tempo_attesa_accelerometro}}, .display = {.special_format = fmt_dec_sec}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1000, 20, &parmac->tempo_scarico_accelerometro}}, .display = {.special_format = fmt_min_sec}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = MIN_SEC_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 100, 90, &parmac->temperatura_massima}}, .display = {.format = (const char **)formato_c}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 60, 2, &parmac->isteresi_temperatura}}, .display = {.format = (const char **)formato_c}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 99, 95, &parmac->temperatura_sicurezza}}, .display = {.format = (const char **)formato_c}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 60, 45, &parmac->temperatura_termodegradazione}}, .display = {.format = (const char **)formato_c}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 2, 0, &parmac->tipo_livello}}, .display = {.string_value = (const char***)stringhe_tipo_livello}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {1, 60, 3, &parmac->tempo_isteresi_livello}}, .display = {.format = (const char **)secondi}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {2, 100, 48, &parmac->centimetri_max_livello}}, .display = {.format = (const char **)formato_cm}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {15, 10000, 50, &parmac->livello_sfioro}}, .display = {.format = (const char **)formato_cm}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 30, 1, &parmac->centimetri_minimo_scarico}}, .display = {.format = (const char **)formato_cm}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {2, 30, 4, &parmac->centimetri_minimo_riscaldo}}, .display = {.format = (const char **)formato_cm}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {15, 10000, 50, &parmac->litri_massimi}}, .display = {.format = (const char **)formato_lt}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {1, 1000, 20, &parmac->litri_minimi_riscaldo}}, .display = {.format = (const char **)formato_lt}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 10000, 328, &parmac->impulsi_litro}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->tipo_inverter}}, .display = {.string_value = (const char ***)stringhe_modello_inverter}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {1, 100, 36, &parmac->velocita_servizio}}, .display = {.format = (const char **)formato_rpm}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {.min  = 0, .max = 150, .def  = 20, .var  = &parmac->velocita_minima_lavaggio}}, .display = {.format = (const char **)formato_rpm}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {.min = 0, .max=150, .def = 60, .var  = &parmac->velocita_massima_lavaggio}}, .display = {.format = (const char **)formato_rpm}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 9, 3, &parmac->abilitazione_preparazione_rotazione}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {1, 200, 20, &parmac->tempo_marcia_preparazione_rotazione}}, .display = {.special_format = fmt_min_sec}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = MIN_SEC_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {2, 240, 5, &parmac->tempo_sosta_preparazione_rotazione}}, .display = {.special_format = fmt_min_sec}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = MIN_SEC_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {.min  = 1, .pmax = &parmac->velocita_massima_preparazione, .def  = 20, .var  = &parmac->velocita_minima_preparazione}}, .display = {.format = (const char **)formato_rpm}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {.pmin = &parmac->velocita_minima_preparazione, .max  = 200, .def  = 50, .var  = &parmac->velocita_massima_preparazione}}, .display = {.format = (const char **)formato_rpm}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {.min  = 0, .max = 1200, .def  = 1, .var  = &parmac->velocita_minima_centrifuga_1}}, .display = {.format = (const char **)formato_rpm}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {.min = 1, .max  = 1200, .def  = 1000, .var  = &parmac->velocita_massima_centrifuga_1}}, .display = {.format = (const char **)formato_rpm}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {.min  = 0, .max = 1200, .def  = 1, .var  = &parmac->velocita_minima_centrifuga_2}}, .display = {.format = (const char **)formato_rpm}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {.min = 1, .max  = 1200, .def  = 1000, .var  = &parmac->velocita_massima_centrifuga_2}}, .display = {.format = (const char **)formato_rpm}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {.min  = 0, .max = 1200, .def  = 1, .var  = &parmac->velocita_minima_centrifuga_3}}, .display = {.format = (const char **)formato_rpm}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {.min = 1, .max  = 1200, .def  = 1000, .var  = &parmac->velocita_massima_centrifuga_3}}, .display = {.format = (const char **)formato_rpm}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {.min = 3, .max = 1000, .def = 15, .var = &parmac->tempo_minimo_rampa}}, .display = {.special_format = fmt_min_sec}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = MIN_SEC_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {.min = 3, .max = 1000, .def = 90, .var = &parmac->tempo_massimo_rampa}}, .display = {.special_format = fmt_min_sec}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = MIN_SEC_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {1, 60, 35, &parmac->nro_max_sbilanciamenti}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->abilitazione_min_sec}}, .display = {.string_value=(const char ***)stringhe_abilitazione}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 3, 0, &parmac->tipo_serratura}}, .display = {.string_value=(const char ***)stringhe_tipo_serratura}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {5, 30, 8, &parmac->durata_impulso_serratura}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->inibizione_allarmi}}, .display = {.string_value=(const char ***)stringhe_abilitazione}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->abilitazione_loop_prog}}, .display = {.string_value=(const char ***)stringhe_abilitazione}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = SWITCH_PARAMETER}},
        // Parametri non visualizzati nella lista con gli altri
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->visualizzazione_data_ora}}, .lvl = BIT_NESSUNO},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->visualizzazione_pedante}}, .lvl = BIT_NESSUNO},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->visualizzazione_esclusione_sapone}}, .lvl = BIT_NESSUNO},
        {.t = unsigned_char, .d = {.uch = {0, 7, 7, &parmac->funzioni_rgb[CONDIZIONE_MACCHINA_FERMO]}}, .lvl = BIT_NESSUNO},
        {.t = unsigned_char, .d = {.uch = {0, 7, 1, &parmac->funzioni_rgb[CONDIZIONE_MACCHINA_LAVORO]}}, .lvl = BIT_NESSUNO},
        {.t = unsigned_char, .d = {.uch = {0, 7, 6, &parmac->funzioni_rgb[CONDIZIONE_MACCHINA_PAUSA]}}, .lvl = BIT_NESSUNO},
        {.t = unsigned_char, .d = {.uch = {0, 7, 2, &parmac->funzioni_rgb[CONDIZIONE_MACCHINA_ATTESA]}}, .lvl = BIT_NESSUNO},
        {.t = unsigned_char, .d = {.uch = {0, 7, 3, &parmac->funzioni_rgb[CONDIZIONE_MACCHINA_AVVISO]}}, .lvl = BIT_NESSUNO},
        {.t = unsigned_char, .d = {.uch = {0, 7, 4, &parmac->funzioni_rgb[CONDIZIONE_MACCHINA_ALLARME]}}, .lvl = BIT_NESSUNO},
    };
    // clang-format on
#endif

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