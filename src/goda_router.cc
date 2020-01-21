#include "php_goda.h"
#include "goda_router.h"
#include "goda_loader.h"
#include "goda_controller.h"
#include "goda_request.h"
#include "goda_exception.h"
#include <iostream>
#include "router/router_trie.hpp"

zend_class_entry *goda_router_ce;
zend_object_handlers goda_router_handlers;

typedef struct {
    Goda::Router *tree;
    zend_object std;
}router_trie_t;

/**
 * 创建trie tree 
*/
static zend_object *goda_router_trie_create_object(zend_class_entry *ce) {
    router_trie_t *trie = (router_trie_t *) ecalloc(1, sizeof(router_trie_t) + zend_object_properties_size(ce));
    trie->tree = new Goda::Router();
    zend_object_std_init(&trie->std, ce);
    object_properties_init(&trie->std, ce);
    trie->std.handlers = &goda_router_handlers;
    return &trie->std;
}

/**
 * 查找函数
*/
static router_trie_t* goda_router_trie_fetch_object(zend_object *obj) {
    return (router_trie_t *) ((char *) obj - goda_router_handlers.offset);
}

/**
 * 释放
*/
static void goda_router_trie_free_object(zend_object *obj) {
    router_trie_t *ob = goda_router_trie_fetch_object(obj);
    delete ob->tree;
    zend_object_std_dtor(&ob->std);
}

void goda_router_instance(zval *this_ptr) {
    zval *pzval;
    zend_string *config_path, *key;

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
    if (goda_loader_include(config_path, &retval)) {
        if (Z_TYPE(retval) == IS_ARRAY) {
            ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL(retval), key, pzval) {
                if (key) {
                    zval *handle;
                    zend_string *pattern;
                    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(pzval), pattern, handle) {
                        if (pattern) {
                            if (Z_TYPE_P(handle) == IS_STRING) {
                                goda_router_add_route(this_ptr, key, pattern, Z_STR_P(handle));
                            } else {
                                zend_error(E_ERROR, "Route pattern %s must a be string", pattern->val);
                            }
                        }
                    }ZEND_HASH_FOREACH_END();
                }
            }ZEND_HASH_FOREACH_END();
        }
    }
    
    zval_ptr_dtor(&retval);
    zend_string_release(config_path);
}

void goda_router_add_route(zval *this_ptr, zend_string *method, zend_string *path, zend_string *handle) {
    router_trie_t *trie = goda_router_trie_fetch_object(Z_OBJ_P(this_ptr));

    auto router = trie->tree;
    zend_string *method_p = php_string_toupper(method);
    router->addRoute(method_p->val, path->val, handle->val);
    zend_string_release(method_p);
}

void goda_router_run(zval *this_ptr) {
    // zval *routes, *route, handle, *request_ptr;
    // zend_string *uri, *key;
    // const char *current_method = goda_request_get_method();
    // zend_bool flag = FAILURE;
    
    // ZVAL_UNDEF(&handle);
    // routes = zend_read_property(goda_router_ce, this_ptr, ZEND_STRL(GODA_ROUTER_ROUTES), 1, NULL);
    // request_ptr = zend_read_property(goda_router_ce, this_ptr, ZEND_STRL(GODA_ROUTER_REQUEST), 1, NULL);
    // uri = goda_request_get_base_uri();

    // ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(routes), key, route) {
    //     if (key) {
    //         const char *method = php_strtoupper(ZSTR_VAL(key), ZSTR_LEN(key));
    //         if (strcmp(method, current_method) == 0 || strcmp(method, "ANY") == 0) {
    //             zend_string *path;
    //             zval *pval;
    //             ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(route), path, pval) {
    //                 const char *path_str = ZSTR_VAL(path);
    //                 if (strcmp(path_str, ZSTR_VAL(uri)) == 0) {
    //                     flag = SUCCESS;
    //                     ZVAL_COPY_VALUE(&handle, pval);
    //                     break;
    //                 } else if (strchr(path_str, '{') != NULL && strchr(path_str, '}') != NULL) {
    //                     if (goda_request_regexp_match_uri(request_ptr, path, uri)) {
    //                         flag = SUCCESS;
    //                         ZVAL_COPY_VALUE(&handle, pval);
    //                         break;
    //                     }
    //                 }
    //             }ZEND_HASH_FOREACH_END();
    //         }
    //         if (flag == SUCCESS) {
    //             break;
    //         }
    //     }
    // } ZEND_HASH_FOREACH_END();

    // if (Z_TYPE(handle) == IS_UNDEF) {
    //     goda_throw_exception(E_ERROR, " 404"); //
    // } else {
    //     goda_controller_call_method(&handle, this_ptr, request_ptr);
    // }

    // zval_ptr_dtor(&handle);
    // zend_string_release(uri);
}

ZEND_BEGIN_ARG_INFO_EX(goda_router_addroute_arg, 0, 0, 3)
    ZEND_ARG_INFO(0, method)
    ZEND_ARG_INFO(0, path)
    ZEND_ARG_INFO(0, handle)
ZEND_END_ARG_INFO()

ZEND_METHOD(goda_router, addRoute) {
    zend_string *method, *path, *handle;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_STR(method);
        Z_PARAM_STR(path)
        Z_PARAM_STR(handle)
    ZEND_PARSE_PARAMETERS_END();
    goda_router_add_route(getThis(), method, path, handle);
}

zend_function_entry goda_router_methods[] = {
    ZEND_ME(goda_router, addRoute, goda_router_addroute_arg, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

GODA_MINIT_FUNCTION(router) {
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Goda\\Router", goda_router_methods);
    goda_router_ce = zend_register_internal_class(&ce);
    goda_router_ce->ce_flags |= ZEND_ACC_FINAL;

    memcpy(&goda_router_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    goda_router_ce->create_object = goda_router_trie_create_object;
    goda_router_handlers.free_obj = goda_router_trie_free_object;
    goda_router_handlers.offset = XtOffsetOf(router_trie_t, std);

    zend_declare_property_null(goda_router_ce, ZEND_STRL(GODA_ROUTER_REQUEST), ZEND_ACC_PUBLIC);
    
    return SUCCESS;
}