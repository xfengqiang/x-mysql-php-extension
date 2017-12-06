/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2017 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_xmysql.h"

#include "src/xmysql_define.h"

#include "src/xmysql_loader.h"
#include "src/xmysql_cond.h"
#include "src/xmysql_db.h"


/* If you declare any globals in php_xmysql.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(xmysql)
*/

/* True global resources - no need for thread safety here */
static int le_xmysql;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("xmysql.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_xmysql_globals, xmysql_globals)
    STD_PHP_INI_ENTRY("xmysql.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_xmysql_globals, xmysql_globals)
PHP_INI_END()
*/
/* }}} */



/* {{{ php_xmysql_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_xmysql_init_globals(zend_xmysql_globals *xmysql_globals)
{
	xmysql_globals->global_value = 0;
	xmysql_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(xmysql)
{
    //初始化个各自文件
	XMYSQL_MODULE_STARTUP(xmysql_loader);
	XMYSQL_MODULE_STARTUP(xmysql_cond);
    XMYSQL_MODULE_STARTUP(xmysql_db);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(xmysql)
{
	ZEND_MODULE_SHUTDOWN_N(xmysql_loader);
    ZEND_MODULE_SHUTDOWN_N(xmysql_cond);
    ZEND_MODULE_SHUTDOWN_N(xmysql_db);
    
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(xmysql)
{
#if defined(COMPILE_DL_XMYSQL) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(xmysql)
{
    ZEND_RSHUTDOWN_FUNCTION(xmysql_loader);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(xmysql)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "xmysql support", "enabled");
    php_info_print_table_row(2, "xmysql version", "1.0.0");
    php_info_print_table_row(2, "xmysql author", "fankxu");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ xmysql_functions[]
 *
 * Every user visible function must have an entry in xmysql_functions[].
 */
const zend_function_entry xmysql_functions[] = {
//	PHP_FE(confirm_xmysql_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in xmysql_functions[] */
};
/* }}} */

static zend_module_dep x_mysql_deps[] = {
    ZEND_MOD_REQUIRED("mysqli")
    {NULL, NULL, NULL}
};
/* {{{ xmysql_module_entry
 */
zend_module_entry xmysql_module_entry = {
	STANDARD_MODULE_HEADER,
	// sizeof(zend_module_entry), 
	// ZEND_MODULE_API_NO, ZEND_DEBUG, 
	// USING_ZTS,
	//  NULL, &x_mysql_deps,
	"xmysql",
	NULL,
	PHP_MINIT(xmysql),
	PHP_MSHUTDOWN(xmysql),
	PHP_RINIT(xmysql),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(xmysql),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(xmysql),
	PHP_XMYSQL_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_XMYSQL
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(xmysql)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
