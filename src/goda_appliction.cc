#include "php_goda.h"
#include "goda_application.h"
#include "goda_router.h"
#include "goda_loader.h"
#include "goda_controller.h"
#include "goda_exception.h"
#include "goda_config.h"
#include "goda_request.h"

zend_class_entry *goda_application_ce;

ZEND_BEGIN_ARG_INFO_EX(goda_app_ctor_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, dir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(goda_app_set_object_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(goda_app_get_object_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(goda_app_run_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(goda_application, __construct) {
    zend_string *dir;
    zval *self = getThis();
    zval *app, router_object = {{0}}, config_object = {{0}}, request_ptr = {{0}};
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(dir)
    ZEND_PARSE_PARAMETERS_END();

    app = zend_read_static_property(goda_application_ce, ZEND_STRL(GODA_APPLICATION_APP), 1);
    
    if (!ZVAL_IS_NULL(app)) {
        zend_error(E_ERROR, "The application can be initialized");
        RETURN_FALSE;
    }
    GODA_G(app_dir) = php_trim(dir, (char *)"/", sizeof("/")-1, 2);
    goda_loader_instance();

    goda_router_instance(&router_object);
    zend_update_property(goda_application_ce, self, ZEND_STRL(GODA_APPLICATION_ROUTER), &router_object);
    zval_ptr_dtor(&router_object);

    goda_config_instance(&config_object);
    zend_update_property(goda_application_ce, self, ZEND_STRL(GODA_APPLICATION_CONDIG), &config_object);
    zval_ptr_dtor(&config_object);

    goda_request_instance(&request_ptr);
    zend_update_property(goda_application_ce, self, ZEND_STRL(GODA_APPLICATION_REQUEST), &request_ptr);
    zval_ptr_dtor(&request_ptr);

    zend_update_static_property(goda_application_ce, ZEND_STRL(GODA_APPLICATION_APP), self);

    zval objects;
    array_init(&objects);
    zend_update_property(goda_application_ce, self, ZEND_STRL(GODA_APPLICATION_OBJECTS), &objects);
    zval_ptr_dtor(&objects);
}

/**
 * 引导程序，没有设置request,response的话，会加载自己的。
 */
ZEND_METHOD(goda_application, bootstrap) {
    RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(goda_application, run) {
    zval *request_ptr = zend_read_property(goda_application_ce, getThis(), ZEND_STRL(GODA_APPLICATION_REQUEST), 1, NULL);
    if (ZVAL_IS_NULL(request_ptr)) {
        goda_throw_exception(E_ERROR, "Request is Uninitialized");
        RETURN_FALSE;
    }

    zval *router_ptr = zend_read_property(goda_application_ce, getThis(), ZEND_STRL(GODA_APPLICATION_ROUTER), 1, NULL);
    if (ZVAL_IS_NULL(router_ptr)) {
        goda_throw_exception(E_ERROR, "Router is Uninitialized");
        RETURN_FALSE;
    }

    if (goda_router_parase_routeing(router_ptr, request_ptr)) {
        goda_router_run(router_ptr, request_ptr);
    } else {
        goda_throw_exception(E_ERROR, "Routeing parase faild");
        RETURN_FALSE;
    }

}

ZEND_METHOD(goda_application, app) {
    zval *app = zend_read_static_property(goda_application_ce, ZEND_STRL(GODA_APPLICATION_APP), 1);
    RETURN_ZVAL(app, 1, 0);
}

ZEND_METHOD(goda_application, setObject) {
    zend_string *key;
    zval *object, new_obj;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(key)
        Z_PARAM_OBJECT(object)
    ZEND_PARSE_PARAMETERS_END();

    array_init(&new_obj);
    add_assoc_zval(&new_obj, ZSTR_VAL(key), object);
    zend_update_property(goda_application_ce, getThis(), ZEND_STRL(GODA_APPLICATION_OBJECTS), &new_obj);
    zval_ptr_dtor(&new_obj);
}

ZEND_METHOD(goda_application, getObject) {
    zend_string *key;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(key)
    ZEND_PARSE_PARAMETERS_END();

    zval *objects = zend_read_property(goda_application_ce, getThis(), ZEND_STRL(GODA_APPLICATION_OBJECTS), 1, NULL);
    
    zval *object = zend_hash_find(Z_ARRVAL_P(objects), key);
    if (object) {
        RETURN_ZVAL(object, 1, 0);
    }

    goda_throw_exception(E_ERROR, "Failed to not find %s object", key);
}

ZEND_METHOD(goda_application, getObjects) {
    zval *objects = zend_read_property(goda_application_ce, getThis(), ZEND_STRL(GODA_APPLICATION_OBJECTS), 1, NULL);
    RETURN_ZVAL(objects, 1, 0);
}

ZEND_METHOD(goda_application, __destruct) {
    zend_update_static_property_null(goda_application_ce, ZEND_STRL(GODA_APPLICATION_APP));
}

zend_function_entry goda_application_methods[] = {
    ZEND_ME(goda_application, __construct, goda_app_ctor_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    ZEND_ME(goda_application, bootstrap, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_application, run, goda_app_run_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_application, setObject, goda_app_set_object_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_application, getObject, goda_app_get_object_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(goda_application, getObjects, goda_app_run_arginfo, ZEND_ACC_PUBLIC)
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
    zend_declare_property_null(goda_application_ce, ZEND_STRL(GODA_APPLICATION_CONDIG), ZEND_ACC_PROTECTED);
    zend_declare_property_null(goda_application_ce, ZEND_STRL(GODA_APPLICATION_OBJECTS), ZEND_ACC_PUBLIC);
    zend_declare_property_null(goda_application_ce, ZEND_STRL(GODA_APPLICATION_APP), ZEND_ACC_STATIC | ZEND_ACC_PROTECTED);
    return SUCCESS;
}