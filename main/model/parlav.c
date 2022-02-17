#include "model.h"
#include "programs.h"
#include "gel/parameter/parameter.h"
#include "esp_log.h"


#define NUMP           61
#define MAX_INDEX_PARS 50


static const char        *TAG            = "Parlav";
static size_t             num_parameters = 0;
static parameter_handle_t p[NUMP];
static struct {
    unsigned int tipo;
    int          num;
    int          t[MAX_INDEX_PARS];
} transformations[STEP_NUM] = {
    {STEP_AMMOLLO, 41, {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                        21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40}},
    {STEP_PRELAVAGGIO, 41, {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                            21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40}},
    {STEP_LAVAGGIO, 41, {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                         21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40}},
    {STEP_RISCIACQUO, 36, {0,  59, 2,  3,  4,  5,  6,  7,  8,  9,  10, 15, 16, 17, 19, 60, 21, 22,
                           23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40}},
    {STEP_SCARICO, 7, {0, 41, 7, 8, 9, 10, 42}},
    {STEP_CENTRIFUGA, 15, {0, 43, 44, 45, 42, 58, 46, 47, 48, 49, 50, 51, 52, 53, 54}},
    {STEP_SROTOLAMENTO, 4, {0, 7, 9, 10}},
    {STEP_ATTESA, 13, {0, 55, 56, 57, 41, 7, 8, 9, 10, 11, 12, 13, 14}},
};

const char *formato_cm = "%i cm";
const char *formato_lt = "%i lt";


int parlav_init(parmac_t *parmac, parametri_step_t *step) {
    int accesso_saponi[10] = {
        parmac->numero_saponi_utilizzabili >= 1 ? BIT_UTENTE : BIT_NESSUNO,
        parmac->numero_saponi_utilizzabili >= 2 ? BIT_UTENTE : BIT_NESSUNO,
        parmac->numero_saponi_utilizzabili >= 3 ? BIT_UTENTE : BIT_NESSUNO,
        parmac->numero_saponi_utilizzabili >= 4 ? BIT_UTENTE : BIT_NESSUNO,
        parmac->numero_saponi_utilizzabili >= 5 ? BIT_UTENTE : BIT_NESSUNO,
        parmac->numero_saponi_utilizzabili >= 6 ? BIT_UTENTE : BIT_NESSUNO,
        parmac->numero_saponi_utilizzabili >= 7 ? BIT_UTENTE : BIT_NESSUNO,
        parmac->numero_saponi_utilizzabili >= 8 ? BIT_UTENTE : BIT_NESSUNO,
        parmac->numero_saponi_utilizzabili >= 9 ? BIT_UTENTE : BIT_NESSUNO,
        parmac->numero_saponi_utilizzabili >= 10 ? BIT_UTENTE : BIT_NESSUNO,
    };

    uint16_t   *livmin, *livmax;
    const char *fmt;

    switch (parmac->tipo_livello) {
        case LIVELLO_CENTIMETRI:
            livmin = &parmac->centimetri_minimo_riscaldo;
            livmax = &parmac->centimetri_max_livello;
            fmt    = formato_cm;
            break;
        default:
            livmin = &parmac->litri_minimi_riscaldo;
            livmax = &parmac->litri_massimi;
            fmt    = formato_lt;
            break;
    }

    parameter_handle_t tmp[NUMP] = {
        PARAMETER(&step->durata, 0, 60 * 60, 120, NULL, BIT_UTENTE),
        PARAMETER(&step->tempo_attivo, 0, 4, 0, NULL, BIT_UTENTE),
        PARAMETER(&step->abilitazione_moto_fermo_riempimento, 0, 1, 0, NULL, BIT_UTENTE),
        PARAMETER_DLIMITS(&step->velocita_riempimento, &parmac->velocita_minima_lavaggio,
                          &parmac->velocita_massima_lavaggio, 0, 0, 40, NULL, BIT_UTENTE),
        PARAMETER(&step->abilitazione_inversione_riempimento, 0, 1, 0, NULL, BIT_UTENTE),
        PARAMETER(&step->giro_riempimento, 0, 900, 0, NULL, BIT_UTENTE),
        PARAMETER(&step->pausa_riempimento, 0, 900, 0, NULL, BIT_UTENTE),
        PARAMETER_DLIMITS(&step->velocita_lavaggio, &parmac->velocita_minima_lavaggio,
                          &parmac->velocita_massima_lavaggio, 0, 0, 40, NULL, BIT_UTENTE),
        PARAMETER(&step->abilitazione_inversione_lavaggio, 0, 1, 0, NULL, BIT_UTENTE),
        PARAMETER(&step->giro_lavaggio, 0, 900, 0, NULL, BIT_UTENTE),
        PARAMETER(&step->pausa_lavaggio, 0, 900, 0, NULL, BIT_UTENTE),
        PARAMETER(&step->abilitazione_riscaldamento, 0, 1, 0, NULL, BIT_UTENTE),
        PARAMETER_DLIMITS(&step->temperatura, NULL, &parmac->temperatura_massima, 0, 0, 0, NULL, BIT_UTENTE),
        PARAMETER(&step->riscaldamento_diretto_indiretto, 0, 1, 0, NULL, BIT_UTENTE),
        PARAMETER(&step->abilitazione_controllo_temperatura_continuo, 0, 1, 0, NULL, BIT_UTENTE),
        PARAMETER_DLIMITS(&step->livello, livmin, livmax, 0, 0, 0, NULL, BIT_UTENTE),
        PARAMETER(&step->abilitazione_ricircolo, 0, 1, 0, NULL, BIT_UTENTE),
        PARAMETER(&step->abilitazione_acqua_fredda, 0, 1, 0, NULL, BIT_UTENTE),
        PARAMETER(&step->abilitazione_acqua_calda, 0, 1, 0, NULL, BIT_UTENTE),
        PARAMETER(&step->abilitazione_acqua_depurata, 0, 1, 0, NULL, BIT_UTENTE),
        PARAMETER(&step->tempo_attivo_sapone, 0, 3, 0, NULL, BIT_UTENTE),
        PARAMETER(&step->tempo_sapone_1, 0, 240, 0, NULL, accesso_saponi[0]),
        PARAMETER(&step->ritardo_sapone_1, 0, 3600, 0, NULL, accesso_saponi[0]),
        PARAMETER(&step->tempo_sapone_2, 0, 240, 0, NULL, accesso_saponi[1]),
        PARAMETER(&step->ritardo_sapone_2, 0, 3600, 0, NULL, accesso_saponi[1]),
        PARAMETER(&step->tempo_sapone_3, 0, 240, 0, NULL, accesso_saponi[2]),
        PARAMETER(&step->ritardo_sapone_3, 0, 3600, 0, NULL, accesso_saponi[2]),
        PARAMETER(&step->tempo_sapone_4, 0, 240, 0, NULL, accesso_saponi[3]),
        PARAMETER(&step->ritardo_sapone_4, 0, 3600, 0, NULL, accesso_saponi[3]),
        PARAMETER(&step->tempo_sapone_5, 0, 240, 0, NULL, accesso_saponi[4]),
        PARAMETER(&step->ritardo_sapone_5, 0, 3600, 0, NULL, accesso_saponi[4]),
        PARAMETER(&step->tempo_sapone_6, 0, 240, 0, NULL, accesso_saponi[5]),
        PARAMETER(&step->ritardo_sapone_6, 0, 3600, 0, NULL, accesso_saponi[5]),
        PARAMETER(&step->tempo_sapone_7, 0, 240, 0, NULL, accesso_saponi[6]),
        PARAMETER(&step->ritardo_sapone_7, 0, 3600, 0, NULL, accesso_saponi[6]),
        PARAMETER(&step->tempo_sapone_8, 0, 240, 0, NULL, accesso_saponi[7]),
        PARAMETER(&step->ritardo_sapone_8, 0, 3600, 0, NULL, accesso_saponi[7]),
        PARAMETER(&step->tempo_sapone_9, 0, 240, 0, NULL, accesso_saponi[8]),
        PARAMETER(&step->ritardo_sapone_9, 0, 3600, 0, NULL, accesso_saponi[8]),
        PARAMETER(&step->tempo_sapone_10, 0, 240, 0, NULL, accesso_saponi[9]),
        PARAMETER(&step->ritardo_sapone_10, 0, 3600, 0, NULL, accesso_saponi[9]),
        PARAMETER(&step->abilitazione_moto_fermo, 0, 1, 0, NULL, BIT_UTENTE),
        PARAMETER(&step->abilitazione_recupero, 0, 1, 0, NULL, BIT_UTENTE),
        PARAMETER(&step->tempo_preparazione, 0, 1, 0, NULL, BIT_UTENTE),
        PARAMETER_DLIMITS(&step->velocita_preparazione, &parmac->velocita_minima_preparazione,
                          &parmac->velocita_massima_preparazione, 0, 0, 0, NULL, BIT_UTENTE),
        PARAMETER(&step->tempo_scarico, 0, 2, 0, NULL, BIT_UTENTE),
        PARAMETER_DLIMITS(&step->velocita_centrifuga_1, &parmac->velocita_minima_centrifuga_1,
                          &parmac->velocita_massima_centrifuga_1, 0, 0, 0, NULL, BIT_UTENTE),
        PARAMETER_DLIMITS(&step->tempo_rampa_1, &parmac->tempo_minimo_rampa, &parmac->tempo_massimo_rampa, 0, 0, 0,
                          NULL, BIT_UTENTE),
        PARAMETER(&step->tempo_attesa_centrifuga_1, 0, 240, 5, NULL, BIT_UTENTE),
        PARAMETER_DLIMITS(&step->velocita_centrifuga_2, &parmac->velocita_minima_centrifuga_2,
                          &parmac->velocita_massima_centrifuga_2, 0, 0, 0, NULL, BIT_UTENTE),
        PARAMETER_DLIMITS(&step->tempo_rampa_2, &parmac->tempo_minimo_rampa, &parmac->tempo_massimo_rampa, 0, 0, 0,
                          NULL, BIT_UTENTE),
        PARAMETER(&step->tempo_attesa_centrifuga_2, 0, 240, 5, NULL, BIT_UTENTE),
        PARAMETER_DLIMITS(&step->velocita_centrifuga_3, &parmac->velocita_minima_centrifuga_3,
                          &parmac->velocita_massima_centrifuga_3, 0, 0, 0, NULL, BIT_UTENTE),
        PARAMETER_DLIMITS(&step->tempo_rampa_3, &parmac->tempo_minimo_rampa, &parmac->tempo_massimo_rampa, 0, 0, 0,
                          NULL, BIT_UTENTE),
        PARAMETER_DLIMITS(&step->tempo_frenata, &parmac->tempo_minimo_frenata, NULL, 0, 240, 35, NULL, BIT_UTENTE),
        PARAMETER(&step->tempo_attesa, 0, 9, 0, NULL, BIT_UTENTE),
        PARAMETER(&step->tempo_avviso_attesa_on, 0, 240, 5, NULL, BIT_UTENTE),
        PARAMETER(&step->tempo_avviso_attesa_off, 0, 240, 5, NULL, BIT_UTENTE),
        PARAMETER(&step->numero_rampe, 1, 3, 3, NULL, BIT_UTENTE),
        PARAMETER(&step->tempo_attivo, 0, 1, 0, NULL, BIT_UTENTE),
        PARAMETER(&step->tempo_attivo_sapone, 0, 1, 0, NULL, BIT_UTENTE),
    };

    num_parameters = 0;
    for (int i = 0; i < STEP_NUM; i++) {
        if (transformations[i].tipo == step->tipo) {
            num_parameters = transformations[i].num;
            for (unsigned int j = 0; j < num_parameters; j++) {
                p[j] = tmp[transformations[i].t[j]];

                // if (p[j].display.description == NULL) p[j].display.description =
                // descrizioni_parametri_lavaggio[transformations[i].t[j]];
            }

            break;
        }
    }

    int num = 0;
    if ((num = parameter_check_ranges(p, num_parameters)) > 0) {
        ESP_LOGW(TAG, "%i parametri di lavaggio avevano un valore non valido", num);
        return 1;
    }

    return 0;
}
