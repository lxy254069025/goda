#ifndef GODA_RESPONSE_H
#define GODA_RESPONSE_H

extern zend_class_entry *goda_response_ce;

void goda_response_instance(zval *this_ptr);

GODA_MINIT_FUNCTION(response);

#endif