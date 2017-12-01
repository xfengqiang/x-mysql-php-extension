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
#include "src/xmysql_loader.h"
#include "src/xmysql_db.h"
#include "src/xmysql_common.h"

zend_class_entry *xmysql_db_ce;

zend_class_entry *get_xmysql_cond_ce() {
    return  (zend_class_entry *)zend_hash_str_find_ptr(EG(class_table), ZEND_STRL("xmysql_cond"));
}

PHP_METHOD(xmysql_db, setGlobalCallBack){
    zend_fcall_info f;
    zend_fcall_info_cache f_cache;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "f", &f, &f_cache)) {
        return ;
    }
    // zend_update_static_property()
    //TODO 如何设置一个callable成员？
}

PHP_METHOD(xmysql_db, enbleGlobalProfile){
    zend_bool enable;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "b", &enable)) {
        return;
    }
    zend_update_static_property_bool(xmysql_db_ce, ZEND_STRL("globalEnableProfie"), enable);
}

PHP_METHOD(xmysql_db, __construct){
    zend_string *dbName;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &dbName)) {
        return ;
    }
    zend_update_property_str(xmysql_db_ce, getThis(), ZEND_STRL("_dbName"), dbName);
}

PHP_METHOD(xmysql_db, callback){
    //TODO 如何设置callback成员？
}

//初始化xmysql_db->_sqlCond
void init_sql_cond(zval *obj, const char *method, const char * table, zend_ulong paramNum, zval params[]) {
    zval cond;
    
    zval tableParam[1];
    ZVAL_STR(&tableParam[0], table);
    air_call_func("xmysql_cond::table", 1, tableParam, &cond);

    php_var_dump(&cond, 0);
    zend_class_entry *cond_ce = get_xmysql_cond_ce();
    air_call_object_method(&cond, cond_ce, method, NULL, paramNum, params);
  
    zend_update_property(xmysql_db_ce, obj, ZEND_STRL("_sqlCond"), &cond);
 
    zval_ptr_dtor(&cond);
}

PHP_METHOD(xmysql_db, select){
    zend_string *table = NULL, *fields = NULL;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "SS", &table, &fields)) {
        RETURN_FALSE;
    }

    //使用这种方式调用会报错，提示 Couldn't find implementation for method xmysql_cond::select
    zval params[1];
    ZVAL_STR(&params[0], fields);
    init_sql_cond(getThis(), "select", ZSTR_VAL(table), 1, params);

    X_RETURN_THIS;
}

PHP_METHOD(xmysql_db, insert){
    zend_string *table = NULL;
    zval *data = NULL;
    zend_bool ignore = 0;

    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sa|b")) {
        RETURN_FALSE;
    }

    zval params[2];
    ZVAL_ARR(&params[0], data);
    ZVAL_BOOL(&params[1], &ignore);
    init_sql_cond(getThis(), "insert", ZSTR_VAL(table), 2, params);

    X_RETURN_THIS;
}

PHP_METHOD(xmysql_db, update){
    zend_string *table = NULL;
    zval *data;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sa")) {
        RETURN_FALSE;
    }

    zval params[1];
    ZVAL_ARR(&params[0], data);
    init_sql_cond(getThis(), "update", ZSTR_VAL(table), 1, params);

    X_RETURN_THIS;
}

PHP_METHOD(xmysql_db, del){
    zend_string *table;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S")) {
        RETURN_FALSE;
    }
    
    zval params[0];
    init_sql_cond(getThis(), "del", ZSTR_VAL(table), 0, params);

    X_RETURN_THIS;
}

void add_sql_cond(zval *obj, const char *method, zend_ulong paramNum, zval params[]) {
    zval rv;
    zval *cond = zend_read_property(xmysql_db_ce, obj, ZEND_STRL("_sqlCond"), 0, &rv);
    if(!cond || Z_TYPE_P(cond)==IS_NULL) {
        return ;
    }
  
    air_call_object_method(cond, get_xmysql_cond_ce(), method, NULL, paramNum, params);
}

PHP_METHOD(xmysql_db, andc){
    zend_string *key = NULL, *op = NULL;
    zval *v = NULL;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sz|S", &key, &v, &op)) {
        RETURN_FALSE;
    }

    zval params[3];
    ZVAL_STR(&params[0], key);
    ZVAL_ZVAL(&params[1], v, 0, 0);
    if(op) {
        ZVAL_STR(&params[2], op);
    }
   
    add_sql_cond(getThis(), "andc", op ? 3 : 2, params);


    X_RETURN_THIS;
}

PHP_METHOD(xmysql_db, orc){
    zend_string *key = NULL, *op = NULL;
    zval *v = NULL;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sz|S")) {
        RETURN_FALSE;
    }

    zval params[3];
    ZVAL_STR(&params[0], key);
    ZVAL_ZVAL(&params[1], v, 0, 0);
    if(op) {
        ZVAL_STR(&params[2], op);
    }
    add_sql_cond(getThis(), "orc", op ? 3 : 2, params);

    X_RETURN_THIS;
}

PHP_METHOD(xmysql_db, equal){
    zval *params;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "a", &params)) {
        RETURN_FALSE;
    }

    zval fparams[1];
    ZVAL_ZVAL(&fparams[0], params, 0, 0);
    add_sql_cond(getThis(), "equal", 1, fparams);

    X_RETURN_THIS;
}

PHP_METHOD(xmysql_db, in){
    zend_string *key = NULL, *cond = NULL;
    zval *vals;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sa|S", &key, &vals, &cond)){
        RETURN_FALSE;
    }

    zval params[3];
    ZVAL_STR(&params[0], key);
    ZVAL_ZVAL(&params[1], vals, 0, 0);
    if(cond) {
        ZVAL_STR(&params[2], cond);
    }
    add_sql_cond(getThis(), "in", cond?3:2, params);

    X_RETURN_THIS;
}

PHP_METHOD(xmysql_db, notIn){
    zend_string *key = NULL, *cond = NULL;
    zval *vals;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sa|S", &key, &vals, &cond)){
        RETURN_FALSE;
    }

    zval params[3];
    ZVAL_STR(&params[0], key);
    ZVAL_ZVAL(&params[1], vals, 0, 0);
    if(cond) {
        ZVAL_STR(&params[2], cond);
    }
    add_sql_cond(getThis(), "notIn", cond?3:2, params);

    X_RETURN_THIS;
}

PHP_METHOD(xmysql_db, order){
    zval *args;
    zend_ulong argc = 0;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "+", &args, &argc)) {
        RETURN_FALSE;
    }

    if(argc > 2) {
        argc = 2;
    }

    add_sql_cond(getThis(), "order", argc, args);

    X_RETURN_THIS;
}

PHP_METHOD(xmysql_db, limit){
    zval *args;
    zend_ulong argc = 0;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "+", &args, &argc)) {
        RETURN_FALSE;
    }

    if(argc > 2) {
        argc = 2;
    }

    add_sql_cond(getThis(), "limit", argc, args);

    X_RETURN_THIS;
}

PHP_METHOD(xmysql_db, where){
    X_RETURN_THIS;
}

PHP_METHOD(xmysql_db, db){
    zend_long type ;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &type) == FAILURE){
        RETURN_FALSE;
    }

    zval rv;
    zval *dbName = zend_read_property(xmysql_db_ce, getThis(), ZEND_STRL("_dbName"), 0, &rv);
    
    zval *ret = NULL;

    if(!xmysql_loader_get_db(&ret, Z_STR_P(dbName), 1)){
        RETURN_NULL();
    }
    
    RETURN_ZVAL(ret, 1, 0);
}

PHP_METHOD(xmysql_db, exec){}
PHP_METHOD(xmysql_db, queryByCond){}
PHP_METHOD(xmysql_db, queryRowByCond){}
PHP_METHOD(xmysql_db, query){}
PHP_METHOD(xmysql_db, queryRow){}
PHP_METHOD(xmysql_db, getQueryType){}

PHP_METHOD(xmysql_db, lastErrCode){}
PHP_METHOD(xmysql_db, lastErrorMsg){}
PHP_METHOD(xmysql_db, lastSql){}
PHP_METHOD(xmysql_db, rowsAffected){}
PHP_METHOD(xmysql_db, lastInsertId){}
PHP_METHOD(xmysql_db, lastQUERYTime){}
PHP_METHOD(xmysql_db, getDbName){}
PHP_METHOD(xmysql_db, isInTx){}
PHP_METHOD(xmysql_db, startTx){}
PHP_METHOD(xmysql_db, commitTx){}
PHP_METHOD(xmysql_db, rollbackTx){}

static zend_function_entry xmysql_db_methods[] = {
    PHP_ME(xmysql_db, setGlobalCallBack, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(xmysql_db, enbleGlobalProfile, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(xmysql_db, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(xmysql_db, callback, NULL, ZEND_ACC_PUBLIC)
    
    PHP_ME(xmysql_db, select, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, insert, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, update, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, del, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, andc, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, orc, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, equal, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, in, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, notIn, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, order, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, limit, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, where, NULL, ZEND_ACC_PUBLIC)

    PHP_ME(xmysql_db, db, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, exec, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, queryByCond, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, queryRowByCond, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, query, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, queryRow, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, getQueryType, NULL, ZEND_ACC_PUBLIC)

    PHP_ME(xmysql_db, lastErrCode, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, lastErrorMsg, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, lastSql, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, rowsAffected, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, lastInsertId, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, lastQUERYTime, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, getDbName, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, isInTx, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, startTx, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, commitTx, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, rollbackTx, NULL, ZEND_ACC_PUBLIC)

    PHP_FE_END
};

ZEND_MINIT_FUNCTION(xmysql_db){
    zend_class_entry ce;
    //方法注册
    INIT_CLASS_ENTRY(ce, "xmysql_db", xmysql_db_methods);
    xmysql_db_ce = zend_register_internal_class(&ce TSRMLS_CC);
    
    //属性
    zend_declare_property_null(xmysql_db_ce, ZEND_STRL("globalCallBack"), ZEND_ACC_PRIVATE|ZEND_ACC_STATIC);
    zend_declare_property_null(xmysql_db_ce, ZEND_STRL("globalEnableProfie"), ZEND_ACC_PRIVATE|ZEND_ACC_STATIC);


    zend_declare_property_long(xmysql_db_ce, ZEND_STRL("_errNo"), 0, ZEND_ACC_PRIVATE);
    zend_declare_property_string(xmysql_db_ce, ZEND_STRL("_errMsg"), "", ZEND_ACC_PRIVATE);
    zend_declare_property_string(xmysql_db_ce, ZEND_STRL("_dbName"), "", ZEND_ACC_PRIVATE);
    zend_declare_property_null(xmysql_db_ce, ZEND_STRL("_sqlCond"), ZEND_ACC_PRIVATE);
    
    zend_declare_property_bool(xmysql_db_ce, ZEND_STRL("_enableProfile"), 1, ZEND_ACC_PRIVATE);
    zend_declare_property_double(xmysql_db_ce, ZEND_STRL("_lastQueryTime"), 0, ZEND_ACC_PRIVATE);
    
    zend_declare_property_null(xmysql_db_ce, ZEND_STRL("_lastQueryDb"), ZEND_ACC_PRIVATE);
    zend_declare_property_string(xmysql_db_ce, ZEND_STRL("_lastSql"), "", ZEND_ACC_PRIVATE);

    zend_declare_property_bool(xmysql_db_ce, ZEND_STRL("_inTx"), 0, ZEND_ACC_PRIVATE);

    return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(xmysql_db) {
    //TODO nothing
}