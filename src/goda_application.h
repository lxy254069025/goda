#ifndef GODA_APPLICATION_H
#define GODA_APPLICATION_H

#define GODA_APPLICATION_ROUTER     "router"
#define GODA_APPLICATION_APP        "app"

extern zend_class_entry *goda_application_ce;

GODA_MINIT_FUNCTION(application);

#endif