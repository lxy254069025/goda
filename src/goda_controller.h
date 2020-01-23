#ifndef GODA_CONTROLLER_H
#define GODA_CONTROLLER_H

#define GODA_CONTROLLER_REUQEST     "request"
#define GODA_CONTROLLER_ASSGIN      "assgin"
#define GODA_CONTROLLER_RESPONSE    "response"

extern zend_class_entry *goda_controller_ce;

GODA_MINIT_FUNCTION(controller);

#endif