#include "php_goda.h"
#include "goda_router.h"
#include "goda_loader.h"
#include "goda_controller.h"
#include "goda_request.h"

zend_class_entry *goda_router_ce;


void goda_router_instance(zval *this_ptr) {
    zval *config, request_ptr = {{0}};
    zend_string *config_path;

    if (Z_ISUNDEF_P(this_ptr)) {
        object_init_ex(this_ptr, goda_router_ce);
    }

    goda_request_instance(&request_ptr);
    zend_update_property(goda_router_ce, this_ptr, ZEND_STRL(GODA_ROUTER_REQUEST), &request_ptr);
    zval_ptr_dtor(&request_ptr);

    char *p = ZSTR_VAL(GODA_G(app_dir));
    if (*(p + strlen(p) - 1) == '/') {
        config_path = strpprintf(0, "%s%s", p, "config/route.php");
    } else {
        config_path = strpprintf(0, "%s/%s", p, "config/route.php");
    }

    zval retval;
    ZVAL_UNDEF(&retval);
    if (goda_loader_include(config_path, &retval)) {
        zend_update_property(goda_router_ce, this_ptr, ZEND_STRL(GODA_ROUTER_ROUTES), &retval);
    } 
    zval_ptr_dtor(&retval);
    zend_string_release(config_path);
}

void goda_router_add_route(zval *this_ptr, zend_string *method, zend_string *path, zend_string *handle) {
    zval *routes, route, *mzval;
	routes = zend_read_property(goda_router_ce, this_ptr, ZEND_STRL(GODA_ROUTER_ROUTES), 1, NULL);
	
	mzval = zend_hash_find(Z_ARRVAL_P(routes), method);

	if (mzval) {
        add_assoc_str_ex(mzval, ZSTR_VAL(path), ZSTR_LEN(path), handle);
	} else {
		array_init(&route);
		add_assoc_str_ex(&route, ZSTR_VAL(path), ZSTR_LEN(path), handle);
		zend_hash_update(Z_ARRVAL_P(routes), method, &route);
	}
}

void goda_router_run(zval *this_ptr) {
    zval *routes, *route, handle, *request_ptr;
    zend_string *uri, *key;
    const char *current_method = goda_request_get_method();
    zend_bool flag = FAILURE;

    ZVAL_NULL(&handle);
    routes = zend_read_property(goda_router_ce, this_ptr, ZEND_STRL(GODA_ROUTER_ROUTES), 1, NULL);
    request_ptr = zend_read_property(goda_router_ce, this_ptr, ZEND_STRL(GODA_ROUTER_REQUEST), 1, NULL);
    uri = goda_request_get_base_uri();

    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(routes), key, route) {
        if (key) {
            const char *method = php_strtoupper(ZSTR_VAL(key), ZSTR_LEN(key));
            if (strcmp(method, current_method) == 0 || strcmp(method, "ANY") == 0) {
                zend_string *path;
                zval *pval;
                ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(route), path, pval) {
                    const char *path_str = ZSTR_VAL(path);
                    if (strcmp(path_str, ZSTR_VAL(uri)) == 0) {
                        flag = SUCCESS;
                        ZVAL_COPY_VALUE(&handle, pval);
                        break;
                    } else if (strchr(path_str, '{') != NULL && strchr(path_str, '}') != NULL) {
                        if (goda_request_regexp_match_uri(request_ptr, path, uri)) {
                            flag = SUCCESS;
                            ZVAL_COPY_VALUE(&handle,pval);
                            break;
                        }
                    }
                }ZEND_HASH_FOREACH_END();
            }
            if (flag == SUCCESS) {
                break;
            }
        }
    } ZEND_HASH_FOREACH_END();


    if (Z_TYPE(handle) == IS_NULL) {
        zend_error(E_ERROR, " 404");
    } else {
        goda_controller_call_method(&handle, this_ptr, request_ptr);
    }
    zval_ptr_dtor(&handle);
    zend_string_release(uri);
}

ZEND_BEGIN_ARG_INFO_EX(goda_router_addroute_arg, 0, 0, 3)
    ZEND_ARG_INFO(0, method)
    ZEND_ARG_INFO(0, path)
    ZEND_ARG_INFO(0, handle)
ZEND_END_ARG_INFO()

ZEND_METHOD(goda_router, addRoute) {
    char *method;
    size_t mlen;
    zend_string *path, *handle , *method_up;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_STRING(method, mlen);
        Z_PARAM_STR(path)
        Z_PARAM_STR(handle)
    ZEND_PARSE_PARAMETERS_END();

    method = php_strtoupper(method, mlen);

    method_up = zend_string_init(method, mlen, 0);
    goda_router_add_route(getThis(), method_up, path, handle);
    zend_string_release(method_up);
}

zend_function_entry goda_router_methods[] = {
    ZEND_ME(goda_router, addRoute, goda_router_addroute_arg, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

GODA_MINIT_FUNCTION(router){
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Goda\\Router", goda_router_methods);
    goda_router_ce = zend_register_internal_class(&ce);
    goda_router_ce->ce_flags |= ZEND_ACC_FINAL;
    zend_declare_property_null(goda_router_ce, ZEND_STRL(GODA_ROUTER_ROUTES), ZEND_ACC_PUBLIC); /*  */
    zend_declare_property_null(goda_router_ce, ZEND_STRL(GODA_ROUTER_REQUEST), ZEND_ACC_PUBLIC);
    
    return SUCCESS;
}