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
  | Author: xufengqiang                                                  |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#include "php.h"
//#include "php_ini.h"
//#include "ext/standard/info.h"
//#include "../php_xmysql.h"
//#include "src/xmysql_db.h"

#include "php.h"
#include "php_ini.h"
#include "ext/standard/php_array.h"

#include "php_xmysql.h"
#include "src/xmysql_db.h"
#include "src/xmysql_common.h"

static zend_function_entry xmysql_db_methods[] = {
        NULL, NULL, NULL
};

ZEND_MINIT_FUNCTION(xmysql_db){
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "xmysql", xmysql_db_methods);
    zend_register_internal_class(&ce TSRMLS_CC);
    return SUCCESS;
}