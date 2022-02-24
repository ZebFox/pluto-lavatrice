#ifndef PARLAV_H_INCLUDED
#define PARLAV_H_INCLUDED

#include "model.h"


int         parlav_init(parmac_t *parmac, parametri_step_t *step);
int         parlav_get_tot_parameters(int level);
void        parlav_operation(model_t *pmodel, size_t parameter, int op, uint8_t al);
const char *parlav_get_description(model_t *pmodel, size_t parameter, uint8_t al);
void        parlav_format_value(model_t *pmodel, char *string, size_t parameter, uint8_t al);


#endif