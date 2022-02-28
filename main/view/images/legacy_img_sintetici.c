#include "legacy.h"

static uint8_t const bitmap[] = {
0x00,0x00,0x00,0x00,  /*  ................................  */
0x00,0x00,0x00,0x00,  /*  ................................  */
0x00,0x0C,0x00,0x00,  /*  ............%%..................  */
0x00,0x1E,0x00,0x00,  /*  ...........%%%%.................  */
0x00,0x1F,0x00,0x00,  /*  ...........%%%%%................  */
0x00,0x3B,0x80,0x00,  /*  ..........%%%.%%%...............  */
0x00,0x39,0xC0,0x00,  /*  ..........%%%..%%%..............  */
0x00,0x70,0xE0,0x00,  /*  .........%%%....%%%.............  */
0x00,0x70,0x70,0x00,  /*  .........%%%.....%%%............  */
0x00,0xE0,0x38,0x00,  /*  ........%%%.......%%%...........  */
0x00,0xE0,0x1C,0x00,  /*  ........%%%........%%%..........  */
0x01,0xC0,0x0E,0x00,  /*  .......%%%..........%%%.........  */
0x01,0xC0,0x07,0x00,  /*  .......%%%...........%%%........  */
0x03,0x80,0x03,0x80,  /*  ......%%%.............%%%.......  */
0x03,0x80,0x01,0x80,  /*  ......%%%..............%%.......  */
0x07,0x00,0x20,0x00,  /*  .....%%%..........%.............  */
0x07,0x00,0x70,0x00,  /*  .....%%%.........%%%............  */
0x0E,0x00,0x38,0x00,  /*  ....%%%...........%%%...........  */
0x0E,0x00,0x1C,0x00,  /*  ....%%%............%%%..........  */
0x1C,0x00,0x0E,0x00,  /*  ...%%%..............%%%.........  */
0x1C,0x00,0x07,0x00,  /*  ...%%%...............%%%........  */
0x38,0x00,0x03,0x80,  /*  ..%%%.................%%%.......  */
0x38,0x00,0x03,0xC0,  /*  ..%%%.................%%%%......  */
0x3F,0xFF,0xFF,0xC0,  /*  ..%%%%%%%%%%%%%%%%%%%%%%%%......  */
0x3F,0xFF,0xFF,0xC0,  /*  ..%%%%%%%%%%%%%%%%%%%%%%%%......  */
0x1F,0xFF,0xFF,0x80,  /*  ...%%%%%%%%%%%%%%%%%%%%%%.......  */
0x00,0x00,0x00,0x00,  /*  ................................  */
0x00,0x00,0x00,0x00   /*  ................................  */
};


const GSYMBOL legacy_img_sintetici = {
    .sh   = {1, 32, 28},
    .data = (uint8_t *)bitmap,
};