#include "php_goda.h"
#include "Zend/zend_inheritance.h"

#include "goda_controller.h"
#include "goda_view.h"
#include "goda_response.h"
#include "goda_request.h"

zend_class_entry *goda_controller_ce;

/**
 * handle Index@Index 
 * router_ptr is router object.
 * request is request object. 
 */
void goda_controller_call_method(zval *handle, zval *router_ptr, zval *request_ptr) {
    char *controller, *action;
    
    if (Z_TYPE_P(handle) == IS_STRING && Z_STRLEN_P(handle) > 0) {
        char *p = Z_STRVAL_P(handle);
        char *pos;
        if ((pos = strchr(p,'@'))) {
            controller = estrndup(p,  Z_STRLEN_P(handle)- strlen(pos));
            while (*pos == '@') {
                pos++;
            }
            action = estrdup(pos);
            zend_str_tolower(action, strlen(action));
        } else {
            zend_error_noreturn(E_ERROR, "Couldn't find controller and action");
            return;
        }

        zend_string *class_controller = strpprintf(0, "controllers\\%s", controller);
        zend_class_entry *ce;
        ce = zend_lookup_class(class_controller);
        zend_string_release(class_controller);
        if (ce) {
            zval controller_ptr;
            object_init_ex(&controller_ptr, ce);
            zend_str_tolower(action, strlen(action));
            if (instanceof_function(ce, goda_controller_ce)) {
                 /* call method construct */
                zend_call_method_with_0_params(&controller_ptr, ce, &ce->constructor, "__construct", NULL);
                /* call init*/
                zend_call_method_with_0_params(&controller_ptr, ce, NULL, "init", NULL);
                zend_update_property(ce, &controller_ptr, ZEND_STRL(GODA_CONTROLLER_REUQEST), request_ptr);
            }
            
            zend_call_method(&controller_ptr, ce, NULL, action, strlen(action), NULL, 0, NULL, NULL);
            zval_ptr_dtor(&controller_ptr);
        } else {
            zend_error_noreturn(E_ERROR, "Couldn't find controller and action");
        }

        efree(controller);
        efree(action);
    }
}

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
    
}

ZEND_METHOD(goda_controller, init) {
    zval assgin, params;
    array_init(&assgin);
    zend_update_property(goda_controller_ce, getThis(), ZEND_STRL(GODA_CONTROLLER_ASSGIN), &assgin);
    zval_ptr_dtor(&assgin);
}

ZEND_METHOD(goda_controller, get) {
    zend_string *key;
    char *ret = "";
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(key)
    ZEND_PARSE_PARAMETERS_END();

    zval *request_ptr = zend_read_property(goda_controller_ce, getThis(), ZEND_STRL(GODA_CONTROLLER_REUQEST), 1, NULL);
    zval *params = goda_request_get_params(request_ptr, key);
    if (params) {
        RETURN_ZVAL(params, 1, 0);
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
    

    return SUCCESS;
}