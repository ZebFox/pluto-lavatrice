#ifndef __LEGACY_FONTS_H__
#define __LEGACY_FONTS_H__

#include <stdbool.h>
#include <stdint.h>
#include "lvgl/lvgl.h"


bool get_glyph_dsc_ramtex(const lv_font_t *font, lv_font_glyph_dsc_t *dsc_out, uint32_t unicode_letter,
                          uint32_t unicode_letter_next);

extern lv_font_t hsw_8x8_font;
extern lv_font_t hsw_8x16_font;

#endif