#ifndef GODA_CONTROLLER_H
#define GODA_CONTROLLER_H

#define GODA_CONTROLLER_ROUTER      "router"
#define GODA_CONTROLLER_REUQEST     "request"
#define GODA_CONTROLLER_ASSGIN      "assgin"
#define GODA_CONTROLLER_PARAMS      "params"

extern zend_class_entry *goda_controller_ce;

void goda_controller_call_method(zval *handle, zval *router_ptr, zval *params);

GODA_MINIT_FUNCTION(controller);

#endif