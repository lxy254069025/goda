#include "php_goda.h"
#include "goda_response.h"

zend_class_entry *goda_response_ce;

void goda_response_instance(zval *this_ptr) {
    if (Z_TYPE_P(this_ptr) == IS_UNDEF) {
        object_init_ex(this_ptr, goda_response_ce);
    }
}

void goda_response_set_header(zval *this_ptr, zend_string *key, zend_string *value) {
    zval *header = zend_read_property(goda_response_ce, this_ptr, ZEND_STRL(GODA_RESPONSE_HEADER), 1, NULL);
    if (Z_TYPE_P(header) == IS_ARRAY) {
        add_assoc_str(header, ZSTR_VAL(key), value);
    } else {
        zval array;
        array_init(&array);
        add_assoc_str(&array, ZSTR_VAL(key), value);
        zend_update_property(goda_response_ce, this_ptr, ZEND_STRL(GODA_RESPONSE_HEADER), &array);
        zval_ptr_dtor(&array);
    }
}

void goda_response_str_send(zval *this_ptr, zend_string *str) {
    zval *header = zend_read_property(goda_response_ce, this_ptr, ZEND_STRL(GODA_RESPONSE_HEADER), 1, NULL);
    if (!ZVAL_IS_NULL(header)) {
        sapi_header_line ctr = {0};
        zval *val;
        zend_string *key;
        ulong idx;

        ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(header), idx, key, val) {
            if (key) {
                ctr.line_len    = spprintf(&(ctr.line), 0, "%s: %s", ZSTR_VAL(key), Z_STRVAL_P(val));
            } else {
                ctr.line_len    = spprintf(&(ctr.line), 0, "%lu: %s", idx, Z_STRVAL_P(val));
            }
            
            ctr.response_code   = 0;
            if (sapi_header_op(SAPI_HEADER_REPLACE, &ctr) == SUCCESS) {
                efree(ctr.line);
                return;
            }
        }ZEND_HASH_FOREACH_END();
        efree(ctr.line);
    }

    if (str) {
        php_write(ZSTR_VAL(str), ZSTR_LEN(str));
    }
}

void goda_response_send(zval *this_ptr, zval *body) {
    zval *header = zend_read_property(goda_response_ce, this_ptr, ZEND_STRL(GODA_RESPONSE_HEADER), 1, NULL);
    if (!ZVAL_IS_NULL(header)) {
        sapi_header_line ctr = {0};
        zval *val;
        zend_string *key;
        ulong idx;

        ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(header), idx, key, val) {
            if (key) {
                ctr.line_len    = spprintf(&(ctr.line), 0, "%s: %s", ZSTR_VAL(key), Z_STRVAL_P(val));
            } else {
                ctr.line_len    = spprintf(&(ctr.line), 0, "%lu: %s", idx, Z_STRVAL_P(val));
            }
            
            ctr.response_code   = 0;
            if (sapi_header_op(SAPI_HEADER_REPLACE, &ctr) == SUCCESS) {
                efree(ctr.line);
                return;
            }
        }ZEND_HASH_FOREACH_END();
        efree(ctr.line);
    }

    if (body) {
        php_write(Z_STRVAL_P(body), Z_STRLEN_P(body));
    }
}

ZEND_BEGIN_ARG_INFO_EX(goda_response_header_arg, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_METHOD(goda_response, setHeader) {

}

ZEND_METHOD(goda_response, getHeader) {

}

zend_function_entry goda_response_methods[] = {
    ZEND_ME(goda_response, setHeader, goda_response_header_arg, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_response, getHeader, NULL, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

GODA_MINIT_FUNCTION(response) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Goda\\Response", goda_response_methods);
    goda_response_ce = zend_register_internal_class(&ce);
    goda_response_ce->ce_flags |= ZEND_ACC_FINAL;

    zend_declare_property_null(goda_response_ce, ZEND_STRL(GODA_RESPONSE_HEADER), ZEND_ACC_PROTECTED);

    return SUCCESS;
}