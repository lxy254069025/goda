#include "php_goda.h"
#include "goda_router.h"
#include "goda_loader.h"
#include "goda_controller.h"
#include "goda_request.h"
#include "goda_exception.h"

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

int goda_router_parase_routeing(zval *this_ptr, zval *request_ptr) {
    zend_string *uri = goda_request_get_base_uri(request_ptr);
    const char *method = goda_request_get_method();

    router_trie_t *trie = goda_router_trie_fetch_object(Z_OBJ_P(this_ptr));
    auto tree = trie->tree;

    auto route = tree->getRoute(method, ZSTR_VAL(uri));
    zend_string_release(uri);

    Goda::Node *node;
    std::map<std::string, std::string> params;
    std::tie(node, params) = route;

    if (node != nullptr) {
        auto handle = tree->getHandler(method, node->m_pattern);

        if (handle.length() > 0) {
            auto controller_action = tree->parsePattern(handle, "@");
            
            if (controller_action.size() > 1) {
                const char *controller = controller_action[0].c_str();
                const char *action =  controller_action[1].c_str();
                zend_update_property_string(goda_router_ce, this_ptr, ZEND_STRL(GODA_ROUTER_CONTROLLER), controller);
                zend_update_property_string(goda_router_ce, this_ptr, ZEND_STRL(GODA_ROUTER_ACTION), action);
                zval request_params;
                array_init(&request_params);

                for (auto param : params) {
                    add_assoc_string(&request_params, param.first.c_str(), param.second.c_str());
                }
                goda_request_set_params(request_ptr, &request_params);
                zval_ptr_dtor(&request_params);
                return 1;
            } else {
                goda_throw_exception(E_ERROR, "Controller is incomplete");
                return 0;
            }
        } else {
            goda_throw_exception(E_ERROR, "Controller cannot be empty");
            return 0;
        }
    } else {
        /**设置404**/
        SG(sapi_headers).http_response_code = 404;
        auto handle = tree->getHandler("ERROR", "notfound");
        if (handle.length() > 0) {
            auto controller_action = tree->parsePattern(handle, "@");
            
            if (controller_action.size() > 1) {
                const char *controller = controller_action[0].c_str();
                const char *action =  controller_action[1].c_str();
                zend_update_property_string(goda_router_ce, this_ptr, ZEND_STRL(GODA_ROUTER_CONTROLLER), controller);
                zend_update_property_string(goda_router_ce, this_ptr, ZEND_STRL(GODA_ROUTER_ACTION), action);
            } else {
                goda_throw_exception(E_ERROR, "Error controller is incomplete");
                return 0;
            }
        } else {
            goda_throw_exception(E_ERROR, "Error controller cannot be empty");
            return 0;
        }
        return 1;
    }
}

void goda_router_run(zval *this_ptr, zval *request_ptr) {
    zval *controller, *action, *status, *params;
    
    controller = zend_read_property(goda_router_ce, this_ptr, ZEND_STRL(GODA_ROUTER_CONTROLLER), 1, NULL);
    action = zend_read_property(goda_router_ce, this_ptr, ZEND_STRL(GODA_ROUTER_ACTION), 1, NULL);
    
    zend_string *action_name = zend_string_tolower(Z_STR_P(action));
    zend_string *class_controller = strpprintf(0, "controllers\\%s", Z_STRVAL_P(controller));
    zend_class_entry *ce;
    ce = zend_lookup_class(class_controller);
    zend_string_release(class_controller);
    if (ce) {
        zval controller_ptr;
        object_init_ex(&controller_ptr, ce);
        if (instanceof_function(ce, goda_controller_ce)) {
                /* call method construct */
            zend_call_method_with_0_params(&controller_ptr, ce, &ce->constructor, "__construct", NULL);
            zend_update_property(ce, &controller_ptr, ZEND_STRL(GODA_CONTROLLER_REUQEST), request_ptr);
            zend_call_method(&controller_ptr, ce, NULL, ZSTR_VAL(action_name), ZSTR_LEN(action_name), NULL, 0, NULL, NULL);
        }
        zval_ptr_dtor(&controller_ptr);
    } else {
        zend_error_noreturn(E_ERROR, "Couldn't find controller and action");
    }
    zend_string_release(action_name);
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

    zend_declare_property_null(goda_router_ce, ZEND_STRL(GODA_ROUTER_CONTROLLER), ZEND_ACC_PUBLIC);
    zend_declare_property_null(goda_router_ce, ZEND_STRL(GODA_ROUTER_ACTION), ZEND_ACC_PUBLIC);
    return SUCCESS;
}