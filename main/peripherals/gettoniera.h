#ifndef GETTONIERA_H_INCLUDED
#define GETTONIERA_H_INCLUDED

typedef enum {
    GETT1 = 0,     // 1 EURO
    GETT2,         // 20 CENT
    GETT3,         // 50 CENT
    GETT4,         // 10 CENT
    GETT5,         // 2 EURO
    GETT_NUM
} gett_t;


void         gettoniera_init(void);
void         gettoniera_reset_count(gett_t i);
void         gettoniera_reset_all_count(void);
int          gettoniera_take_insert(void);
unsigned int gettoniera_get_count(gett_t i);
int          gettoniera_get_pulse_level(gett_t i);


#endif