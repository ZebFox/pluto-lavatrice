#include "controller.h"
#include "model/model.h"
#include "view/view.h"


void controller_init(model_t *pmodel) {
    view_change_page(pmodel, &page_splash);
}