#include "php_goda.h"
#include "goda_session.h"

zend_class_entry *goda_session_ce;

zend_function_entry goda_session_methods[] = {

    ZEND_FE_END
};

GODA_MINIT_FUNCTION(session) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Goda\\Session", goda_session_methods);
    goda_session_ce = zend_register_internal_class(&ce);

    return SUCCESS;
}