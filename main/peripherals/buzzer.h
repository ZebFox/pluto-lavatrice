#ifndef BUZZER_H_INCLUDED
#define	BUZZER_H_INCLUDED

#include "hal/gpio_types.h"
#include <string.h>
#include <stdint.h>

void buzzer_init(void);
void buzzer_bip(size_t r, unsigned long t_on, unsigned long t_off);
void buzzer_check(void);


#endif