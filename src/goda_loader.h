#ifndef GODA_LOADER_H
#define GODA_LOADER_H

extern zend_class_entry *goda_loader_ce;

void goda_loader_instance();

int goda_loader_include(zend_string *file, zval *retval);

int goda_loader_autoloader(char *class, size_t len);

GODA_MINIT_FUNCTION(loader);

#endif