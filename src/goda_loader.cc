#include "php_goda.h"
#include "php_main.h"
#include "goda_loader.h"

zend_class_entry *goda_loader_ce;

void goda_loader_instance() {
    zval this_ptr, function, params, retval;
    object_init_ex(&this_ptr, goda_loader_ce);

    ZVAL_STRING(&function, "spl_autoload_register");

    array_init(&params);
    add_next_index_zval(&params, &this_ptr);
    add_next_index_string(&params, "autoloader");

    zend_fcall_info zfci = {
        sizeof(zfci),
#if PHP_VERSION_ID < 70100
        EG(function_table)
#endif
        function,
#if PHP_VERSION_ID < 70100
        NULL,
#endif
        &retval,
        &params, 
        NULL,
        1,
        1
    };

    if (zend_call_function(&zfci, NULL) == FAILURE) {
        php_error_docref(NULL, E_WARNING, "Unable to register autoload function spl_autoload_register!");
    } 
    zval_ptr_dtor(&function);
    zval_ptr_dtor(&params);
    zval_ptr_dtor(&retval);
}


int goda_loader_include(zend_string *filename, zval *retval) {

    zend_file_handle file_handle;
	zend_op_array 	*op_array;
	char realpath[MAXPATHLEN];

	if (!VCWD_REALPATH(ZSTR_VAL(filename), realpath)) {
        zend_error(E_ERROR, "Failed opening template %s: %s", ZSTR_VAL(filename), strerror(errno));
		return 0;
	}

	file_handle.filename = ZSTR_VAL(filename);
	file_handle.free_filename = 0;
	file_handle.type = ZEND_HANDLE_FILENAME;
	file_handle.opened_path = NULL;
	file_handle.handle.fp = NULL;

	op_array = zend_compile_file(&file_handle, ZEND_INCLUDE);

	if (op_array && file_handle.handle.stream.handle) {
		if (!file_handle.opened_path) {
			file_handle.opened_path = zend_string_copy(filename);
		}
		zend_hash_add_empty_element(&EG(included_files), file_handle.opened_path);

        zval result;

        ZVAL_UNDEF(&result);
		zend_execute(op_array, &result);
		destroy_op_array(op_array);
		efree(op_array);
        if (!EG(exception)) {
            if (!retval) {
                zval_ptr_dtor(&result);
            } else {
                ZVAL_COPY_VALUE(retval, &result);
            }
        }
	    return 1;
	}
	zend_destroy_file_handle(&file_handle);

	return 0;
}

int goda_loader_autoloader(char *class_name, size_t len) {
    zend_string *class_path;

    for (size_t i = 0; i < len; i++) {
        if (class_name[i] == '\\') {
            class_name[i] = '/';
        }
    }

    class_path = strpprintf(0, "%s/%s.php", ZSTR_VAL(GODA_G(app_dir)), class_name);
    if (!goda_loader_include(class_path, NULL)) {
        zend_error(E_ERROR, "Couldn't find file: %s", ZSTR_VAL(class_path));
        return 0;
    }
    zend_string_release(class_path);

    return 1;
}

ZEND_BEGIN_ARG_INFO_EX(goda_loader_load_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, class)
ZEND_END_ARG_INFO()

ZEND_METHOD(goda_loader, autoloader) {
    char *class_name;
    size_t len;

    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_STRING(class_name, len)
    ZEND_PARSE_PARAMETERS_END();

    (void)goda_loader_autoloader(class_name, len);
}

zend_function_entry goda_loader_methods[] = {
    ZEND_ME(goda_loader, autoloader, goda_loader_load_arginfo, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

GODA_MINIT_FUNCTION(loader) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Goda\\Loader", goda_loader_methods);
    goda_loader_ce = zend_register_internal_class(&ce);
    goda_loader_ce->ce_flags |= ZEND_ACC_FINAL;

    return SUCCESS;
}