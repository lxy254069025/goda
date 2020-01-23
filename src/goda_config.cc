#include "php_goda.h"
#include "goda_config.h"
#include "goda_loader.h"

zend_class_entry *goda_config_ce;

void goda_config_instance(zval *this_ptr) {
    zend_string *config_path;
    zval retval;

    if (Z_TYPE_P(this_ptr) == IS_UNDEF) {
        object_init_ex(this_ptr, goda_config_ce);
    }

    config_path = strpprintf(0, "%s/%s", ZSTR_VAL(GODA_G(app_dir)),"config/config.php");
    if (goda_loader_include(config_path, &retval)) {
        zend_update_property(goda_config_ce, this_ptr, ZEND_STRL(GODA_CONFIG_CONFIG), &retval);
        zval_ptr_dtor(&retval);
    }
    zend_string_release(config_path);
}

static void goda_config_add(zval *this_ptr, zend_string *key, zval *value) {
    zval *config = zend_read_property(goda_config_ce, this_ptr, ZEND_STRL(GODA_CONFIG_CONFIG), 1, NULL);

    zend_hash_update(Z_ARRVAL_P(config), key, value);
}

static zval *goda_config_get(zval *this_ptr, zend_string *key) {
    zval *config = zend_read_property(goda_config_ce, this_ptr, ZEND_STRL(GODA_CONFIG_CONFIG), 1, NULL);
    return zend_hash_find(Z_ARRVAL_P(config), key);
}

ZEND_BEGIN_ARG_INFO_EX(goda_config_construct_arg, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(goda_config_set_arg, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(goda_config_get_arg, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_METHOD(goda_config, __construct) {
    goda_config_instance(getThis());
}

ZEND_METHOD(goda_config, set) {
    zend_string *key;
    zval *value;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(key)
        Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END();
    goda_config_add(getThis(), key, value);
}

ZEND_METHOD(goda_config, get) {
    zend_string *key;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(key)
    ZEND_PARSE_PARAMETERS_END();

    RETURN_ZVAL(goda_config_get(getThis(), key), 1, 0);
}

zend_function_entry goda_config_methods[] = {
    ZEND_ME(goda_config, __construct, goda_config_construct_arg, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(goda_config, set, goda_config_set_arg, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_config, get, goda_config_get_arg, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

GODA_MINIT_FUNCTION(config) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Goda\\Config", goda_config_methods);
    goda_config_ce = zend_register_internal_class_ex(&ce, NULL);
    goda_config_ce->ce_flags |= ZEND_ACC_FINAL;

    zend_declare_property_null(goda_config_ce, ZEND_STRL(GODA_CONFIG_CONFIG), ZEND_ACC_PROTECTED);
    
    return SUCCESS;
}