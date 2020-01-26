#ifndef GODA_EXCEPTION_H
#define GODA_EXCEPTION_H

extern zend_class_entry *goda_exception_ce;

void goda_throw_exception(int type, const char *format, ...);

GODA_MINIT_FUNCTION(exception);

#endif