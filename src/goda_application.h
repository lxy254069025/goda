#ifndef GODA_APPLICATION_H
#define GODA_APPLICATION_H

#define GODA_APPLICATION_ROUTER     "router"
#define GODA_APPLICATION_APP        "app"
#define GODA_APPLICATION_REQUEST    "request"
#define GODA_APPLICATION_CONDIG     "config"

extern zend_class_entry *goda_application_ce;

GODA_MINIT_FUNCTION(application);

#endif