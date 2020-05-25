#include "php_goda.h"
#include "Zend/zend_inheritance.h"

#include "ext/json/php_json.h"

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
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(key)
    ZEND_PARSE_PARAMETERS_END();

    zval *request_ptr = zend_read_property(goda_controller_ce, getThis(), ZEND_STRL(GODA_CONTROLLER_REUQEST), 1, NULL);
    zval *param = goda_request_get_param(request_ptr, key);
    if (param) {
        RETURN_ZVAL(param, 1, 0);
    }
    RETURN_EMPTY_STRING();
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
    zval *val = NULL, *assgin, *value;
    zval view_result, render_val;
    zend_bool saveData = 1;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|ab", &filename, &val, &saveData) == FAILURE) {
		return;
	}

    ZVAL_UNDEF(&view_result);
    ZVAL_UNDEF(&render_val);
    if (saveData) {
        assgin = zend_read_property(goda_controller_ce, getThis(), ZEND_STRL(GODA_CONTROLLER_ASSGIN), 1, NULL);
        
        if (val && IS_ARRAY == Z_TYPE_P(val) && zend_hash_num_elements(Z_ARRVAL_P(val)) > 0) {
            zend_hash_merge(Z_ARRVAL_P(assgin), Z_ARRVAL_P(val), zval_add_ref, 1);
        }

        ZVAL_COPY_VALUE(&render_val, assgin);
    } else {
        if (val) {
            ZVAL_COPY_VALUE(&render_val, val);
        }
    }

    
    if (Z_TYPE(render_val) == IS_UNDEF) {
        array_init(&render_val);
    }

    if (goda_view_render(filename, &render_val, &view_result) == 0) {
        zval_ptr_dtor(&view_result);
        zval_ptr_dtor(&render_val);
        goda_throw_exception(E_ERROR, "View render fiald");
        RETURN_EMPTY_STRING();
    }
    zval_ptr_dtor(&render_val);
    RETURN_ZVAL(&view_result, 1, 1);
}

ZEND_METHOD(goda_controller, renderText) {
    zend_string *str;
    zval text;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(str)
    ZEND_PARSE_PARAMETERS_END();
    
    RETURN_STR(str);
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
    if (call_user_function(EG(function_table), NULL, &json_ecode, &retval, 2, params) == FAILURE) {
        goda_throw_exception(E_ERROR, "call json_encode fiald");
    } 
    zval_ptr_dtor(&json_ecode);
    // zval_ptr_dtor(&retval);
    zval_ptr_dtor(&params[0]);
    zval_ptr_dtor(&params[1]);

    RETURN_ZVAL(&retval, 1, 1);

    // JSON_G(error_code) = PHP_JSON_ERROR_NONE;
    // JSON_G(encode_max_depth) = 512;

    // smart_str buf = { 0 };

    // // php_json_encode(&buf, array, PHP_JSON_UNESCAPED_UNICODE);

    // if (JSON_G(error_code) != PHP_JSON_ERROR_NONE && !(PHP_JSON_UNESCAPED_UNICODE & PHP_JSON_PARTIAL_OUTPUT_ON_ERROR))
    // {
    //     smart_str_free(&buf);
    //     RETURN_FALSE;
    // }
    // else
    // {
    //     smart_str_0(&buf);
    //     zval *response_ptr = zend_read_property(goda_controller_ce, getThis(), ZEND_STRL(GODA_CONTROLLER_RESPONSE), 1, NULL);
    //     goda_response_str_send(response_ptr, buf.s);
    //     smart_str_free(&buf);
    // }
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