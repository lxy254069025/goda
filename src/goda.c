/* goda extension for PHP */

#include "php_goda.h"
#include "goda_logo.h"
#include "goda_application.h"
#include "goda_router.h"
#include "goda_loader.h"
#include "goda_controller.h"
#include "goda_view.h"
#include "goda_response.h"
#include "goda_config.h"
#include "goda_request.h"
#include "goda_session.h"
#include "goda_cookie.h"
#include "goda_model.h"
#include "goda_exception.h"

ZEND_DECLARE_MODULE_GLOBALS(goda);

/* 初始化module时运行 */
PHP_MINIT_FUNCTION(goda)
{
    /* If you have INI entries, uncomment these lines
    REGISTER_INI_ENTRIES();
    */
   	GODA_STARTUP(exception);
   	GODA_STARTUP(application);
	GODA_STARTUP(router);
	GODA_STARTUP(loader);
	GODA_STARTUP(response);
	GODA_STARTUP(controller);
	GODA_STARTUP(view);
	GODA_STARTUP(config);
	GODA_STARTUP(request);
	GODA_STARTUP(session);
	GODA_STARTUP(cookie);
    return SUCCESS;
}

/* 当module被卸载时运行 */
PHP_MSHUTDOWN_FUNCTION(goda)
{
    /* uncomment this line if you have INI entries
    UNREGISTER_INI_ENTRIES();
    */
    return SUCCESS;
}

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(goda)
{
#if defined(ZTS) && defined(COMPILE_DL_GODA)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/*当一个REQUEST请求结束时运行*/
PHP_RSHUTDOWN_FUNCTION(goda)
{
	if (GODA_G(app_dir)) {
		zend_string_release(GODA_G(app_dir));
		GODA_G(app_dir) = NULL;
	}
    return SUCCESS;
}

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(goda)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "goda support", GODA_LOGO_BASE64"enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ goda_module_entry
 */
zend_module_entry goda_module_entry = {
	STANDARD_MODULE_HEADER,
	"goda",					/* Extension name */
	NULL,			/* zend_function_entry */
	PHP_MINIT(goda),			/* PHP_MINIT - Module initialization */
	PHP_MSHUTDOWN(goda),		/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(goda),			/* PHP_RINIT - Request initialization */
	PHP_RSHUTDOWN(goda),		/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(goda),			/* PHP_MINFO - Module info */
	PHP_GODA_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_GODA
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(goda)
#endif

