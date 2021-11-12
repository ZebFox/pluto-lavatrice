/**
 * @file lv_img.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "custom_lv_img.h"
#include "view/images/legacy.h"

#include "lvgl/src/lv_core/lv_debug.h"
#include "lvgl/src/lv_themes/lv_theme.h"
#include "lvgl/src/lv_draw/lv_img_decoder.h"
#include "lvgl/src/lv_misc/lv_fs.h"
#include "lvgl/src/lv_misc/lv_txt.h"
#include "lvgl/src/lv_misc/lv_log.h"

/*********************
 *      DEFINES
 *********************/
#define LV_OBJX_NAME "custom_lv_img"

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static bool     custom_lv_img_design(lv_obj_t *img, const lv_area_t *mask, lv_design_mode_t mode);
static lv_res_t custom_lv_img_signal(lv_obj_t *img, lv_signal_t sign, void *param);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_signal_cb_t ancestor_signal;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create an image objects
 * @param par pointer to an object, it will be the parent of the new button
 * @param copy pointer to a image object, if not NULL then the new object will be copied from it
 * @return pointer to the created image
 */
lv_obj_t *custom_lv_img_create(lv_obj_t *par, const lv_obj_t *copy) {
    LV_LOG_TRACE("custom image create started");

    lv_obj_t *new_img = NULL;

    /*Create a basic object*/
    new_img = lv_obj_create(par, copy);
    LV_ASSERT_MEM(new_img);
    if (new_img == NULL)
        return NULL;

    if (ancestor_signal == NULL)
        ancestor_signal = lv_obj_get_signal_cb(new_img);

    /*Extend the basic object to image object*/
    custom_lv_img_ext_t *ext = lv_obj_allocate_ext_attr(new_img, sizeof(custom_lv_img_ext_t));
    LV_ASSERT_MEM(ext);
    if (ext == NULL)
        return NULL;

    ext->src = NULL;
    ext->w   = lv_obj_get_width(new_img);
    ext->h   = lv_obj_get_height(new_img);

    /*Init the new object*/
    lv_obj_set_signal_cb(new_img, custom_lv_img_signal);
    lv_obj_set_design_cb(new_img, custom_lv_img_design);

    if (copy == NULL) {
        lv_obj_set_click(new_img, false);
        /* Enable auto size for non screens
         * because image screens are wallpapers
         * and must be screen sized*/
        if (par != NULL) {
            lv_obj_set_style(new_img, NULL); /*Inherit the style  by default*/
        } else {
            lv_obj_set_style(new_img, &lv_style_plain); /*Set a style for screens*/
        }
    } else {
        custom_lv_img_ext_t *copy_ext = lv_obj_get_ext_attr(copy);
        custom_lv_img_set_src(new_img, copy_ext->src);

        /*Refresh the style with new signal function*/
        lv_obj_refresh_style(new_img);
    }

    LV_LOG_INFO("image created");

    return new_img;
}

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the pixel map to display by the image
 * @param img pointer to an image object
 * @param data the image data
 */
void custom_lv_img_set_src(lv_obj_t *img, const void *src_img) {
    LV_ASSERT_OBJ(img, LV_OBJX_NAME);

    custom_lv_img_ext_t *ext = lv_obj_get_ext_attr(img);

    ext->src         = src_img;
    PGSYMBOL imgdata = (PGSYMBOL)ext->src;

    lv_obj_set_size(img, imgdata->sh.cxpix, imgdata->sh.cypix);

    lv_obj_invalidate(img);
}


/*=====================
 * Getter functions
 *====================*/

/**
 * Get the source of the image
 * @param img pointer to an image object
 * @return the image source (symbol, file name or C array)
 */
const void *custom_lv_img_get_src(lv_obj_t *img) {
    LV_ASSERT_OBJ(img, LV_OBJX_NAME);

    custom_lv_img_ext_t *ext = lv_obj_get_ext_attr(img);

    return ext->src;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Handle the drawing related tasks of the images
 * @param img pointer to an object
 * @param mask the object will be drawn only in this area
 * @param mode LV_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *                                  (return 'true' if yes)
 *             LV_DESIGN_DRAW: draw the object (always return 'true')
 *             LV_DESIGN_DRAW_POST: drawing after every children are drawn
 * @param return true/false, depends on 'mode'
 */
static bool custom_lv_img_design(lv_obj_t *img, const lv_area_t *mask, lv_design_mode_t mode) {
    custom_lv_img_ext_t *ext = lv_obj_get_ext_attr(img);

    if (mode == LV_DESIGN_COVER_CHK) {
        lv_coord_t x = lv_obj_get_x(img);
        lv_coord_t y = lv_obj_get_y(img);

        lv_coord_t w = ext->w;
        lv_coord_t h = ext->h;

        if (x <= mask->x1 && x + w >= mask->x2 && y <= mask->x1 && y + h >= mask->y2) {
            return true;
        } else {
            return false;
        }
    } else if (mode == LV_DESIGN_DRAW_MAIN) {
        if (ext->h == 0 || ext->w == 0)
            return true;
        lv_area_t coords, partial_coords;
        lv_obj_get_coords(img, &coords);

        // TODO: controlla l'intersezione tra maschera e coordinate prima di fare tutte le operazioni
        PGSYMBOL imgdata = (PGSYMBOL)ext->src;

        uint8_t set   = imgdata->sh.reverse ? 0 : 1;
        uint8_t unset = imgdata->sh.reverse ? 1 : 0;
        size_t  i = 0, j = 0, p = 0;
        for (i = 0; i < imgdata->sh.cypix; i++) {
            partial_coords.y1 = coords.y1 + i;
            partial_coords.y2 = partial_coords.y1;
            for (j = 0; j < imgdata->sh.cxpix / 8; j++) {
                lv_color_t color[8];
                partial_coords.x1 = coords.x1 + j * 8;
                partial_coords.x2 = partial_coords.x1 + 7;

                for (p = 0; p < 8; p++) {
                    if (((imgdata->data[i * imgdata->sh.cxpix / 8 + j] >> (7 - p)) & 1) > 0) {
                        color[p].full = set;
                    } else {
                        color[p].full = unset;
                    }
                }
                lv_draw_map(&partial_coords, mask, (const uint8_t *)&color, LV_OPA_COVER, false, false, LV_COLOR_WHITE,
                            LV_OPA_TRANSP);
            }
        }
    }

    return true;
}

/**
 * Signal function of the image
 * @param img pointer to an image object
 * @param sign a signal type from lv_signal_t enum
 * @param param pointer to a signal specific variable
 * @return LV_RES_OK: the object is not deleted in the function; LV_RES_INV: the object is deleted
 */
static lv_res_t custom_lv_img_signal(lv_obj_t *img, lv_signal_t sign, void *param) {
    lv_res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(img, sign, param);
    if (res != LV_RES_OK)
        return res;

    if (sign == LV_SIGNAL_GET_TYPE)
        return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);

    //custom_lv_img_ext_t *ext = lv_obj_get_ext_attr(img);
    if (sign == LV_SIGNAL_CLEANUP) {
        /*if(ext->src_type == LV_IMG_SRC_FILE || ext->src_type == LV_IMG_SRC_SYMBOL) {
            lv_mem_free(ext->src);
            ext->src      = NULL;
            ext->src_type = LV_IMG_SRC_UNKNOWN;
        }*/
    } else if (sign == LV_SIGNAL_STYLE_CHG) {
        /*Refresh the file name to refresh the symbol text size*/
        /*if(ext->src_type == LV_IMG_SRC_SYMBOL) {
            lv_img_set_src(img, ext->src);
        }*/
    }

    return res;
}
