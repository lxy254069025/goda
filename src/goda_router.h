#ifndef GODA_ROUTER_H
#define GODA_ROUTER_H

extern zend_class_entry *goda_router_ce;

zval *goda_router_request_get_server(char *key, size_t len);

const char *goda_router_request_get_method();

zend_string *goda_router_get_base_uri();

void goda_router_instance(zval *this_ptr);

void goda_router_add_route(zval *this_ptr, zend_string *method, zend_string *path, zend_string *handle);

void goda_router_run(zval *this_ptr);

int goda_router_match_pce_regexp(zval *this_ptr, zend_string *key, zend_string *uri, zval *params);

GODA_MINIT_FUNCTION(router);

#endif