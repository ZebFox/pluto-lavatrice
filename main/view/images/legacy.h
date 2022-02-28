#ifndef LEGACY_H_INCLUDED
#define LEGACY_H_INCLUDED

#include <stdlib.h>

#include "lvgl/lvgl.h"


#define LEGACY_CANVAS_BUFFER_SIZE(x) LV_CANVAS_BUF_SIZE_INDEXED_1BIT(x.sh.cxpix, x.sh.cypix)


typedef struct _GSYMHEAD /* Symbol header */
{
    uint8_t reverse; /* Symbol ID (not used by GLCD)*/
    size_t  cxpix;   /* Symbol size in no X pixels */
    size_t  cypix;   /* Symbol size in no Y pixels */
} GSYMHEAD, *PGSYMHEAD;

typedef struct _GSYMBOL /* One table entry */
{
    GSYMHEAD sh;   /* Symbol header */
    uint8_t *data; /* Symbol data, variable length = (cxpix/8+1)*cypix */
} GSYMBOL, *PGSYMBOL;



extern const GSYMBOL legacy_img_insert_coin;
extern const GSYMBOL legacy_img_programs;
extern const GSYMBOL legacy_img_warning;
extern const GSYMBOL legacy_img_stop;
extern const GSYMBOL legacy_img_chiudere_oblo;
extern const GSYMBOL legacy_img_aprire_oblo;

extern const GSYMBOL legacy_img_program_caldo;
extern const GSYMBOL legacy_img_program_medio;
extern const GSYMBOL legacy_img_program_tiepido;
extern const GSYMBOL legacy_img_program_freddo;
extern const GSYMBOL legacy_img_program_lana;
extern const GSYMBOL legacy_img_raffreddamento;
extern const GSYMBOL legacy_img_antipiega;

extern const GSYMBOL legacy_img_logo_ciao;
extern const GSYMBOL legacy_img_logo_hoover;
extern const GSYMBOL legacy_img_logo_msgroup;
extern const GSYMBOL legacy_img_logo_schulthess;
extern const GSYMBOL legacy_img_logo_rotondi;
extern const GSYMBOL legacy_img_logo_hsw;

extern const GSYMBOL legacy_img_left;
extern const GSYMBOL legacy_img_right;
extern const GSYMBOL legacy_img_wash_sm;
extern const GSYMBOL legacy_img_level;
extern const GSYMBOL legacy_img_time;
extern const GSYMBOL legacy_img_temperature;
extern const GSYMBOL legacy_img_speed;

extern const GSYMBOL legacy_img_molto_sporchi;
extern const GSYMBOL legacy_img_sporchi;
extern const GSYMBOL legacy_img_colorati;
extern const GSYMBOL legacy_img_sintetici;
extern const GSYMBOL legacy_img_piumoni;
extern const GSYMBOL legacy_img_delicati;
extern const GSYMBOL legacy_img_lana;
extern const GSYMBOL legacy_img_lino_tendaggi;
extern const GSYMBOL legacy_img_solo_centrifuga;
extern const GSYMBOL legacy_img_solo_centrifuga_delicati;
extern const GSYMBOL legacy_img_sanificazione;
extern const GSYMBOL legacy_img_ammollo;
extern const GSYMBOL legacy_img_prelavaggio;
extern const GSYMBOL legacy_img_risciacquo;

#endif
