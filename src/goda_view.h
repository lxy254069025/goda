#ifndef GODA_VIEW_H
#define GODA_VIEW_H

zend_long goda_extract_ref_overwrite(zend_array *arr, zend_array *symbol_table);

void goda_view_render(zend_string *filename, zval *assgin);

GODA_MINIT_FUNCTION(view);

#endif