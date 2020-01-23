#include "php_goda.h"
#include "goda_request.h"

zend_class_entry *goda_request_ce;

void goda_request_instance(zval *this_ptr) {
    if (Z_TYPE_P(this_ptr) == IS_UNDEF) {
        object_init_ex(this_ptr, goda_request_ce);
    }
}

zval *goda_request_get_server(char *key, size_t len) {
    zval *ret = NULL;
    if (PG(auto_globals_jit)) {
        zend_is_auto_global_str(ZEND_STRL("_SERVER"));
    }
    zval *_server = &PG(http_globals)[TRACK_VARS_SERVER];
    ret = zend_hash_str_find(Z_ARRVAL_P(_server),key, len);
    return ret;
}


const char *goda_request_get_method() {
    if (SG(request_info).request_method) {
        return SG(request_info).request_method;
    } else if (strncasecmp(sapi_module.name, "cli", 3) == 0) {
        return "CLI";
    } else {
        return "UNKNOW";
    }
}

zend_string *goda_request_get_base_uri(zval *this_ptr) {
	zval *uri;
	zend_string *base_uri = NULL;

    uri = zend_read_property(goda_request_ce, this_ptr, ZEND_STRL(GODA_REQUEST_URI), 1, NULL);
    if (ZVAL_IS_NULL(uri)) {
        uri = goda_request_get_server(ZEND_STRL("REQUEST_URI"));
        zend_update_property(goda_request_ce, this_ptr, ZEND_STRL(GODA_REQUEST_URI), uri);
    }
	
	char *pos;
	if ((pos = strstr(Z_STRVAL_P(uri), "?"))) {
		base_uri = zend_string_init(Z_STRVAL_P(uri), pos - Z_STRVAL_P(uri), 0);
	} else {
		base_uri = zend_string_dup(Z_STR_P(uri), 0);
	}

	return base_uri;
}

static void goda_request_set_uri(zval *this_ptr, zend_string *uri) {
    zend_update_property_str(goda_request_ce, this_ptr, ZEND_STRL(GODA_REQUEST_URI), uri);
}

/*设置参数*/
void goda_request_set_params(zval *this_ptr, zval *params) {
    zend_update_property(goda_request_ce, this_ptr, ZEND_STRL(GODA_REQUEST_PARAMS), params);
}

zval *goda_request_get_params(zval *this_ptr) {
    return zend_read_property(goda_request_ce, this_ptr, ZEND_STRL(GODA_REQUEST_PARAMS), 1, NULL);
}

zval *goda_request_get_param(zval *this_ptr, zend_string *key) {
    zval *params = zend_read_property(goda_request_ce, this_ptr, ZEND_STRL(GODA_REQUEST_PARAMS), 1, NULL);

    if (ZSTR_LEN(key) && Z_TYPE_P(params) == IS_ARRAY) {
        zval *result = zend_hash_find(Z_ARRVAL_P(params), key);
        if (result) {
            return result;
        }
    }

    return NULL;
}

ZEND_BEGIN_ARG_INFO_EX(goda_request_get_arg, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(goda_request_void_arg, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(goda_request, get) {
    zend_string *key;
    char *ret = "";
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(key)
    ZEND_PARSE_PARAMETERS_END();

    zval *param = goda_request_get_param(getThis(), key);
    if (param) {
        RETURN_ZVAL(param, 1, 0);
    }
    RETURN_STRING(ret);
}

ZEND_METHOD(goda_request, setUrl) {
    zend_string *uri;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(uri)
    ZEND_PARSE_PARAMETERS_END();

    if (ZSTR_LEN(uri) > 0) {
        zend_update_property_str(goda_request_ce, getThis(), ZEND_STRL(GODA_REQUEST_URI), uri);
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

ZEND_METHOD(goda_request, method) {
    RETURN_STRING(goda_request_get_method());
}

ZEND_METHOD(goda_request, getUrl) {
    RETURN_STR(goda_request_get_base_uri(getThis()));
}

zend_function_entry goda_request_methods[] = {
    ZEND_ME(goda_request, get, goda_request_get_arg, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_request, method, goda_request_void_arg, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_request, getUrl, goda_request_void_arg, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_request, setUrl, goda_request_get_arg, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

GODA_MINIT_FUNCTION(request) {
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Goda\\Request", goda_request_methods);
    goda_request_ce = zend_register_internal_class_ex(&ce, NULL);
    goda_request_ce->ce_flags |= ZEND_ACC_FINAL;

    zend_declare_property_null(goda_request_ce, ZEND_STRL(GODA_REQUEST_PARAMS), ZEND_ACC_PUBLIC);
    zend_declare_property_null(goda_request_ce, ZEND_STRL(GODA_REQUEST_URI), ZEND_ACC_PUBLIC);
    return SUCCESS;
}