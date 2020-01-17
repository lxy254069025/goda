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

zend_string *goda_request_get_base_uri() {
	zval *uri;
	zend_string *base_uri = NULL;
	uri = goda_request_get_server(ZEND_STRL("REQUEST_URI"));
	char *pos;
	if ((pos = strstr(Z_STRVAL_P(uri), "?"))) {
		base_uri = zend_string_init(Z_STRVAL_P(uri), pos - Z_STRVAL_P(uri), 0);
	} else {
		base_uri = zend_string_dup(Z_STR_P(uri), 0);
	}

	return base_uri;
}


int goda_request_regexp_match_uri(zval *this_ptr, zend_string *path, zend_string *uri) {
    zval params;

    array_init(&params);
    smart_str pattern = {0};
	pcre_cache_entry *pce_regexp;
	smart_str_appendl(&pattern, "#^", sizeof("#^") - 1);
	char *p = ZSTR_VAL(path);
	size_t plen = ZSTR_LEN(path);
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
                zend_hash_update(Z_ARRVAL(params), key, pval);
            }
        }ZEND_HASH_FOREACH_END();
        zval_ptr_dtor(&subparts);
        goda_request_set_params(this_ptr, &params);
        zval_ptr_dtor(&params);

        return 1;
	}
	return 0;
}

/*设置参数*/
void goda_request_set_params(zval *this_ptr, zval *params) {
    zend_update_property(goda_request_ce, this_ptr, ZEND_STRL(GODA_REQUEST_PARAMS), params);
}

zval *goda_request_get_params(zval *this_ptr, zend_string *key) {
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

    zval *params = goda_request_get_params(getThis(), key);
    if (params) {
        RETURN_ZVAL(params, 1, 0);
    }
    RETURN_STRING(ret);
}

ZEND_METHOD(goda_request, method) {
    RETURN_STRING(goda_request_get_method());
}

ZEND_METHOD(goda_request, uri) {
    RETURN_STR(goda_request_get_base_uri());
}

zend_function_entry goda_request_methods[] = {
    ZEND_ME(goda_request, get, goda_request_get_arg, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_request, method, goda_request_void_arg, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_request, uri, goda_request_void_arg, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

GODA_MINIT_FUNCTION(request) {
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Goda\\Request", goda_request_methods);
    goda_request_ce = zend_register_internal_class_ex(&ce, NULL);
    goda_request_ce->ce_flags |= ZEND_ACC_FINAL;
    zend_declare_property_null(goda_request_ce, ZEND_STRL(GODA_REQUEST_PARAMS), ZEND_ACC_PUBLIC);

    return SUCCESS;
}