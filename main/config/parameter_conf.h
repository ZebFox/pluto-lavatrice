#ifndef GEL_CONF_H_INCLUDED
#define GEL_CONF_H_INCLUDED

#include <stdint.h>
#include <stdlib.h>


typedef struct {
    const char **descrizione;
    void (*format)(char *string, const void *);
    size_t *opzioni;
    char ** unita;
} parameter_user_data_t;

#define GEL_PARAMETER_USER_DATA parameter_user_data_t

#endif
