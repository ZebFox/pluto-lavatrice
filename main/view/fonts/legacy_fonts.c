#include "legacy_fonts.h"

bool get_glyph_dsc_ramtex(const lv_font_t *font, lv_font_glyph_dsc_t *dsc_out, uint32_t unicode_letter,
                          uint32_t unicode_letter_next) {
    dsc_out->adv_w = font->user_data.char_width;   /*Horizontal space required by the glyph in [px]*/
    dsc_out->box_h = font->user_data.height;       /*Height of the bitmap in [px]*/
    dsc_out->box_w = font->user_data.bitmap_width; /*Width of the bitmap in [px]*/
    dsc_out->ofs_x = 0;                            /*X offset of the bitmap in [pf]*/
    dsc_out->ofs_y = 0;                            /*Y offset of the bitmap measured from the as line*/
    dsc_out->bpp   = 1;                            /*Bits per pixel: 1/2/4/8*/

    return true; /*true: glyph found; false: glyph was not found*/
}
