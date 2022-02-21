#ifndef AUTOGEN_FILE_PARS_H_INCLUDED
#define AUTOGEN_FILE_PARS_H_INCLUDED

typedef enum {
    PARS_LINGUE_ITALIANO = 0,
    PARS_LINGUE_INGLESE,
} pars_lingue_t;

extern const char *pars_lingue[2][2];
typedef enum {
    PARS_LOGHI_NESSUNO = 0,
    PARS_LOGHI_MS,
    PARS_LOGHI_LAVENDA,
    PARS_LOGHI_ROTONDI,
    PARS_LOGHI_SCHULTHESS,
    PARS_LOGHI_HOOVER,
} pars_loghi_t;

extern const char *pars_loghi[6][1];
typedef enum {
    PARS_DESCRIPTIONS_LINGUA = 0,
    PARS_DESCRIPTIONS_LOGO,
    PARS_DESCRIPTIONS_LIVELLO_ACCESSO,
    PARS_DESCRIPTIONS_TEMPO_TASTO_PAUSA,
    PARS_DESCRIPTIONS_TEMPO_TASTO_STOP,
} pars_descriptions_t;

extern const char *pars_descriptions[5][2];

#endif
