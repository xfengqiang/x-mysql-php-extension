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

#ifndef PHP_XMYSQL_COMMON_H
#define PHP_XMYSQL_COMMON_H

#include "php.h"

#define X_IS_EMPTY_P(z) ( !(z!=NULL && Z_TYPE_P(z)!=IS_NULL) )
#define X_RETURN_THIS RETURN_ZVAL(getThis(), 1, 0)
#define X_ZSTR_EQUAL(z, str) (strcmp(Z_STRVAL_P(z), str)==0)

/*关联数组级联查找， 如key=a.b.c*/
zval *x_hash_get_path(zval *data, char *path, int path_length);

static int air_call_func(const char *func_name, uint32_t param_count, zval params[], zval *retval){
	zval func;
	ZVAL_STRING(&func, func_name);
	zval *_retval = NULL;
	if(!retval){
		zval ret;
		_retval = &ret;
	}
	int status = call_user_function(EG(function_table), NULL, &func, retval?retval: _retval, param_count, params);
	zval_ptr_dtor(&func);
	if(_retval){
		zval_ptr_dtor(_retval);
	}
	return status;
}


#define AIR_OBJ_INIT(pz, classname) do {\
		zend_class_entry *ce = NULL;\
		if((ce = (zend_class_entry *)zend_hash_str_find_ptr(EG(class_table), classname, sizeof(classname)-1)) != NULL) {\
			object_init_ex((pz), ce);\
		}\
	}while(0)


#define  AIR_METHOD_MAX_PARAM_SIZE 8
static inline zval* air_call_method(zval *object, zend_class_entry *obj_ce, zend_function **fn_proxy, const char *function_name, size_t function_name_len, zval *retval_ptr, int param_count, zval params[])
{
	int result;
	zend_fcall_info fci;
	zval retval;
	HashTable *function_table;
	if(param_count > AIR_METHOD_MAX_PARAM_SIZE){
		php_error(E_ERROR, "too many params");
	}
	
	zval args[AIR_METHOD_MAX_PARAM_SIZE];
	int i = 0;
	for(; i<param_count; i++){
		ZVAL_COPY_VALUE(&args[i], &params[i]);
	}
	
	fci.size = sizeof(fci);
	/*fci.function_table = NULL; will be read form zend_class_entry of object if needed */
	fci.object = (object && Z_TYPE_P(object) == IS_OBJECT) ? Z_OBJ_P(object) : NULL;
	ZVAL_STRINGL(&fci.function_name, function_name, function_name_len);
	fci.retval = retval_ptr ? retval_ptr : &retval;
	fci.param_count = param_count;
	fci.params = params;
	fci.no_separation = 1;
//	fci.symbol_table = NULL; //by fankxu
	
	if (!fn_proxy && !obj_ce) {
		/* no interest in caching and no information already present that is
		 * needed later inside zend_call_function. */
//		fci.function_table = !object ? EG(function_table) : NULL; //by fankxu
		result = zend_call_function(&fci, NULL);
		zval_ptr_dtor(&fci.function_name);
	} else {
		zend_fcall_info_cache fcic;
		
		fcic.initialized = 1;
		if (!obj_ce) {
			obj_ce = object ? Z_OBJCE_P(object) : NULL;
		}
		if (obj_ce) {
			function_table = &obj_ce->function_table;
		} else {
			function_table = EG(function_table);
		}
		if (!fn_proxy || !*fn_proxy) {
			if ((fcic.function_handler = zend_hash_find_ptr(function_table, Z_STR(fci.function_name))) == NULL) {
				/* error at c-level */
				zend_error_noreturn(E_CORE_ERROR, "Couldn't find implementation for method %s%s%s", obj_ce ? ZSTR_VAL(obj_ce->name) : "", obj_ce ? "::" : "", function_name);
			}
			if (fn_proxy) {
				*fn_proxy = fcic.function_handler;
			}
		} else {
			fcic.function_handler = *fn_proxy;
		}
		fcic.calling_scope = obj_ce;
		if (object) {
			fcic.called_scope = Z_OBJCE_P(object);
		} else {
			zend_class_entry *called_scope = zend_get_called_scope(EG(current_execute_data));
			
			if (obj_ce &&
			    (!called_scope ||
			     !instanceof_function(called_scope, obj_ce))) {
				fcic.called_scope = obj_ce;
			} else {
				fcic.called_scope = called_scope;
			}
		}
		fcic.object = object ? Z_OBJ_P(object) : NULL;
		result = zend_call_function(&fci, &fcic);
		zval_ptr_dtor(&fci.function_name);
	}
	if (result == FAILURE) {
		/* error at c-level */
		if (!obj_ce) {
			obj_ce = object ? Z_OBJCE_P(object) : NULL;
		}
		if (!EG(exception)) {
			zend_error_noreturn(E_CORE_ERROR, "Couldn't execute method %s%s%s", obj_ce ? ZSTR_VAL(obj_ce->name) : "", obj_ce ? "::" : "", function_name);
		}
	}
	/* copy arguments back, they might be changed by references */
	for(i=0; i<param_count; i++){
		if(Z_ISREF(args[i]) && !Z_ISREF(params[i])){
			ZVAL_COPY_VALUE(&params[i], &args[i]);
		}
	}
	if (!retval_ptr) {
		zval_ptr_dtor(&retval);
		return NULL;
	}
	return retval_ptr;
}



#define air_call_object_method(obj, obj_ce, function_name, retval_ptr, param_count, params) air_call_method(obj, obj_ce, NULL, function_name, strlen(function_name), retval_ptr, param_count, params)
#define air_call_static_method(obj_ce, function_name, retval_ptr, param_count, params) air_call_method(NULL, obj_ce, NULL, function_name, sizeof(function_name)-1, retval_ptr, param_count, params)


#endif

