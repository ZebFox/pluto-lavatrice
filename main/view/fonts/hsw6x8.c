#include "legacy_fonts.h"

static const struct {
    struct {
        char carattere;
        int  w;
        int  h;
    } header;
    uint8_t data[8];

} hsw_6x8fnt[128] = {
#include "hsw6x8.fnt"
};

/* Get the bitmap of `unicode_letter` from `font`. */
static const uint8_t *my_get_glyph_bitmap_cb(const lv_font_t *font, uint32_t unicode_letter) {
    return unicode_letter < 128 ? (const uint8_t *)&(hsw_6x8fnt[unicode_letter].data) : NULL;
}

lv_font_t hsw_6x8_font = {
    .get_glyph_dsc    = get_glyph_dsc_ramtex,   /*Set a callback to get info about gylphs*/
    .get_glyph_bitmap = my_get_glyph_bitmap_cb, /*Set a callback to get bitmap of a glyp*/
    .line_height      = 8,                      /*The real line height where any text fits*/
    .base_line        = 0,                      /*Base line measured from the top of line_height*/
    .user_data        = {.height = 8, .bitmap_width = 8, .char_width = 6},
};
