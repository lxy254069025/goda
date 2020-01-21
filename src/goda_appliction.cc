#include "php_goda.h"
#include "goda_application.h"
#include "goda_router.h"
#include "goda_loader.h"
#include "goda_controller.h"
#include "goda_exception.h"

zend_class_entry *goda_application_ce;

ZEND_BEGIN_ARG_INFO_EX(goda_app_ctor_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, dir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(goda_app_run_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(goda_application, __construct) {
    zend_string *dir;
    zval *self = getThis();
    zval *app, router_object = {{0}};
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(dir)
    ZEND_PARSE_PARAMETERS_END();

    app = zend_read_static_property(goda_application_ce, ZEND_STRL(GODA_APPLICATION_APP), 1);
    
    if (!ZVAL_IS_NULL(app)) {
        zend_error(E_ERROR, "The application can be initialized");
        RETURN_FALSE;
    }


    GODA_G(app_dir) = php_trim(dir, "/", 1, 2);
    goda_loader_instance();

    goda_router_instance(&router_object);
    zend_update_property(goda_application_ce, self, ZEND_STRL(GODA_APPLICATION_ROUTER), &router_object);
    zval_ptr_dtor(&router_object);
    zend_update_static_property(goda_application_ce, ZEND_STRL(GODA_APPLICATION_APP), self);
}

/**
 * 兼容swoole, 1、$request 2、$response
 */
ZEND_METHOD(goda_application, swoole) {
    zval *request, *response;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_OBJECT(request)
        Z_PARAM_OBJECT(response)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property(goda_application_ce, getThis(), ZEND_STRL(GODA_APPLICATION_REQUEST), request);
    zend_update_property(goda_application_ce, getThis(), ZEND_STRL(GODA_APPLICATION_RESPONSE), response);
    RETURN_ZVAL(getThis(), 1, 0);
}

/**
 * 引导程序，没有设置request,response的话，会加载自己的。
 */
ZEND_METHOD(goda_application, bootstrap) {

    RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(goda_application, run) {
    // zval *router_ptr = zend_read_property(goda_application_ce, getThis(), ZEND_STRL(GODA_APPLICATION_ROUTER), 1, NULL);
    // if (Z_TYPE_P(router_ptr) == IS_OBJECT) {
    //     goda_router_run(router_ptr);
    // } else {
    //     zend_error(E_ERROR, "Router initialization failed!");
    //     RETURN_FALSE;
    // }
    // RETURN_TRUE;
}

ZEND_METHOD(goda_application, app) {
    Goda::Zval app = zend_read_static_property(goda_application_ce, ZEND_STRL(GODA_APPLICATION_APP), 1);
    RETURN_ZVAL(app, 1, 0);
}

ZEND_METHOD(goda_application, __destruct) {
    zend_update_static_property_null(goda_application_ce, ZEND_STRL(GODA_APPLICATION_APP));
}

zend_function_entry goda_application_methods[] = {
    ZEND_ME(goda_application, __construct, goda_app_ctor_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    ZEND_ME(goda_application, swoole, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_application, bootstrap, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_application, run, goda_app_run_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_application, app, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    ZEND_ME(goda_application, __destruct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
    ZEND_FE_END
};

GODA_MINIT_FUNCTION(application) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Goda\\Application", goda_application_methods);
    goda_application_ce = zend_register_internal_class(&ce);
    goda_application_ce->ce_flags |= ZEND_ACC_FINAL;

    zend_declare_property_null(goda_application_ce, ZEND_STRL(GODA_APPLICATION_ROUTER), ZEND_ACC_PROTECTED);
    zend_declare_property_null(goda_application_ce, ZEND_STRL(GODA_APPLICATION_REQUEST), ZEND_ACC_PROTECTED);
    zend_declare_property_null(goda_application_ce, ZEND_STRL(GODA_APPLICATION_RESPONSE), ZEND_ACC_PROTECTED);
    zend_declare_property_null(goda_application_ce, ZEND_STRL(GODA_APPLICATION_APP), ZEND_ACC_STATIC | ZEND_ACC_PROTECTED);
    return SUCCESS;
}