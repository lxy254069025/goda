/* goda extension for PHP */

#ifndef PHP_GODA_H
# define PHP_GODA_H

extern "C" {
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"
#include "Zend/zend_interfaces.h"
#include "main/SAPI.h"
#include "ext/standard/php_string.h"
#include "Zend/zend_smart_str.h"
#include "ext/pcre/php_pcre.h"
#include "Zend/zend_exceptions.h"
#include "php_globals.h"
}

int goda_call_method(zval *obj, zend_class_entry *obj_ce, const char *func_name, size_t func_name_len, zval *retval_ptr, int number, zval *param);

extern zend_module_entry goda_module_entry;
# define phpext_goda_ptr &goda_module_entry

# define PHP_GODA_VERSION "0.1.2"

# if defined(ZTS) && defined(COMPILE_DL_GODA)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#define GODA_STARTUP(module)                 ZEND_MODULE_STARTUP_N(goda_##module)(INIT_FUNC_ARGS_PASSTHRU)
#define GODA_MINIT_FUNCTION(module)          ZEND_MINIT_FUNCTION(goda_##module) 

#define LUR_SHUTDOWN(module)                ZEND_MODULE_SHUTDOWN_N(goda_##module)(SHUTDOWN_FUNC_ARGS_PASSTHRU)
#define LUR_MSHUTDOWN_FUNCTION(module)      ZEND_MSHUTDOWN_FUNCTION(goda_##module)


/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

BEGIN_EXTERN_C()
ZEND_BEGIN_MODULE_GLOBALS(goda)
	zend_string *app_dir;
ZEND_END_MODULE_GLOBALS(goda)

extern ZEND_DECLARE_MODULE_GLOBALS(goda);

END_EXTERN_C()

#ifdef ZTS
#define GODA_G(v) TSRMG(goda_globals_id, zend_goda_globals *, v)
#else
#define GODA_G(v) (goda_globals.v)
#endif


#endif	/* PHP_GODA_H */

