#include "php_goda.h"
#include "goda_cookie.h"

zend_class_entry *goda_cookie_ce;

zend_function_entry goda_cookie_methods[] = {

    ZEND_FE_END
};

GODA_MINIT_FUNCTION(cookie) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Goda\\Cookie", goda_cookie_methods);
    goda_cookie_ce = zend_register_internal_class(&ce);
    
    return SUCCESS;
}