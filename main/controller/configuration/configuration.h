#ifndef CONFIGURATION_H_INCLUDED
#define CONFIGURATION_H_INCLUDED


#include "model/programs.h"


void configuration_init(void);
int  configuration_save_data_version(void);
void configuration_remove_program_file(char *name);
int  configuration_update_program(programma_lavatrice_t *p);
int  configuration_update_program_index(programma_lavatrice_t *p, int len);
void configuration_clear_orphan_programs(programma_preview_t *preview, int num);
int  configuration_read_local_data_version(void);


#endif