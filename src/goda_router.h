#ifndef GODA_ROUTER_H
#define GODA_ROUTER_H

#define GODA_ROUTER_CONTROLLER      "controller"
#define GODA_ROUTER_ACTION          "action"
#define GODA_ROUTER_STATUS          "status"


extern zend_class_entry *goda_router_ce;

void goda_router_instance(zval *this_ptr);

void goda_router_add_route(zval *this_ptr, zend_string *method, zend_string *path, zend_string *handle);

int goda_router_parase_routeing(zval *this_ptr, zval *request_ptr);

void goda_router_run(zval *this_ptr, zval *request_ptr);

GODA_MINIT_FUNCTION(router);

#endif