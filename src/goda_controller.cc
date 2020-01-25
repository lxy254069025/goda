#include "php_goda.h"
#include "Zend/zend_inheritance.h"

#include "goda_controller.h"
#include "goda_view.h"
#include "goda_response.h"
#include "goda_request.h"
#include "goda_exception.h"


zend_class_entry *goda_controller_ce;

ZEND_BEGIN_ARG_INFO_EX(goda_controller_get_arg, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(goda_controller_assgin_arg, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, val)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(goda_controller_render_arg, 0, 0, 3)
    ZEND_ARG_INFO(0, filename)
    ZEND_ARG_ARRAY_INFO(0, val, 1)
    ZEND_ARG_INFO(0, saveData)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(goda_controller_render_json_arg, 0, 0, 1)
    ZEND_ARG_ARRAY_INFO(0, array, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(goda_controller_render_text_arg, 0, 0, 1)
    ZEND_ARG_INFO(0, str)
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
    zval *val, *assgin, *value;
    zval view_result, render_val;
    zend_bool saveData = 1;

    ZEND_PARSE_PARAMETERS_START(1, 3)
        Z_PARAM_STR(filename)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(val)
        Z_PARAM_BOOL(saveData)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_UNDEF(&view_result);
    ZVAL_UNDEF(&render_val);
    if (saveData) {
        if (val) {
            assgin = zend_read_property(goda_controller_ce, getThis(), ZEND_STRL(GODA_CONTROLLER_ASSGIN), 1, NULL);
            zend_hash_merge(Z_ARRVAL_P(assgin), Z_ARRVAL_P(val), zval_add_ref, 0);
            ZVAL_COPY_VALUE(&render_val, assgin);
        }
    } else {
        if (val) {
            ZVAL_COPY_VALUE(&render_val, val);
        }
    }

    if (Z_TYPE(render_val) == IS_UNDEF) {
        ZVAL_EMPTY_ARRAY(&render_val);
    }

    if (goda_view_render(filename, &render_val, &view_result) == 0) {
        zval_ptr_dtor(&view_result);
        zval_ptr_dtor(&render_val);
        goda_throw_exception(E_ERROR, "View render fiald");
        RETURN_FALSE;
    }

    zval *response_ptr = zend_read_property(goda_controller_ce, getThis(), ZEND_STRL(GODA_CONTROLLER_RESPONSE), 1, NULL);
    goda_response_send(response_ptr, &view_result);
    zval_ptr_dtor(&view_result);
    zval_ptr_dtor(&render_val);
    RETURN_TRUE;
}

ZEND_METHOD(goda_controller, renderText) {
    zend_string *str;
    zval text;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(str)
    ZEND_PARSE_PARAMETERS_END();
    
    zval *response_ptr = zend_read_property(goda_controller_ce, getThis(), ZEND_STRL(GODA_CONTROLLER_RESPONSE), 1, NULL);
    goda_response_str_send(response_ptr, str);
    RETURN_TRUE;
}

ZEND_METHOD(goda_controller, renderJson) {
    zval *array, json_ecode, retval, params[2];
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(array)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_STRING(&json_ecode, "json_encode");
    
    ZVAL_ZVAL(&params[0], array, 1, 0);
    ZVAL_LONG(&params[1], 1<<8);

    ZVAL_NULL(&retval);
    if (call_user_function(EG(function_table), NULL, &json_ecode, &retval, 2, params) == SUCCESS) {
        zval *response_ptr = zend_read_property(goda_controller_ce, getThis(), ZEND_STRL(GODA_CONTROLLER_RESPONSE), 1, NULL);
        goda_response_send(response_ptr, &retval);
    } 
    zval_ptr_dtor(&json_ecode);
    zval_ptr_dtor(&retval);
    zval_ptr_dtor(&params[0]);
    zval_ptr_dtor(&params[1]);
}

ZEND_METHOD(goda_controller, redirect) {
    zend_string *str;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(str)
    ZEND_PARSE_PARAMETERS_END();

    zval *response_ptr = zend_read_property(goda_controller_ce, getThis(), ZEND_STRL(GODA_CONTROLLER_RESPONSE), 1, NULL);

    zend_string *location = zend_string_init("Location", sizeof("Location")-1, 0);
    goda_response_set_header(response_ptr, location, str);
    goda_response_str_send(response_ptr, NULL);
    zend_string_release(location);
    RETURN_TRUE;
}

zend_function_entry goda_controller_methods[] = {
    ZEND_ME(goda_controller, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(goda_controller, get, goda_controller_get_arg, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_controller, assgin, goda_controller_assgin_arg, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_controller, render, goda_controller_render_arg, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_controller, renderText, goda_controller_render_text_arg, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_controller, renderJson, goda_controller_render_json_arg, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_controller, redirect, goda_controller_render_text_arg, ZEND_ACC_PUBLIC)
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