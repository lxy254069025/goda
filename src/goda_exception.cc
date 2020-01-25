#include "php_goda.h"

zend_class_entry *goda_exception_ce;

void goda_throw_exception(int type, char *format, ...) {
    va_list args;
	char *message;

	va_start(args, format);
	vspprintf(&message, 0, format, args);
	va_end(args);
    
    zend_throw_exception(goda_exception_ce, message, type);
}

GODA_MINIT_FUNCTION(exception) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Goda\\Exception", NULL);
    goda_exception_ce = zend_register_internal_class_ex(&ce, zend_ce_exception);
    goda_exception_ce->ce_flags |= ZEND_ACC_FINAL;

    return SUCCESS;
}
