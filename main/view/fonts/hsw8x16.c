#include "legacy_fonts.h"

static const struct {
    struct {
        char carattere;
        int  w;
        int  h;
    } header;
    uint8_t data[16];

} hsw_8x16fnt[256] = {
#include "hsw8x16.fnt"
};

/* Get the bitmap of `unicode_letter` from `font`. */
static const uint8_t *my_get_glyph_bitmap_cb(const lv_font_t *font, uint32_t unicode_letter) {
    return unicode_letter < 256 ? (const uint8_t *)&(hsw_8x16fnt[unicode_letter].data) : NULL;
}

lv_font_t hsw_8x16_font = {
    .get_glyph_dsc    = get_glyph_dsc_ramtex,   /*Set a callback to get info about gylphs*/
    .get_glyph_bitmap = my_get_glyph_bitmap_cb, /*Set a callback to get bitmap of a glyp*/
    .line_height      = 16,                     /*The real line height where any text fits*/
    .base_line        = 0,                      /*Base line measured from the top of line_height*/
    .user_data        = {.height = 16, .bitmap_width = 8, .char_width = 8},
};
