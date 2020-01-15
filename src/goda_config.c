#include "php_goda.h"
#include "goda_config.h"

zend_class_entry *goda_config_ce;

zend_function_entry goda_config_methods[] = {

    ZEND_FE_END
};

GODA_MINIT_FUNCTION(config) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Goda\\Config", goda_config_methods);
    goda_config_ce = zend_register_internal_class_ex(&ce, NULL);
    goda_config_ce->ce_flags |= ZEND_ACC_FINAL;
    
    return SUCCESS;
}