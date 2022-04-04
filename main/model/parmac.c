#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "config/parameter_conf.h"
#include "gel/parameter/parameter.h"
#include "descriptions/parstring.h"
#include "model.h"
#include "parmac.h"
#include "descriptions/AUTOGEN_FILE_pars.h"

#define NUM_PARAMETERS 105

enum {
    LIVELLO_ACCESSO_ESTESI  = 0,
    LIVELLO_ACCESSO_RIDOTTI = 1,
    NUM_LIVELLI_ACCESSO,
};

parameter_handle_t parameters[NUM_PARAMETERS];

static parameter_handle_t *get_actual_parameter(model_t *pmodel, size_t parameter, uint8_t al);


void parmac_init(model_t *pmodel, int reset) {
    size_t              i  = 0;
    parmac_t           *p  = &pmodel->prog.parmac;
    parameter_handle_t *ps = parameters;

    char *fmt_sec  = "%i s";
    char *fmt_min  = "%i min";
    char *fmt_cm   = "%i cm";
    char *fmt_perc = "%i %%";
    char *fmt_lt   = "%i lt";
    char *fmt_C    = "%i C";
    char *fmt_rpm  = "%i rpm";

    // clang-format off
    ps[i++] = PARAMETER(&p->lingua, 0, NUM_LINGUE - 1, 0, FOPT(PARS_DESCRIPTIONS_LINGUA, pars_lingue), BIT_UTENTE);
    ps[i++] = PARAMETER(&p->logo, 0, 5, 0, FOPT(PARS_DESCRIPTIONS_LOGO, pars_loghi), BIT_UTENTE);
    ps[i++] = PARAMETER(&p->livello_accesso, 0, 3, 0, FOPT(PARS_DESCRIPTIONS_LIVELLO_ACCESSO, pars_livello_accesso), BIT_TECNICO);
    ps[i++] = PARAMETER(&p->visualizzazione_stop, 0, 1, 0, FOPT(PARS_DESCRIPTIONS_INTERFACCIA_STOP, pars_visualizzazione), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->visualizzazione_start, 0, 1, 0, FOPT(PARS_DESCRIPTIONS_INTERFACCIA_START, pars_visualizzazione), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->numero_massimo_programmi_utente, 1, MAX_PROGRAMMI, 20, FINT(PARS_DESCRIPTIONS_MAX_PROGRAMMI_UTENTE), BIT_UTENTE);
    ps[i++] = PARAMETER(&p->visualizzazione_menu, 0, 1, 0, FOPT(PARS_DESCRIPTIONS_INTERFACCIA_MENU, pars_abilitazione), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->visualizzazione_menu_saponi, 0, 1, 0, FOPT(PARS_DESCRIPTIONS_INTERFACCIA_SAPONI, pars_abilitazione), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->abilitazione_lavaggio_programmato, 0, 1, 0, FOPT(PARS_DESCRIPTIONS_LAVAGGIO_PROGRAMMATO, pars_abilitazione), BIT_UTENTE);
    ps[i++] = PARAMETER(&p->tipo_gettoniera, 0, 8, 0, FOPT(PARS_DESCRIPTIONS_TIPO_GETTONIERA, pars_gettoniera), BIT_UTENTE);
    ps[i++] = PARAMETER(&p->valore_impulso, 1, 0xFFFF, 100, FINT(PARS_DESCRIPTIONS_VALORE_IMPULSO), BIT_UTENTE);
    ps[i++] = PARAMETER(&p->valore_prezzo_unico, 1, 0xFFFF, 500, FINT(PARS_DESCRIPTIONS_VALORE_PREZZO_UNICO), BIT_TECNICO);
    ps[i++] = PARAMETER(&p->prezzo_unico, 0, 1, 0, FOPT(PARS_DESCRIPTIONS_PREZZO_UNICO, pars_abilitazione), BIT_UTENTE);
    ps[i++] = PARAMETER(&p->cifre_prezzo, 1, 6, 4, FINT(PARS_DESCRIPTIONS_CIFRE_PREZZO), BIT_TECNICO);
    ps[i++] = PARAMETER_DLIMITS(&p->cifre_decimali_prezzo, NULL, &p->cifre_prezzo, 0, 6, 2, FINT(PARS_DESCRIPTIONS_CIFRE_DECIMALI_PREZZO), BIT_TECNICO);
    ps[i++] = PARAMETER(&p->modo_vis_prezzo, 0, 4, 0, FOPT(PARS_DESCRIPTIONS_VISUALIZZAZIONE_PREZZO, pars_tipo_pagamento), BIT_TECNICO);
    ps[i++] = PARAMETER(&p->richiesta_pagamento, 0, 2, 0, FOPT(PARS_DESCRIPTIONS_RICHIESTA_PAGAMENTO, pars_richiesta_pagamento), BIT_TECNICO);
    ps[i++] = PARAMETER(&p->abilitazione_sblocco_get, 0, 1, 0, FOPT(PARS_DESCRIPTIONS_SBLOCCO_GETTONIERA, pars_abilitazione), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->secondi_pausa, 0, 10, 1, FFINT(PARS_DESCRIPTIONS_TEMPO_TASTO_PAUSA, fmt_sec), BIT_UTENTE);
    ps[i++] = PARAMETER(&p->secondi_stop, 0, 10, 3, FFINT(PARS_DESCRIPTIONS_TEMPO_TASTO_STOP, fmt_sec), BIT_UTENTE);
    ps[i++] = PARAMETER(&p->tempo_allarme_livello, 1, 100, 30, FFINT(PARS_DESCRIPTIONS_TEMPO_ALLARME_LIVELLO, fmt_min), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tempo_allarme_temperatura, 1, 100, 45, FFINT(PARS_DESCRIPTIONS_TEMPO_ALLARME_TEMPERATURA, fmt_min), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tempo_allarme_scarico, 1, 100, 45, FFINT(PARS_DESCRIPTIONS_TEMPO_ALLARME_SCARICO, fmt_min), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tempo_ritardo_micro_oblo, 0, 240, 15, FFINT(PARS_DESCRIPTIONS_TEMPO_RITARDO_MICRO_OBLO, fmt_sec), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tempo_precarica, 0, 240, 10, FTIME(PARS_DESCRIPTIONS_TEMPO_PRECARICA), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tempo_h2o_pulizia_saponi, 0, 240, 10, FTIME(PARS_DESCRIPTIONS_TEMPO_PULIZIA_SAPONI), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tempo_tasto_carico_saponi, 0, 240, 5, FTIME(PARS_DESCRIPTIONS_TEMPO_CARICO_SAPONI), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tempo_scarico_servizio, 1, 240, 15, FTIME(PARS_DESCRIPTIONS_TEMPO_SCARICO), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tempo_colpo_aperto_scarico, 1, 240, 24, FTIME(PARS_DESCRIPTIONS_TEMPO_COLPO_SCARICO), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tempo_minimo_scarico, 1, 240, 10, FTIME(PARS_DESCRIPTIONS_TEMPO_MINIMO_SCARICO), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tempo_minimo_frenata, 1, 250, 45, FTIME(PARS_DESCRIPTIONS_TEMPO_MINIMO_FRENATA), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->diametro_cesto, 0, 10000, 0, FFINT(PARS_DESCRIPTIONS_DIAMETRO_CESTO, fmt_cm), BIT_DISTRIBUTORE);
    ps[i++] = PARAMETER(&p->profondita_cesto, 0, 10000, 0, FFINT(PARS_DESCRIPTIONS_PROFONDITA_CESTO, fmt_cm), BIT_DISTRIBUTORE);
    ps[i++] = PARAMETER(&p->altezza_trappola, 0, 1000, 0, FFINT(PARS_DESCRIPTIONS_ALTEZZA_TRAPPOLA, fmt_cm), BIT_DISTRIBUTORE);
    ps[i++] = PARAMETER(&p->abilitazione_espansione_io, 0, 1, 1, FOPT(PARS_DESCRIPTIONS_ESPANSIONE_IO, pars_abilitazione), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->numero_saponi_utilizzabili, 3, 10, 6, FINT(PARS_DESCRIPTIONS_NUMERO_SAPONI), BIT_TECNICO);
    ps[i++] = PARAMETER(&p->esclusione_sapone, 0, 10, 0, FINT(PARS_DESCRIPTIONS_ESCLUSIONE_SAPONE), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->abilitazione_macchina_libera, 0, 2, 2, FOPT(PARS_DESCRIPTIONS_MACCHINA_LIBERA, pars_macchina_libera), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->f_macchina_libera, 0, 1, 1, FOPT(PARS_DESCRIPTIONS_TIPO_MACCHINA_LIBERA, pars_na_nc), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tipo_out_aux_1, 0, 1, 1, FOPT(PARS_DESCRIPTIONS_TIPO_IN_AUX_1, &pars_ausiliari[0]), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tipo_out_aux_2, 0, 1, 1, FOPT(PARS_DESCRIPTIONS_TIPO_OUT_AUX_2, &pars_ausiliari[2]), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tipo_out_aux_3, 0, 1, 1, FOPT(PARS_DESCRIPTIONS_TIPO_OUT_AUX_3, &pars_ausiliari[4]), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tipo_out_aux_4, 0, 1, 1, FOPT(PARS_DESCRIPTIONS_TIPO_OUT_AUX_4, &pars_ausiliari[6]), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->f_scarico_recupero, 0, 1, 0, FOPT(PARS_DESCRIPTIONS_SCARICO_RECUPERO, pars_nc_na), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->percentuale_livello_carico_ridotto, 10, 190, 100, FFINT(PARS_DESCRIPTIONS_PERCENTUALE_LIVELLO_CARICO_RIDOTTO, fmt_perc), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->percentuale_sapone_carico_ridotto, 10, 190, 100, FFINT(PARS_DESCRIPTIONS_PERCENTUALE_SAPONE_CARICO_RIDOTTO, fmt_perc), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->autoavvio, 0, 1, 1, FOPT(PARS_DESCRIPTIONS_AUTOAVVIO, pars_abilitazione), BIT_UTENTE);
    ps[i++] = PARAMETER(&p->f_proximity, 0, 1, 0, FOPT(PARS_DESCRIPTIONS_SENSORE_PROSSIMITA, pars_abilitazione), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->numero_raggi, 1, 12, 6, FINT(PARS_DESCRIPTIONS_NUMERO_RAGGI), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->correzione_contagiri, 0, 200, 111, FFINT(PARS_DESCRIPTIONS_CORREZIONE_CONTAGIRI, fmt_perc), BIT_DISTRIBUTORE);
    ps[i++] = PARAMETER(&p->abilitazione_accelerometro, 0, 3, 2, FOPT(PARS_DESCRIPTIONS_ACCELEROMETRO, pars_accelerometro), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->scala_accelerometro, 0, 5, 3, FOPT(PARS_DESCRIPTIONS_SCALA_ACCELEROMETRO, pars_scala_accelerometro), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->soglia_x_accelerometro, 0, 511, 100, FINT(PARS_DESCRIPTIONS_SOGLIA_X_ACCELEROMETRO), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->soglia_y_accelerometro, 0, 511, 90, FINT(PARS_DESCRIPTIONS_SOGLIA_Y_ACCELEROMETRO), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->soglia_z_accelerometro, 0, 511, 110, FINT(PARS_DESCRIPTIONS_SOGLIA_Z_ACCELEROMETRO), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->soglia_x_accelerometro_h, 0, 511, 155, FINT(PARS_DESCRIPTIONS_SOGLIA_X_ACCELEROMETRO_H), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->soglia_y_accelerometro_h, 0, 511, 135, FINT(PARS_DESCRIPTIONS_SOGLIA_Y_ACCELEROMETRO_H), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->soglia_z_accelerometro_h, 0, 511, 110, FINT(PARS_DESCRIPTIONS_SOGLIA_Z_ACCELEROMETRO_H), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->giri_accelerometro, 0, 1000, 200, FINT(PARS_DESCRIPTIONS_GIRI_ACCELEROMETRO), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->giri_accelerometro_2, 0, 1000, 300, FINT(PARS_DESCRIPTIONS_GIRI_ACCELEROMETRO_2), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->delta_val_accelerometro, 0, 1000, 200, FFINT(PARS_DESCRIPTIONS_DELTA_ACCELEROMETRO, fmt_perc), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tempo_attesa_accelerometro, 0, 60, 20, FFINT(PARS_DESCRIPTIONS_TEMPO_ATTESA_ACCELEROMETRO, fmt_sec), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tempo_scarico_accelerometro, 0, 1000, 20, FTIME(PARS_DESCRIPTIONS_TEMPO_SCARICO_ACCELEROMETRO), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->temperatura_massima, 0, 100, 90, FFINT(PARS_DESCRIPTIONS_TEMPERATURA_MASSIMA, fmt_C), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->isteresi_temperatura, 0, 60, 2, FFINT(PARS_DESCRIPTIONS_ISTERESI_TEMPERATURA, fmt_C), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->temperatura_sicurezza, 0, 99, 95, FFINT(PARS_DESCRIPTIONS_TEMPERATURA_SICUREZZA, fmt_C), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->temperatura_termodegradazione, 0, 60, 45, FFINT(PARS_DESCRIPTIONS_TEMPERATURA_TERMODEGRADAZIONE, fmt_C), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tipo_livello, 0, 2, 0, FOPT(PARS_DESCRIPTIONS_TIPO_LIVELLO, pars_tipo_livello), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tempo_isteresi_livello, 1, 60, 3, FFINT(PARS_DESCRIPTIONS_TEMPO_ISTERESI_LIVELLO, fmt_sec), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->centimetri_max_livello, 2, 100, 48, FFINT(PARS_DESCRIPTIONS_MAX_LIVELLO, fmt_cm), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->livello_sfioro, 15, 10000, 50, FFINT(PARS_DESCRIPTIONS_LIVELLO_SFIORO, fmt_cm), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->centimetri_minimo_scarico, 1, 30, 1, FFINT(PARS_DESCRIPTIONS_LIVELLO_MINIMO_SCARICO, fmt_cm), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->centimetri_minimo_riscaldo, 0, 30, 2, FFINT(PARS_DESCRIPTIONS_LIVELLO_MINIMO_RISCALDAMENTO, fmt_cm), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->litri_massimi, 15, 10000, 50, FFINT(PARS_DESCRIPTIONS_LITRI_MASSIMI, fmt_lt), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->litri_minimi_riscaldo, 1, 1000, 20, FFINT(PARS_DESCRIPTIONS_LITRI_MINIMI_RISCALDAMENTO, fmt_lt), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->impulsi_litro, 0, 10000, 328, FINT(PARS_DESCRIPTIONS_IMPULSI_LITRI), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tipo_inverter, 0, 1, 0, FOPT(PARS_DESCRIPTIONS_TIPO_INVERTER, pars_tipo_inverter), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->velocita_servizio, 0, 100, 36, FFINT(PARS_DESCRIPTIONS_VELOCITA_SERVIZIO, fmt_rpm), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->velocita_minima_lavaggio, 0, 150, 20, FFINT(PARS_DESCRIPTIONS_VELOCITA_MINIMA_LAVAGGIO, fmt_rpm), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->velocita_massima_lavaggio, 0, 150, 60, FFINT(PARS_DESCRIPTIONS_VELOCITA_MASSIMA_LAVAGGIO, fmt_rpm), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->abilitazione_preparazione_rotazione, 0, 9, 3, FINT(PARS_DESCRIPTIONS_PREPARAZIONE_ROTAZIONE), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tempo_marcia_preparazione_rotazione, 1, 200, 20, FTIME(PARS_DESCRIPTIONS_TEMPO_MARCIA_PREPARAZIONE_ROTAZIONE), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tempo_sosta_preparazione_rotazione, 1, 200, 20, FTIME(PARS_DESCRIPTIONS_TEMPO_SOSTA_PREPARAZIONE_ROTAZIONE), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->velocita_minima_preparazione, 1, 200, 20, FFINT(PARS_DESCRIPTIONS_VELOCITA_MINIMA_PREPARAZIONE, fmt_rpm), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->velocita_massima_preparazione, 1, 200, 50, FFINT(PARS_DESCRIPTIONS_VELOCITA_MASSIMA_PREPARAZIONE, fmt_rpm), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->velocita_minima_centrifuga_1, 0, 1200, 1, FFINT(PARS_DESCRIPTIONS_VELOCITA_MINIMA_CENTRIFUGA_1, fmt_rpm), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->velocita_massima_centrifuga_1, 1, 1200, 1000, FFINT(PARS_DESCRIPTIONS_VELOCITA_MASSIMA_CENTRIFUGA_1, fmt_rpm), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->velocita_minima_centrifuga_2, 0, 1200, 1, FFINT(PARS_DESCRIPTIONS_VELOCITA_MINIMA_CENTRIFUGA_2, fmt_rpm), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->velocita_massima_centrifuga_2, 1, 1200, 1000, FFINT(PARS_DESCRIPTIONS_VELOCITA_MASSIMA_CENTRIFUGA_2, fmt_rpm), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->velocita_minima_centrifuga_3, 0, 1200, 1, FFINT(PARS_DESCRIPTIONS_VELOCITA_MINIMA_CENTRIFUGA_3, fmt_rpm), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->velocita_massima_centrifuga_3, 1, 1200, 1000, FFINT(PARS_DESCRIPTIONS_VELOCITA_MASSIMA_CENTRIFUGA_3, fmt_rpm), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tempo_minimo_rampa, 3, 1000, 15, FTIME(PARS_DESCRIPTIONS_TEMPO_MINIMO_RAMPA), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tempo_massimo_rampa, 3, 1000, 90, FTIME(PARS_DESCRIPTIONS_TEMPO_MASSIMO_RAMPA), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->nro_max_sbilanciamenti, 1, 60, 35, FINT(PARS_DESCRIPTIONS_NUMERO_MAX_SBILANCIAMENTI), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->abilitazione_min_sec, 0, 1, 0, FOPT(PARS_DESCRIPTIONS_MIN_SEC, pars_abilitazione), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->tipo_serratura, 0, 3, 0, FOPT(PARS_DESCRIPTIONS_TIPO_SERRATURA, pars_tipo_serratura), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->durata_impulso_serratura, 1, 30, 8, FFINT(PARS_DESCRIPTIONS_DURATA_IMPULSO_SERRATURA, fmt_sec), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->inibizione_allarmi, 0, 1, 0, FOPT(PARS_DESCRIPTIONS_INIBIZIONE_ALLARMI, pars_abilitazione), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->abilitazione_loop_prog, 0, 1, 0, FOPT(PARS_DESCRIPTIONS_RIPETIZIONE_CICLO, pars_abilitazione), BIT_COSTRUTTORE);
    ps[i++] = PARAMETER(&p->funzioni_rgb[CONDIZIONE_MACCHINA_FERMO], 0, 7, 7, FOPT(PARS_DESCRIPTIONS_MACCHINA_FERMA, pars_rgb), BIT_UTENTE);
    ps[i++] = PARAMETER(&p->funzioni_rgb[CONDIZIONE_MACCHINA_LAVORO], 0, 7, 1, FOPT(PARS_DESCRIPTIONS_MACCHINA_LAVORO, pars_rgb), BIT_UTENTE);
    ps[i++] = PARAMETER(&p->funzioni_rgb[CONDIZIONE_MACCHINA_PAUSA], 0, 7, 6, FOPT(PARS_DESCRIPTIONS_MACCHINA_PAUSA, pars_rgb), BIT_UTENTE);
    ps[i++] = PARAMETER(&p->funzioni_rgb[CONDIZIONE_MACCHINA_ATTESA], 0, 7, 2, FOPT(PARS_DESCRIPTIONS_MACCHINA_ATTESA, pars_rgb), BIT_UTENTE);
    ps[i++] = PARAMETER(&p->funzioni_rgb[CONDIZIONE_MACCHINA_AVVISO], 0, 7, 3, FOPT(PARS_DESCRIPTIONS_MACCHINA_AVVISO, pars_rgb), BIT_UTENTE);
    ps[i++] = PARAMETER(&p->funzioni_rgb[CONDIZIONE_MACCHINA_ALLARME], 0, 7, 4, FOPT(PARS_DESCRIPTIONS_MACCHINA_ALLARME, pars_rgb), BIT_UTENTE);

    // clang-format on


#if 0
    parameter_data_t tmp[NUMP] = {
        // clang-format off
        {.t = unsigned_int, .d = {.uint = {0, NUM_LINGUE - 1, 1, &parmac->lingua_max_bandiera}}, .display = {.string_value = (const char ***)lingue}, .lvl = BIT_TECNICO, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 255, 0, &parmac->codice_nodo_macchina}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 255, 255, &parmac->modello_macchina}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 255, 255, &parmac->submodello_macchina}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {1, 50, 20, &parmac->numero_massimo_programmi}}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 100, 0, &parmac->visualizzazione_kg}}, .display={.format=formato_kg}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 0xFFFF, 0, &parmac->visualizzazione_prezzo_macchina}}, .display={.special_format=fmt_price}, .lvl = BIT_UTENTE, .runtime = {.userdata = PRICE_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->visualizzazione_help_1}}, .display={.string_value=(const char ***)stringhe_abilitazione}, .lvl = BIT_DISTRIBUTORE, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->visualizzazione_help_2}}, .display={.string_value=(const char ***)stringhe_abilitazione}, .lvl = BIT_DISTRIBUTORE, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->visualizzazione_help_3}}, .display={.string_value=(const char ***)stringhe_abilitazione}, .lvl = BIT_DISTRIBUTORE, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->visualizzazione_help_4}}, .display={.string_value=(const char ***)stringhe_abilitazione}, .lvl = BIT_DISTRIBUTORE, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->visualizzazione_help_5}}, .display={.string_value=(const char ***)stringhe_abilitazione}, .lvl = BIT_DISTRIBUTORE, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->visualizzazione_opl}}, .display={.string_value=(const char ***)stringhe_abilitazione}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = SWITCH_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {0, 3, 0, &parmac->visualizzazione_tot_cicli}}, .display = {.string_value = (const char ***)stringhe_totale_cicli}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = DROPLIST_PARAMETER}},
        {.t = unsigned_int, .d = {.uint = {3, 60, 20, &parmac->tempo_out_pagine}}, .display = {.format = (const char **)secondi}, .lvl = BIT_COSTRUTTORE, .runtime = {.userdata = NUMBER_PARAMETER}},
        // Parametri non visualizzati nella lista con gli altri
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->visualizzazione_data_ora}}, .lvl = BIT_NESSUNO},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->visualizzazione_pedante}}, .lvl = BIT_NESSUNO},
        {.t = unsigned_int, .d = {.uint = {0, 1, 0, &parmac->visualizzazione_esclusione_sapone}}, .lvl = BIT_NESSUNO},
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

    data.format(string, model_get_language(pmodel), par);
}

size_t parmac_get_tot_parameters(uint8_t al) {
    return parameter_get_count(parameters, NUM_PARAMETERS, model_get_bit_accesso(al));
}



static parameter_handle_t *get_actual_parameter(model_t *pmodel, size_t parameter, uint8_t al) {
    parameter_handle_t *par = parameter_get_handle(parameters, NUM_PARAMETERS, parameter, model_get_bit_accesso(al));
    assert(par != NULL);
    return par;
}


const char *parmac_commissioning_language_get_description(model_t *pmodel) {
    parameter_user_data_t data = parameter_get_user_data(&parameters[PARMAC_COMMISSIONING_LINGUA]);
    return data.descrizione[pmodel->prog.parmac.lingua];
}



void parmac_commissioning_operation(model_t *pmodel, parmac_commissioning_t parameter, int op) {
    parameter_operator(&parameters[parameter], op);
}