#include "php_goda.h"
#include "goda_router.h"
#include "goda_loader.h"
#include "goda_controller.h"


zend_class_entry *goda_router_ce;


zval *goda_router_request_get_server(char *key, size_t len) {
    zval *ret = NULL;
    if (PG(auto_globals_jit)) {
        zend_is_auto_global_str(ZEND_STRL("_SERVER"));
    }
    zval *_server = &PG(http_globals)[TRACK_VARS_SERVER];
    ret = zend_hash_str_find(Z_ARRVAL_P(_server),key, len);
    return ret;
}


const char *goda_router_request_get_method() {
    if (SG(request_info).request_method) {
        return SG(request_info).request_method;
    } else if (strncasecmp(sapi_module.name, "cli", 3) == 0) {
        return "CLI";
    } else {
        return "UNKNOW";
    }
}

zend_string *goda_router_get_base_uri() {
	zval *uri;
	zend_string *base_uri = NULL;
	uri = goda_router_request_get_server(ZEND_STRL("REQUEST_URI"));
	char *pos;
	if ((pos = strstr(Z_STRVAL_P(uri), "?"))) {
		base_uri = zend_string_init(Z_STRVAL_P(uri), pos - Z_STRVAL_P(uri), 0);
	} else {
		base_uri = zend_string_copy(Z_STR_P(uri));
	}

	return base_uri;
}

void goda_router_instance(zval *this_ptr) {
    zval *config;
    zend_string *config_path;

    if (Z_ISUNDEF_P(this_ptr)) {
        object_init_ex(this_ptr, goda_router_ce);
    }

    char *p = ZSTR_VAL(GODA_G(app_dir));
    if (*(p + strlen(p) - 1) == '/') {
        config_path = strpprintf(0, "%s%s", p, "config/route.php");
    } else {
        config_path = strpprintf(0, "%s/%s", p, "config/route.php");
    }

    zval retval;
    ZVAL_UNDEF(&retval);
    if (goda_loader_include(config_path, &retval)) {
        zend_update_property(goda_router_ce, this_ptr, ZEND_STRL("routes"), &retval);
    } 
    zval_ptr_dtor(&retval);
    zend_string_release(config_path);
    
}

void goda_router_add_route(zval *this_ptr, zend_string *method, zend_string *path, zend_string *handle) {
    zval *routes, route, *mzval;
	routes = zend_read_property(goda_router_ce, this_ptr, ZEND_STRL("routes"), 1, NULL);
	
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
    zval *routes, *route , callback = {{0}}, params;
    zend_string *uri, *key;
    const char *method = goda_router_request_get_method();
    int flag = 0;

    uri = goda_router_get_base_uri();
    routes = zend_read_property(goda_router_ce, this_ptr, ZEND_STRL("routes"), 1, NULL);
    array_init(&params);
    // ZVAL_NULL(&callback);

    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(routes), key, route) {
        if (key) {
            char *md = php_strtoupper(ZSTR_VAL(key), ZSTR_LEN(key));
            if (strcmp(md, method) == 0 || strcmp(md, "ANY") == 0) {
                zend_string *rkey;
                zval *rval;
                ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(route), rkey, rval) {
                    if (rkey) {
                        char *rk = ZSTR_VAL(rkey);
                        if (strcmp(rk, ZSTR_VAL(uri)) == 0) {
                            flag = 1;
                            ZVAL_COPY_VALUE(&callback,rval);
                            break;
                        } else if(strchr(rk, '{') != NULL && strchr(rk, '}') != NULL) {
                            
                            if (goda_router_match_pce_regexp(this_ptr, rkey, uri, &params) ){
                                flag = 1;
                                ZVAL_COPY_VALUE(&callback,rval);
                                break;
                            }
                            // zval_ptr_dtor(&params);
                        }
                    }
                }ZEND_HASH_FOREACH_END();
            }
        }
        if (flag) {
            break;
        }
    }ZEND_HASH_FOREACH_END();
    
    if (Z_TYPE(callback) == IS_UNDEF) {
        zend_error(E_ERROR, " 404");
    } else {
        goda_controller_call_method(&callback, this_ptr, &params);
        zval_ptr_dtor(&callback);
    }

    zval_ptr_dtor(&params);
    zend_string_release(uri);
}

int goda_router_match_pce_regexp(zval *this_ptr, zend_string *key, zend_string *uri, zval *params) {
	smart_str pattern = {0};
	pcre_cache_entry *pce_regexp;
	smart_str_appendl(&pattern, "#^", sizeof("#^") - 1);
	char *p = ZSTR_VAL(key);
	size_t plen = ZSTR_LEN(key);
	for (size_t i = 0; i < plen; i++) {
		if (p[i] == '{') {
			smart_str_appendl(&pattern, "(?P<", sizeof("(?P<")-1);
		} else if (p[i] == '}') {
			smart_str_appendl(&pattern, ">.+?)", sizeof(">.+?)")-1);
		} else {
			smart_str_appendc(&pattern, p[i]);
		}
	}

	smart_str_appendc(&pattern, '#');
	smart_str_appendc(&pattern, 'i');
    
	smart_str_0(&pattern);
	if ((pce_regexp = pcre_get_compiled_regex_cache(pattern.s)) == NULL) {
		smart_str_free(&pattern);
		return 0;
	} else {
        smart_str_free(&pattern);
		zval matches, subparts;

		ZVAL_NULL(&subparts);
		php_pcre_match_impl(pce_regexp, ZSTR_VAL(uri), ZSTR_LEN(uri), &matches, &subparts, 0, 0, 0, 0);

        if (!zend_hash_num_elements(Z_ARRVAL(subparts))) {
            zval_ptr_dtor(&subparts);
            return 0;
        }

        zval *pval;
        zend_string *key;
        ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL(subparts), key, pval) {
            if (key) {
                Z_ADDREF_P(pval);
                zend_hash_update(Z_ARRVAL_P(params), key, pval);
            }
        }ZEND_HASH_FOREACH_END();
        zval_ptr_dtor(&subparts);
        return 1;
	}
	return 0;
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
    zend_declare_property_null(goda_router_ce, ZEND_STRL("routes"), ZEND_ACC_PUBLIC); /*  */
    zend_declare_property_null(goda_router_ce, ZEND_STRL("params"), ZEND_ACC_PUBLIC);
    return SUCCESS;
}