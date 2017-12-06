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
  | Author: fankxu                                                  |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_XMYSQL_LOADER_H
#define PHP_XMYSQL_LOADER_H

#define XMYSQL_DB_TYPE_AUTO 0
#define XMYSQL_DB_TYPE_MASTER 1
#define XMYSQL_DB_TYPE_SLAVE 2

int xmysql_loader_get_db(zval *mysqli, zend_string *dbName, zend_ulong  type);

ZEND_MINIT_FUNCTION(xmysql_loder);
ZEND_MSHUTDOWN_FUNCTION(xmysql_loader);
ZEND_RSHUTDOWN_FUNCTION(xmysql_loader);

#endif

