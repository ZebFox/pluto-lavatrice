#include "legacy.h"

static uint8_t const bitmap[] = {
0x00,0x00,0x00,0x00,  /*  ................................  */
0x00,0x00,0x00,0x00,  /*  ................................  */
0x01,0xE0,0x78,0x00,  /*  .......%%%%......%%%%...........  */
0x03,0x10,0x8C,0x00,  /*  ......%%...%....%...%%..........  */
0x05,0x09,0x0A,0x00,  /*  .....%.%....%..%....%.%.........  */
0x09,0x06,0x09,0x00,  /*  ....%..%.....%%.....%..%........  */
0x11,0x00,0x08,0x80,  /*  ...%...%............%...%.......  */
0x21,0x00,0x08,0x40,  /*  ..%....%............%....%......  */
0x41,0x00,0x08,0x20,  /*  .%.....%............%.....%.....  */
0x41,0x0C,0x08,0x20,  /*  .%.....%....%%......%.....%.....  */
0x21,0x1C,0x08,0x40,  /*  ..%....%...%%%......%....%......  */
0x13,0x3F,0x0C,0x80,  /*  ...%..%%..%%%%%%....%%..%.......  */
0x0D,0x1E,0x0B,0x00,  /*  ....%%.%...%%%%.....%.%%........  */
0x01,0x0E,0x08,0x00,  /*  .......%....%%%.....%...........  */
0x01,0x0E,0x08,0x00,  /*  .......%....%%%.....%...........  */
0x01,0x00,0x08,0x00,  /*  .......%............%...........  */
0x01,0x00,0x08,0x00,  /*  .......%............%...........  */
0x01,0x00,0x08,0x00,  /*  .......%............%...........  */
0x01,0x00,0x08,0x00,  /*  .......%............%...........  */
0x01,0x00,0x08,0x00,  /*  .......%............%...........  */
0x01,0x00,0x08,0x00,  /*  .......%............%...........  */
0x01,0x00,0x08,0x00,  /*  .......%............%...........  */
0x01,0x00,0x08,0x00,  /*  .......%............%...........  */
0x01,0x00,0x08,0x00,  /*  .......%............%...........  */
0x01,0x00,0x08,0x00,  /*  .......%............%...........  */
0x01,0x00,0x08,0x00,  /*  .......%............%...........  */
0x01,0xFF,0xF8,0x00,  /*  .......%%%%%%%%%%%%%%...........  */
0x00,0x00,0x00,0x00   /*  ................................  */
};


const GSYMBOL legacy_img_sporchi = {
    .sh   = {1, 32, 28},
    .data = (uint8_t *)bitmap,
};