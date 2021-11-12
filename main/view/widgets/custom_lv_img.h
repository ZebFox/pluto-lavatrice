/**
 * @file lv_img.h
 *
 */

#ifndef CUSTOM_LV_IMG_H_INCLUDED
#define CUSTOM_LV_IMG_H_INCLUDED

/*********************
 *      INCLUDES
 *********************/
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif

#include "lvgl/src/lv_core/lv_obj.h"
#include "lvgl/src/lv_misc/lv_fs.h"
#include "lvgl/src/lv_draw/lv_draw.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
/*Data of image*/
typedef struct {
    /*No inherited ext. because inherited from the base object*/ /*Ext. of ancestor*/
    /*New data for this type */
    const void *src; /*Image source: Pointer to an array or a file or a symbol*/
    lv_coord_t  w;   /*Width of the image (Handled by the library)*/
    lv_coord_t  h;   /*Height of the image (Handled by the library)*/
} custom_lv_img_ext_t;


typedef uint8_t custom_lv_img_style_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create an image objects
 * @param par pointer to an object, it will be the parent of the new button
 * @param copy pointer to a image object, if not NULL then the new object will be copied from it
 * @return pointer to the created image
 */
lv_obj_t *custom_lv_img_create(lv_obj_t *par, const lv_obj_t *copy);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the pixel map to display by the image
 * @param img pointer to an image object
 * @param data the image data
 */
void custom_lv_img_set_src(lv_obj_t *img, const void *src_img);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the source of the image
 * @param img pointer to an image object
 * @return the image source (symbol, file name or C array)
 */
const void *custom_lv_img_get_src(lv_obj_t *img);

#endif