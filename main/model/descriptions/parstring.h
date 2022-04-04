#ifndef PARSTRING_H_INCLUDED
#define PARSTRING_H_INCLUDED

#include "config/parameter_conf.h"


#define FDS(i)        ((parameter_user_data_t){pars_descriptions[i], formatta_decimi_secondo, NULL, NULL})
#define FTIME(i)      ((parameter_user_data_t){pars_descriptions[i], formatta_ms, NULL, NULL})
#define FFINT(i, fmt) ((parameter_user_data_t){pars_descriptions[i], formatta_int, fmt, NULL})
#define FINT(i)       ((parameter_user_data_t){pars_descriptions[i], formatta_int, NULL, NULL})
#define FOPT(i, vals) ((parameter_user_data_t){pars_descriptions[i], formatta_opt, NULL, (const char ***)vals})
#define FNULL         ((parameter_user_data_t){NULL, NULL, NULL, NULL})


void formatta_int(char *string, uint16_t language, const void *arg);
void formatta_opt(char *string, uint16_t language, const void *arg);
void formatta_ms(char *string, uint16_t language, const void *arg);
void formatta_decimi_secondo(char *string, uint16_t language, const void *arg);


#endif