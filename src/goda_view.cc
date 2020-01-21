#include "php_goda.h"
#include "ext/standard/php_array.h"
#include "goda_view.h"
#include "goda_loader.h"

zend_class_entry *goda_view_ce;

/**
 * copy ext/standard/array.c line 1650 . 
 * static zend_always_inline int php_valid_var_name(const char *var_name, size_t var_name_len) 
 */
static zend_always_inline int goda_valid_var_name(const char *var_name, size_t var_name_len) /* {{{ */
{
#if 1
	/* first 256 bits for first character, and second 256 bits for the next */
	static const uint32_t charset[8] = {
	     /*  31      0   63     32   95     64   127    96 */
			0x00000000, 0x00000000, 0x87fffffe, 0x07fffffe,
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};
	static const uint32_t charset2[8] = {
	     /*  31      0   63     32   95     64   127    96 */
			0x00000000, 0x03ff0000, 0x87fffffe, 0x07fffffe,
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};
#endif
	size_t i;
	uint32_t ch;

	if (UNEXPECTED(!var_name_len)) {
		return 0;
	}

	/* These are allowed as first char: [a-zA-Z_\x7f-\xff] */
	ch = (uint32_t)((unsigned char *)var_name)[0];
#if 1
	if (UNEXPECTED(!ZEND_BIT_TEST(charset, ch))) {
#else
	if (var_name[0] != '_' &&
		(ch < 65  /* A    */ || /* Z    */ ch > 90)  &&
		(ch < 97  /* a    */ || /* z    */ ch > 122) &&
		(ch < 127 /* 0x7f */ || /* 0xff */ ch > 255)
	) {
#endif
		return 0;
	}

	/* And these as the rest: [a-zA-Z0-9_\x7f-\xff] */
	if (var_name_len > 1) {
		i = 1;
		do {
			ch = (uint32_t)((unsigned char *)var_name)[i];
#if 1
			if (UNEXPECTED(!ZEND_BIT_TEST(charset2, ch))) {
#else
			if (var_name[i] != '_' &&
				(ch < 48  /* 0    */ || /* 9    */ ch > 57)  &&
				(ch < 65  /* A    */ || /* Z    */ ch > 90)  &&
				(ch < 97  /* a    */ || /* z    */ ch > 122) &&
				(ch < 127 /* 0x7f */ || /* 0xff */ ch > 255)
			) {
#endif
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


static void goda_view_render_ob() {
    
    // zend_string *file_path; //
    // zval ret = {{0}};

    

    // if (count) {
    
        
    //     // if (goda_view_include_file(file_path)) {
    //     //     zend_string_release(file_path);
    //     // } else {
    //     //     zend_string_release(file_path);
    //     // }
        
    //     if (php_output_get_contents(&ret) == FAILURE) {
	// 		php_output_end();
	// 		php_error_docref(NULL, E_WARNING, "Unable to fetch ob content");
	// 		return;
	// 	}

    //     // php_write(Z_STRVAL(ret), Z_STRLEN(ret));

    //     php_var_dump(&ret, 10);
        
        
	// 	if (php_output_discard() != SUCCESS ) {
    //         zend_printf("ob end");
	// 		return;
	// 	}

    // }

}

void goda_view_render(zend_string *filename, zval *assgin) {
    // zend_file_handle file_handle;
	// zend_op_array 	*op_array;
	// char realpath[MAXPATHLEN];
    zend_string *path;
    zend_array *symbol_table;
    // zval ret = {{0}};

    symbol_table = zend_rebuild_symbol_table();
    (void)goda_extract_ref_overwrite(Z_ARRVAL_P(assgin), symbol_table);
	
    zend_string *file_trim = php_trim(filename, "/", 1, 3);

    path = strpprintf(0, "%s/views/%s.php", ZSTR_VAL(GODA_G(app_dir)), ZSTR_VAL(file_trim));
	goda_loader_include(path, NULL);
    zend_string_release(path);
    zend_string_release(file_trim);
}


ZEND_BEGIN_ARG_INFO_EX(goda_view_render_arg, 0, 0, 2)
    ZEND_ARG_INFO(0, filename)
    ZEND_ARG_ARRAY_INFO(0, val, 1)
ZEND_END_ARG_INFO()

ZEND_METHOD(goda_view, render) {
    zend_string *filename;
    zval *val;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(filename)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(val)
    ZEND_PARSE_PARAMETERS_END();
    goda_view_render(filename, val);
}

zend_function_entry goda_view_methods[] = {
    ZEND_ME(goda_view, render, goda_view_render_arg, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    ZEND_FE_END
};

GODA_MINIT_FUNCTION(view) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Goda\\View", goda_view_methods);
    goda_view_ce = zend_register_internal_class(&ce);
    goda_view_ce->ce_flags |= ZEND_ACC_FINAL;

    return SUCCESS;
}