#ifndef POWER_OFF_H_INCLUDED
#define POWER_OFF_H_INCLUDED

void power_off_init(void);
void power_off_register_callback(void (*cb)(void *), void *data);

#endif