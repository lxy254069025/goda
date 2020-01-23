#include "php_goda.h"
#include "Zend/zend_inheritance.h"

#include "goda_controller.h"
#include "goda_view.h"
#include "goda_response.h"
#include "goda_request.h"

zend_class_entry *goda_controller_ce;

ZEND_BEGIN_ARG_INFO_EX(goda_controller_get_arg, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(goda_controller_assgin_arg, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, val)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(goda_controller_render_arg, 0, 0, 2)
    ZEND_ARG_INFO(0, filename)
    ZEND_ARG_ARRAY_INFO(0, val, 1)
ZEND_END_ARG_INFO()

ZEND_METHOD(goda_controller, __construct) {
    zval assgin, response = {{0}};
    array_init(&assgin);
    zend_update_property(goda_controller_ce, getThis(), ZEND_STRL(GODA_CONTROLLER_ASSGIN), &assgin);
    zval_ptr_dtor(&assgin);

    goda_response_instance(&response);
    zend_update_property(goda_controller_ce, getThis(), ZEND_STRL(GODA_CONTROLLER_RESPONSE), &response);
    zval_ptr_dtor(&response);
}

ZEND_METHOD(goda_controller, init) {
    
}

ZEND_METHOD(goda_controller, get) {
    zend_string *key;
    char *ret = "";
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(key)
    ZEND_PARSE_PARAMETERS_END();

    zval *request_ptr = zend_read_property(goda_controller_ce, getThis(), ZEND_STRL(GODA_CONTROLLER_REUQEST), 1, NULL);
    zval *param = goda_request_get_param(request_ptr, key);
    if (param) {
        RETURN_ZVAL(param, 1, 0);
    }
    RETURN_STRING(ret);
}

ZEND_METHOD(goda_controller, assgin) {
    zend_string *key;
    zval *val, *assgin;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(key)
        Z_PARAM_ZVAL(val)
    ZEND_PARSE_PARAMETERS_END();

    assgin = zend_read_property(goda_controller_ce, getThis(), ZEND_STRL(GODA_CONTROLLER_ASSGIN), 1, NULL);
    zend_hash_update(Z_ARRVAL_P(assgin), key, val);
}

ZEND_METHOD(goda_controller, render) {
    zend_string *filename, *key;
    zend_long idx;
    zval *val = nullptr, *assgin = nullptr, *value;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(filename)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(val)
    ZEND_PARSE_PARAMETERS_END();

    if (val) {
        assgin = zend_read_property(goda_controller_ce, getThis(), ZEND_STRL(GODA_CONTROLLER_ASSGIN), 1, NULL);

        ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(val), idx, key, value) {
            if(key) {
                zend_hash_update(Z_ARRVAL_P(assgin), key, value);
            } else {
                zend_hash_next_index_insert(Z_ARRVAL_P(assgin), value);
            }
        }ZEND_HASH_FOREACH_END();
    }

    goda_view_render(filename, assgin);
}

zend_function_entry goda_controller_methods[] = {
    ZEND_ME(goda_controller, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(goda_controller, init, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_controller, get, goda_controller_get_arg, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_controller, assgin, goda_controller_assgin_arg, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_controller, render, goda_controller_render_arg, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

GODA_MINIT_FUNCTION(controller) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Goda\\Controller", goda_controller_methods);
    goda_controller_ce = zend_register_internal_class(&ce);

    zend_declare_property_null(goda_controller_ce, ZEND_STRL(GODA_CONTROLLER_REUQEST), ZEND_ACC_PUBLIC);
    zend_declare_property_null(goda_controller_ce, ZEND_STRL(GODA_CONTROLLER_ASSGIN), ZEND_ACC_PUBLIC);
    zend_declare_property_null(goda_controller_ce, ZEND_STRL(GODA_CONTROLLER_RESPONSE), ZEND_ACC_PUBLIC);
    

    return SUCCESS;
}