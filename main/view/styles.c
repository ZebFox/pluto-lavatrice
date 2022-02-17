#include "styles.h"
#include "fonts/legacy_fonts.h"


lv_style_t style_label_8x16;
lv_style_t style_label_8x16_reverse;
lv_style_t style_label_6x8;
lv_style_t style_label_normal;
lv_style_t style_label_reverse;


void styles_init(void) {
    lv_style_copy(&style_label_8x16, &lv_style_plain);
    style_label_8x16.text.font = &hsw_8x16_font;

    lv_style_copy(&style_label_8x16_reverse, &lv_style_plain);
    style_label_8x16_reverse.text.font           = &hsw_8x16_font;
    style_label_8x16_reverse.body.main_color     = LV_COLOR_BLACK;
    style_label_8x16_reverse.text.color          = LV_COLOR_WHITE;
    style_label_8x16_reverse.body.padding.top    = 0;
    style_label_8x16_reverse.body.padding.bottom = 17;
    style_label_8x16_reverse.body.border.width   = 0;
    style_label_8x16_reverse.body.padding.inner  = 0;

    lv_style_copy(&style_label_normal, &lv_style_scr);

    lv_style_copy(&style_label_6x8, &lv_style_scr);
    style_label_6x8.text.font = &hsw_6x8_font;

    lv_style_copy(&style_label_reverse, &lv_style_scr);
    style_label_reverse.body.main_color     = LV_COLOR_BLACK;
    style_label_reverse.text.color          = LV_COLOR_WHITE;
    style_label_reverse.body.padding.top    = 0;
    style_label_reverse.body.padding.bottom = 9;
    style_label_reverse.body.border.width   = 0;
    style_label_reverse.body.padding.inner  = 0;
}