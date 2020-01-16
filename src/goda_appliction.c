#include "php_goda.h"
#include "goda_application.h"
#include "goda_router.h"
#include "goda_loader.h"
#include "goda_controller.h"

zend_class_entry *goda_application_ce;

ZEND_BEGIN_ARG_INFO_EX(goda_app_ctor_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, dir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(goda_app_run_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(goda_application, __construct) {
    zend_string *dir;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(dir)
    ZEND_PARSE_PARAMETERS_END();

    GODA_G(app_dir) = php_trim(dir, "/", 1, 2);

    zval router_ptr = {{0}};
    goda_router_instance(&router_ptr);

    if (Z_TYPE(router_ptr) == IS_OBJECT) {
        zend_update_property(goda_application_ce, getThis(), ZEND_STRL("router"), &router_ptr);
    } else {

    }
    zend_string_release(dir);
    zval_ptr_dtor(&router_ptr);
    goda_loader_instance();
}

ZEND_METHOD(goda_application, run) {
    zval *router_ptr = zend_read_property(goda_application_ce, getThis(), ZEND_STRL("router"), 1, NULL);
    if (Z_TYPE_P(router_ptr) == IS_OBJECT) {
        goda_router_run(router_ptr);
    } else {
        zend_error(E_ERROR, "Router initialization failed!");
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

zend_function_entry goda_application_methods[] = {
    ZEND_ME(goda_application, __construct, goda_app_ctor_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    ZEND_ME(goda_application, run, goda_app_run_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    ZEND_FE_END
};

GODA_MINIT_FUNCTION(application) {
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Goda\\Application", goda_application_methods);
    goda_application_ce = zend_register_internal_class(&ce);
    zend_declare_property_null(goda_application_ce, ZEND_STRL("router"), ZEND_ACC_PUBLIC);
    return SUCCESS;
}