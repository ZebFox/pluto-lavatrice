#include "intl.h"


const char *view_intl_get_string(model_t *pmodel, strings_t id) {
    return strings[id][model_get_language(pmodel)];
}


const char *view_intl_get_string_from_language(uint16_t language, strings_t id) {
    return strings[id][language];
}