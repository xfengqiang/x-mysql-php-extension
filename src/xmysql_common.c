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

#include "xmysql_common.h"

zval *x_hash_get_path(zval *data, char *path, int path_length) {
	zval *ret = data;
	if (data == NULL) {
		return ret;
	}

	zval **tmp =  NULL;
	char *seg = NULL, *sptr = NULL;
	char *skey = estrndup(path, path_length);
	seg = php_strtok_r( skey, ".", &sptr);
	while(seg) {
		if(Z_TYPE_P(ret) != IS_ARRAY) {
			ret = NULL;
			break;
		}
		ret = zend_hash_str_find(Z_ARRVAL_P(ret), seg, strlen(seg));
		if(!ret) {
			php_error(E_NOTICE, "config path:%s not found\n");
			break;
		}
		seg = php_strtok_r(NULL, ".", &sptr);
	}
	efree(skey);
	return ret;
}
