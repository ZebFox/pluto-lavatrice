#include "legacy.h"

static uint8_t const bitmap[] = {
0x00,0x00,0x00,0x00,  /*  ................................  */
0x00,0x00,0x00,0x00,  /*  ................................  */
0x00,0x00,0x00,0x00,  /*  ................................  */
0x00,0x1F,0x80,0x00,  /*  ...........%%%%%%...............  */
0x00,0x7F,0x80,0x00,  /*  .........%%%%%%%%...............  */
0x00,0xE0,0x00,0x00,  /*  ........%%%.....................  */
0x01,0xC0,0x00,0x00,  /*  .......%%%......................  */
0x03,0x80,0x00,0x00,  /*  ......%%%.......................  */
0x03,0x00,0x00,0x00,  /*  ......%%........................  */
0x06,0x07,0xF0,0x00,  /*  .....%%......%%%%%%%............  */
0x06,0x0F,0xF8,0x00,  /*  .....%%.....%%%%%%%%%...........  */
0x0E,0x1C,0x1C,0x00,  /*  ....%%%....%%%.....%%%..........  */
0x0C,0x38,0x0E,0x00,  /*  ....%%....%%%.......%%%.........  */
0x0C,0x30,0x06,0x00,  /*  ....%%....%%.........%%.........  */
0x0C,0x30,0x47,0x00,  /*  ....%%....%%.....%...%%%........  */
0x0C,0x30,0x63,0x00,  /*  ....%%....%%.....%%...%%........  */
0x0C,0x30,0xE3,0x00,  /*  ....%%....%%....%%%...%%........  */
0x0E,0x19,0xC3,0x00,  /*  ....%%%....%%..%%%....%%........  */
0x06,0x1F,0xC7,0x00,  /*  .....%%....%%%%%%%...%%%........  */
0x07,0x07,0x06,0x00,  /*  .....%%%.....%%%.....%%.........  */
0x03,0x00,0x0E,0x00,  /*  ......%%............%%%.........  */
0x03,0xC0,0x1C,0x00,  /*  ......%%%%.........%%%..........  */
0x01,0xF0,0xF8,0x00,  /*  .......%%%%%....%%%%%...........  */
0x00,0x7F,0xF0,0x00,  /*  .........%%%%%%%%%%%............  */
0x00,0x1F,0x80,0x00,  /*  ...........%%%%%%...............  */
0x00,0x00,0x00,0x00,  /*  ................................  */
0x00,0x00,0x00,0x00,  /*  ................................  */
0x00,0x00,0x00,0x00   /*  ................................  */
};


const GSYMBOL legacy_img_solo_centrifuga = {
    .sh   = {1, 32, 28},
    .data = (uint8_t *)bitmap,
};