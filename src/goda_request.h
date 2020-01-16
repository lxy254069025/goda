#ifndef GODA_REQUEST_H
#define GODA_REQUEST_H

#define GODA_REQUEST_PARAMS     "params"

void goda_request_instance(zval *this_ptr);

zval *goda_request_get_server(char *key, size_t len);

const char *goda_request_get_method();

zend_string *goda_request_get_base_uri();

int goda_request_regexp_match_uri(zval *this_ptr, zend_string *path, zend_string *uri);

void goda_request_set_params(zval *this_ptr, zval *params);

zval *goda_request_get_params(zval *this_ptr, zend_string *key);

GODA_MINIT_FUNCTION(request);

#endif