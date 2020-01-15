#include "php_goda.h"
#include "goda_response.h"

zend_class_entry *goda_response_ce;

ZEND_BEGIN_ARG_INFO_EX(goda_response_render_json_arg, 0, 0, 1)
    ZEND_ARG_ARRAY_INFO(0, array, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(goda_response_render_text_arg, 0, 0, 1)
    ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_METHOD(goda_response, renderJson) {
    zval *array, json_ecode, retval, params[2];
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(array)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_STRING(&json_ecode, "json_encode");
    
    ZVAL_ZVAL(&params[0], array, 1, 0);
    ZVAL_LONG(&params[1], 1<<8);

    ZVAL_NULL(&retval);
    if (call_user_function(EG(function_table), NULL, &json_ecode, &retval, 2, params) == SUCCESS) {
        php_write(Z_STRVAL_P(&retval), Z_STRLEN_P(&retval));
    } 
    zval_ptr_dtor(&json_ecode);
    zval_ptr_dtor(&retval);
    zval_ptr_dtor(&params[0]);
    zval_ptr_dtor(&params[1]);
}

ZEND_METHOD(goda_response, renderText) {
    char *str;
    size_t len;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STRING(str, len)
    ZEND_PARSE_PARAMETERS_END();

    php_write(str, len);
}

ZEND_METHOD(goda_response, redirect) {
    char *str;
    size_t len;
    sapi_header_line ctr = {0};
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STRING(str, len)
    ZEND_PARSE_PARAMETERS_END();

	ctr.line_len    = spprintf(&(ctr.line), 0, "%s %s", "Location:", str);
	ctr.response_code   = 0;
	if (sapi_header_op(SAPI_HEADER_REPLACE, &ctr) == SUCCESS) {
		efree(ctr.line);
        RETURN_FALSE;
	}
	efree(ctr.line);
    RETURN_TRUE;
}

zend_function_entry goda_response_methods[] = {
    ZEND_ME(goda_response, renderJson, goda_response_render_json_arg, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_response, renderText, goda_response_render_text_arg, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_response, redirect, goda_response_render_text_arg, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

GODA_MINIT_FUNCTION(response) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Goda\\Response", goda_response_methods);
    goda_response_ce = zend_register_internal_class(&ce);
    goda_response_ce->ce_flags |= ZEND_ACC_TRAIT;

    return SUCCESS;
}