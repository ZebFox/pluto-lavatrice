#include "legacy.h"

static uint8_t const bitmap[] = {
0x00,0x00,0x00,0x00,  /*  ................................  */
0x00,0x00,0x00,0x00,  /*  ................................  */
0x00,0x00,0x00,0x00,  /*  ................................  */
0x00,0x00,0x00,0x00,  /*  ................................  */
0x00,0x00,0x00,0x00,  /*  ................................  */
0x00,0x00,0x00,0x00,  /*  ................................  */
0x30,0x00,0x00,0xC0,  /*  ..%%....................%%......  */
0x30,0x86,0x10,0xC0,  /*  ..%%....%....%%....%....%%......  */
0x31,0xCF,0x38,0xC0,  /*  ..%%...%%%..%%%%..%%%...%%......  */
0x33,0x79,0xEC,0xC0,  /*  ..%%..%%.%%%%..%%%%.%%..%%......  */
0x32,0x30,0xC4,0xC0,  /*  ..%%..%...%%....%%...%..%%......  */
0x30,0x00,0x00,0xC0,  /*  ..%%....................%%......  */
0x30,0x06,0x00,0xC0,  /*  ..%%.........%%.........%%......  */
0x30,0x06,0x00,0xC0,  /*  ..%%.........%%.........%%......  */
0x30,0x06,0x00,0xC0,  /*  ..%%.........%%.........%%......  */
0x30,0x06,0x00,0xC0,  /*  ..%%.........%%.........%%......  */
0x30,0x06,0x00,0xC0,  /*  ..%%.........%%.........%%......  */
0x30,0x06,0x00,0xC0,  /*  ..%%.........%%.........%%......  */
0x30,0x06,0x00,0xC0,  /*  ..%%.........%%.........%%......  */
0x38,0x00,0x01,0xC0,  /*  ..%%%..................%%%......  */
0x1F,0xFF,0xFF,0x80,  /*  ...%%%%%%%%%%%%%%%%%%%%%%.......  */
0x0F,0xFF,0xFF,0x00,  /*  ....%%%%%%%%%%%%%%%%%%%%........  */
0x00,0x00,0x00,0x00,  /*  ................................  */
0x00,0x00,0x00,0x00,  /*  ................................  */
0x00,0x00,0x00,0x00,  /*  ................................  */
0x00,0x00,0x00,0x00,  /*  ................................  */
0x00,0x00,0x00,0x00,  /*  ................................  */
0x00,0x00,0x00,0x00   /*  ................................  */
};


const GSYMBOL legacy_img_prelavaggio = {
    .sh   = {1, 32, 28},
    .data = (uint8_t *)bitmap,
};