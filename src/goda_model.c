#include "php_goda.h"
#include "goda_model.h"

zend_class_entry *goda_model_ce;

zend_function_entry goda_model_methods[] = {

    ZEND_FE_END
};

GODA_MINIT_FUNCTION(model) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Goda\\Model", goda_model_methods);
    goda_model_ce = zend_register_internal_class(&ce);
    
    return SUCCESS;
}