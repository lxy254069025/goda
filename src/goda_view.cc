#include "php_goda.h"
#include "php_main.h"
#include "ext/standard/php_array.h"
#include "goda_view.h"
#include "goda_loader.h"
#include "goda_exception.h"

zend_class_entry *goda_view_ce;

/**
 * copy ext/standard/array.c line 1650 . 
 * static zend_always_inline int php_valid_var_name(const char *var_name, size_t var_name_len) 
 */
static zend_always_inline int goda_valid_var_name(const char *var_name, size_t var_name_len) /* {{{ */
{
	size_t i;
	uint32_t ch;

	if (UNEXPECTED(!var_name_len)) {
		return 0;
	}

	/* These are allowed as first char: [a-zA-Z_\x7f-\xff] */
	ch = (uint32_t)((unsigned char *)var_name)[0];
	if (var_name[0] != '_' &&
		(ch < 65  /* A    */ || /* Z    */ ch > 90)  &&
		(ch < 97  /* a    */ || /* z    */ ch > 122) &&
		(ch < 127 /* 0x7f */ || /* 0xff */ ch > 255)
	) {
		return 0;
	}

	/* And these as the rest: [a-zA-Z0-9_\x7f-\xff] */
	if (var_name_len > 1) {
		i = 1;
		do {
			ch = (uint32_t)((unsigned char *)var_name)[i];
			if (var_name[i] != '_' &&
				(ch < 48  /* 0    */ || /* 9    */ ch > 57)  &&
				(ch < 65  /* A    */ || /* Z    */ ch > 90)  &&
				(ch < 97  /* a    */ || /* z    */ ch > 122) &&
				(ch < 127 /* 0x7f */ || /* 0xff */ ch > 255)
			) {
				return 0;
			}
		} while (++i < var_name_len);
	}
	return 1;
}

zend_long goda_extract_ref_overwrite(zend_array *arr, zend_array *symbol_table) /* {{{ */
{
	zend_long count = 0;
	zend_string *var_name;
	zval *entry, *orig_var;

	ZEND_HASH_FOREACH_STR_KEY_VAL_IND(arr, var_name, entry) {
		if (!var_name) {
			continue;
		}
		if (!goda_valid_var_name(ZSTR_VAL(var_name), ZSTR_LEN(var_name))) {
			continue;
		}
		if (zend_string_equals_literal(var_name, "this")) {
			zend_throw_error(NULL, "Cannot re-assign $this");
			return -1;
		}
		orig_var = zend_hash_find_ex(symbol_table, var_name, 1);
		if (orig_var) {
			if (Z_TYPE_P(orig_var) == IS_INDIRECT) {
				orig_var = Z_INDIRECT_P(orig_var);
			}
			if (zend_string_equals_literal(var_name, "GLOBALS")) {
				continue;
			}
			if (Z_ISREF_P(entry)) {
				Z_ADDREF_P(entry);
			} else {
				ZVAL_MAKE_REF_EX(entry, 2);
			}
			zval_ptr_dtor(orig_var);
			ZVAL_REF(orig_var, Z_REF_P(entry));
		} else {
			if (Z_ISREF_P(entry)) {
				Z_ADDREF_P(entry);
			} else {
				ZVAL_MAKE_REF_EX(entry, 2);
			}
			zend_hash_add_new(symbol_table, var_name, entry);
		}
		count++;
	} ZEND_HASH_FOREACH_END();

	return count;
}

static int goda_view_ob(zend_op_array *op_array, zval *retval) {
	zval result;
	ZVAL_UNDEF(&result);
	if (php_output_start_user(NULL, 0, PHP_OUTPUT_HANDLER_STDFLAGS) == FAILURE) {
		php_error_docref("ref.outcontrol", E_WARNING, "failed to create buffer");
		return 0;
	}
	
	zend_execute(op_array, &result);
	zval_ptr_dtor(&result);

	if (EG(exception) != NULL) {
		php_output_discard();
	}

	if (php_output_get_contents(retval) == FAILURE) {
		php_output_end();
		php_error_docref(NULL, E_WARNING, "Unable to fetch ob content");
		return 0;
	}

	if (php_output_discard() != SUCCESS ) {
		return 0;
	}
	return 1;
}

int goda_view_render(zend_string *filename, zval *assgin, zval *retval) {
    zend_string *path;
    zend_array *symbol_table;
	int status = 0;
	zend_file_handle file_handle;
	zend_op_array 	*op_array;
	char realpath[MAXPATHLEN];

    symbol_table = zend_rebuild_symbol_table();
    (void)goda_extract_ref_overwrite(Z_ARRVAL_P(assgin), symbol_table);
	
    zend_string *file_trim = php_trim(filename, (char *)"/", sizeof("/")-1, 3);

    path = strpprintf(0, "%s/views/%s.php", ZSTR_VAL(GODA_G(app_dir)), ZSTR_VAL(file_trim));
	zend_string_release(file_trim);

	if (!VCWD_REALPATH(ZSTR_VAL(path), realpath)) {
		zend_string_release(path);
		goda_throw_exception(E_ERROR, "Failed opening template %s: %s", ZSTR_VAL(path), strerror(errno));
		return 0;
	}

	file_handle.filename = ZSTR_VAL(path);
	file_handle.free_filename = 0;
	file_handle.type = ZEND_HANDLE_FILENAME;
	file_handle.opened_path = NULL;
	file_handle.handle.fp = NULL;

	op_array = zend_compile_file(&file_handle, ZEND_INCLUDE);

	if (op_array) {
		if (file_handle.handle.stream.handle) {
			if (!file_handle.opened_path) {
				file_handle.opened_path = zend_string_copy(path);
			}
			zend_hash_add_empty_element(&EG(included_files), file_handle.opened_path);
		}

		status = goda_view_ob(op_array, retval);

		destroy_op_array(op_array);
		efree(op_array);
	} 
	zend_destroy_file_handle(&file_handle);
	zend_string_release(path);

	return status;
}



zend_function_entry goda_view_methods[] = {
    ZEND_FE_END
};

GODA_MINIT_FUNCTION(view) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Goda\\View", goda_view_methods);
    goda_view_ce = zend_register_internal_class(&ce);
    goda_view_ce->ce_flags |= ZEND_ACC_FINAL;

    return SUCCESS;
}