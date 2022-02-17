#ifndef CONTROLLER_H_INCLUDED
#define CONTROLLER_H_INCLUDED

#include "model/model.h"
#include "view/view_types.h"


void controller_init(model_t *model);
void controller_process_msg(view_controller_command_t *msg, model_t *pmodel);
void controller_manage(model_t *pmodel);

#endif