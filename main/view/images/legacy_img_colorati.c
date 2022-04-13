#include "legacy.h"

static uint8_t const bitmap[] = {
0x00,0x00,0x00,0x00,  /*  ................................  */
0x00,0x00,0x00,0x00,  /*  ................................  */
0x00,0x00,0x07,0x00,  /*  .....................%%%........  */
0x00,0x00,0x06,0x00,  /*  .....................%%.........  */
0x00,0x00,0x0E,0x00,  /*  ....................%%%.........  */
0x00,0x3F,0x8E,0x00,  /*  ..........%%%%%%%...%%%.........  */
0x00,0xFF,0xEE,0x00,  /*  ........%%%%%%%%%%%.%%%.........  */
0x01,0x80,0x7E,0x00,  /*  .......%%........%%%%%%.........  */
0x03,0x21,0x9E,0x00,  /*  ......%%..%....%%..%%%%.........  */
0x06,0x73,0xCF,0x00,  /*  .....%%..%%%..%%%%..%%%%........  */
0x0C,0x7B,0xCF,0x80,  /*  ....%%...%%%%.%%%%..%%%%%.......  */
0x18,0x71,0xCE,0x80,  /*  ...%%....%%%...%%%..%%%.%.......  */
0x1B,0x00,0x00,0xC0,  /*  ...%%.%%................%%......  */
0x37,0x00,0x0E,0x60,  /*  ..%%.%%%............%%%..%%.....  */
0x37,0x00,0x0E,0x60,  /*  ..%%.%%%............%%%..%%.....  */
0x32,0x00,0x0E,0x60,  /*  ..%%..%.............%%%..%%.....  */
0x60,0x00,0x0E,0x60,  /*  .%%.................%%%..%%.....  */
0x64,0x00,0x0E,0x60,  /*  .%%..%..............%%%..%%.....  */
0x6E,0x00,0x0E,0x60,  /*  .%%.%%%.............%%%..%%.....  */
0x6E,0x00,0x00,0xC0,  /*  .%%.%%%.................%%......  */
0x6E,0x00,0x00,0x80,  /*  .%%.%%%.................%.......  */
0x30,0xE0,0x7F,0x80,  /*  ..%%....%%%......%%%%%%%%.......  */
0x31,0xE0,0xFF,0x00,  /*  ..%%...%%%%.....%%%%%%%%........  */
0x19,0xC1,0x84,0x00,  /*  ...%%..%%%.....%%....%..........  */
0x0C,0x03,0x04,0x00,  /*  ....%%........%%.....%..........  */
0x07,0xFE,0x00,0x00,  /*  .....%%%%%%%%%%.................  */
0x00,0xFC,0x00,0x00,  /*  ........%%%%%%..................  */
0x00,0x00,0x00,0x00   /*  ................................  */
};


const GSYMBOL legacy_img_colorati = {
    .sh   = {1, 32, 28},
    .data = (uint8_t *)bitmap,
};