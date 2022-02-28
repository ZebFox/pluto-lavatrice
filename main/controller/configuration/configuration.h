#ifndef CONFIGURATION_H_INCLUDED
#define CONFIGURATION_H_INCLUDED


#include "model/model.h"
#include "model/programs.h"


void configuration_init(void);
int  configuration_load_all_data(model_t *pmodel);
int  configuration_save_data_version(void);
void configuration_remove_program_file(char *name);
int  configuration_update_program(programma_lavatrice_t *p);
int  configuration_add_program_to_index(char *filename);
void configuration_clear_orphan_programs(programma_preview_t *preview, int num);
int  configuration_read_local_data_version(void);
int  configuration_create_empty_program(model_t *pmodel);
int  configuration_load_programs_preview(programma_preview_t *previews, size_t len, uint16_t lingua);
int  configuration_load_program(model_t *pmodel, size_t num);
int  configuration_save_parmac(parmac_t *parmac);
void configuration_remove_program(programma_preview_t *previews, size_t len, size_t num);
void configuration_delete_all(void);


#endif