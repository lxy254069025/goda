#ifndef GODA_RESPONSE_H
#define GODA_RESPONSE_H

#define GODA_RESPONSE_HEADER        "header"

extern zend_class_entry *goda_response_ce;

void goda_response_instance(zval *this_ptr);

void goda_response_set_header(zval *this_ptr, zend_string *key, zend_string *value);

void goda_response_str_send(zval *this_ptr, zend_string *str);

void goda_response_send(zval *this_ptr, zval *body);

GODA_MINIT_FUNCTION(response);

#endif