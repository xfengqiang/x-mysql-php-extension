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
#include "zend_smart_str_public.h"
#include "zend_smart_str.h"
#include "time.h"

#include "php_xmysql.h"
#include "src/xmysql_define.h"
#include "src/xmysql_loader.h"
#include "src/xmysql_db.h"
#include "src/xmysql_common.h"

zend_class_entry *xmysql_db_ce;

zend_class_entry *get_xmysql_cond_ce() {
    return  (zend_class_entry *)zend_hash_str_find_ptr(EG(class_table), ZEND_STRL("xmysql_cond"));
}

PHP_METHOD(xmysql_db, setGlobalCallBack){
    // zend_fcall_info f;
    // zend_fcall_info_cache f_cache;
    // if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "z", &f, &f_cache)) {
    //     return ;
    // }
    zval *f;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "z", &f)) {
        return ;
    }
    zend_update_static_property(xmysql_db_ce, ZEND_STRL("globalCallBack"), f);
}

PHP_METHOD(xmysql_db, enableGlobalProfile){
    zend_bool enable;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "b", &enable)) {
        return;
    }
    zend_update_static_property_bool(xmysql_db_ce, ZEND_STRL("globalEnableProfile"), enable);
}

PHP_METHOD(xmysql_db, __construct){
    zend_string *dbName;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &dbName)) {
        return ;
    }
    zend_update_property_str(xmysql_db_ce, getThis(), ZEND_STRL("_dbName"), dbName);
}

PHP_METHOD(xmysql_db, callback){
    zval *f;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "z", &f)) {
        return ;
    }

    zend_update_property(xmysql_db_ce, getThis(), ZEND_STRL("_callback"), f);

    X_RETURN_THIS;
}

//初始化xmysql_db->_sqlCond
void init_sql_cond(zval *obj, const char *method, const char * table, zend_ulong paramNum, zval params[]) {
    zval cond;
    
    zval tableParam[1];
    ZVAL_STRING(&tableParam[0], table);
    air_call_func("xmysql_cond::table", 1, tableParam, &cond);
    zval_ptr_dtor(&tableParam[0]);

    //  
    zend_class_entry *cond_ce = get_xmysql_cond_ce();
    air_call_object_method(&cond, cond_ce, method, NULL, paramNum, params);
  
    zend_update_property(xmysql_db_ce, obj, ZEND_STRL("_sqlCond"), &cond);

    zval_ptr_dtor(&cond);
}

PHP_METHOD(xmysql_db, select){
    zend_string *table = NULL, *fields = NULL;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S|S", &table, &fields)) {
        RETURN_FALSE;
    }

    //使用这种方式调用会报错，提示 Couldn't find implementation for method xmysql_cond::select
    zval params[1];
    if(fields) {
        ZVAL_STR(&params[0], fields);
    }
  
    init_sql_cond(getThis(), "select", ZSTR_VAL(table), fields ? 1 : 0, params);

    X_RETURN_THIS;
}

PHP_METHOD(xmysql_db, insert){
    zend_string *table = NULL;
    zval *data = NULL;
    zend_bool ignore = 0;

    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sa|b", &table, &data, &ignore)) {
        RETURN_FALSE;
    }
    php_printf("db insert data \n");
    php_debug_zval_dump(data, 0);
    
    zval params[2];
    ZVAL_ZVAL(&params[0], data, 0, 0);
    ZVAL_BOOL(&params[1], ignore);

    init_sql_cond(getThis(), "insert", ZSTR_VAL(table), 2, params);

    X_RETURN_THIS;
}

PHP_METHOD(xmysql_db, update){
    zend_string *table = NULL;
    zval *data = NULL;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sa", &table, &data)) {
        RETURN_FALSE;
    }
    php_printf("db update data \n");
    zval_add_ref(data);
    php_debug_zval_dump(data, 0);
    zval params[1];
    ZVAL_ZVAL(&params[0], data, 0, 0);
    init_sql_cond(getThis(), "update", ZSTR_VAL(table), 1, params);

    X_RETURN_THIS;
}

PHP_METHOD(xmysql_db, del){
    zend_string *table = NULL;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &table)) {
        RETURN_FALSE;
    }
    
    init_sql_cond(getThis(), "del", ZSTR_VAL(table), 0, NULL);

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
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sz|S", &key, &v, &op)) {
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

//判断src是否包含前缀prefix
zend_bool xmysql_db_str_hashPrefix(const char *src, int srcLen, const char *prefix, int prefixLen) {
    int i = 0;
    while(i<srcLen && i<prefixLen) {
        if(src[i] == ' ' || src[i] == '\n' || src[i] == '\r' || src[i] == '\t' || src[i] == '\v'){
            //跳过空白字符
            continue;
        }
        if(toupper(src[i]) != prefix[i]) {
            return 0;
        }
        i++;
    }
    return 1;
}

zend_ulong xmysql_db_get_query_type(zend_string *sql, zend_ulong type) {
    if(type != QUERY_TYPE_AUTO) {
        return type;
    }
    zend_ulong ret = 0;
    size_t sqlLen = strlen(sql);
    if(xmysql_db_str_hashPrefix(sql, sqlLen, ZEND_STRL("SELECT")) || 
    xmysql_db_str_hashPrefix(sql, sqlLen, ZEND_STRL("SHOW"))) {
        ret = QUERY_TYPE_READ;
    }else{
        ret = QUERY_TYPE_WRITE;
    }
    return ret;
}

int xmysql_db_get_db(zval *obj, zend_ulong type, zval *mysqli) {
    zval rv; 
    zval *inTxVal = zend_read_property(xmysql_db_ce, obj, ZEND_STRL("_inTx"), 0, &rv);
    zend_bool inTx = Z_LVAL_P(inTxVal);

    if(inTx) {
        zval *lastDb = zend_read_property(xmysql_db_ce, obj, ZEND_STRL("_lastQueryDb"), 0, &rv);
        if(!X_IS_EMPTY_P(lastDb)) {
            ZVAL_ZVAL(mysqli, lastDb, 0, 0);
              return 1;
        }
        type = QUERY_TYPE_WRITE;
    }

    zval *dbName = zend_read_property(xmysql_db_ce, obj, ZEND_STRL("_dbName"), 0, &rv);
    zval params[2];
    ZVAL_ZVAL(&params[0], dbName, 0, 0);
    ZVAL_LONG(&params[1], type);
    air_call_func("xmysql_loader::getDb", 2, params, mysqli);
    return 1;
} 

void xmysql_db_call_callback(zval *obj, zval *callback, zval *mysqli, zend_string *sql) {
    zval params[3];
    ZVAL_ZVAL(&params[0], obj, 0, 0);
    ZVAL_ZVAL(&params[1], mysqli, 0, 0);
    ZVAL_STR(&params[2], sql);
    if(Z_TYPE_P(callback) == IS_STRING) {   
        air_call_func(Z_STRVAL_P(callback), 3, params, NULL);
    }else if(Z_TYPE_P(callback) == IS_ARRAY){
        zval *obj = zend_hash_index_find(Z_ARR_P(callback), 0);
        zval *method = zend_hash_index_find(Z_ARR_P(callback), 1);
        if(Z_TYPE_P(obj) == IS_OBJECT && Z_TYPE_P(method) == IS_STRING) {
            air_call_object_method(obj, Z_OBJCE_P(obj), Z_STRVAL_P(method), NULL, 3, params);
        }else{
            smart_str buf = {0};
	        php_var_export_ex(callback, 0, &buf);
	        smart_str_0(&buf);
	        php_error(E_WARNING, "invalid call back argument %s", ZSTR_VAL(buf.s));
	        smart_str_free(&buf);
           
        }
    
    }
}

static zend_always_inline double getTime() {
    double retV = 0.0;
    zval ret;
	zval params[1];
    ZVAL_BOOL(&params[0], 1);
    
    air_call_func("microtime", 1, params, &ret);
    
    retV = Z_DVAL(ret);
    zval_ptr_dtor(&ret);

    return retV;
}

void xmysql_db_query(zval *obj, zend_string *sql, zend_ulong type, zend_bool isRow, zval *ret) {
    zend_bool releaseSql = 0;
    zval mysqli;
    zend_ulong dbType = 0;

    if(!sql || ZSTR_LEN(sql) == 0) {
        zval rv;
        zval *sqlCond = zend_read_property(xmysql_db_ce, obj, ZEND_STRL("_sqlCond"), 0, &rv);
        if(X_IS_EMPTY_P(sqlCond)) {
            zend_update_property_string(xmysql_db_ce, obj, ZEND_STRL("_errMsg"), "No sql condition was given");
            zend_update_property_long(xmysql_db_ce, obj, ZEND_STRL("_errNo"), -2);
            ZVAL_BOOL(ret, 0);
            return ;
        }
       
        if(type == QUERY_TYPE_AUTO) {
            zval retVal;
            air_call_object_method(sqlCond, get_xmysql_cond_ce(), "getquerytype", &retVal, 0, NULL);
            //这里必须用小写的
            // air_call_object_method(sqlCond, get_xmysql_cond_ce(), "getQueryType", &retVal, 0, NULL);
            dbType = Z_LVAL(retVal);
            zval_ptr_dtor(&retVal);
        }

        xmysql_db_get_db(obj, type, &mysqli);
        zval sqlVal;
        zval params[1];
        ZVAL_ZVAL(&params[0], &mysqli, 0, NULL);

        air_call_object_method(sqlCond, get_xmysql_cond_ce(), "sql", &sqlVal, 1, params);
        
        sql = zend_string_copy(Z_STR(sqlVal));
        releaseSql = 1;
       
        // zval_ptr_dtor(&sqlVal);   
    }else{
        dbType = xmysql_db_get_query_type(sql, type);
        xmysql_db_get_db(obj, dbType, &mysqli);
    }
 
    zend_update_property_string(xmysql_db_ce, obj, ZEND_STRL("_errMsg"), "");
    zend_update_property_long(xmysql_db_ce, obj, ZEND_STRL("_errNo"), 0);
    zend_update_property(xmysql_db_ce, obj, ZEND_STRL("_lastQueryDb"), &mysqli);
 
    zval rv;
    zval *gProfileEnabled = zend_read_static_property(xmysql_db_ce, ZEND_STRL("globalEnableProfile"), 0);
    zval *profileEnabled = zend_read_property(xmysql_db_ce, obj, ZEND_STRL("_enableProfile"), 0, &rv);
    zend_bool enableProfile = Z_TYPE_P(gProfileEnabled)==IS_TRUE || Z_TYPE_P(profileEnabled)==IS_TRUE;

    double  startTime, endTime;
    if(enableProfile) {
        startTime = getTime();
    }
    
    zend_update_property_string(xmysql_db_ce, obj, ZEND_STRL("_lastSql"), ZSTR_VAL(sql));

    zval queryParam[1];
    ZVAL_STR(&queryParam[0], sql);
   
    zval sqlRet;
    air_call_object_method(&mysqli, get_mysqli_class_ce(), "query", &sqlRet, 1, queryParam);
    
    if(Z_TYPE(sqlRet) != IS_OBJECT){
        ZVAL_ZVAL(ret, &sqlRet, 1, 0);
    }else{
         if(isRow){
            air_call_object_method(&sqlRet, get_mysqli_result_class_ce(), "fetch_assoc", ret, 0, NULL);
         }else{
            zval fetchParam[1];
            ZVAL_LONG(&fetchParam[0], 1);
            air_call_object_method(&sqlRet, get_mysqli_result_class_ce(), "fetch_all", ret, 1, fetchParam);
         }
    }
    zval_ptr_dtor(&sqlRet);
    if(enableProfile) {
        endTime = getTime();
        zend_update_property_double(xmysql_db_ce, obj, ZEND_STRL("_lastQueryTime"), endTime-startTime);
    }
    
    zval *globalCallBack = zend_read_static_property(xmysql_db_ce, ZEND_STRL("globalCallBack"), 0);
    xmysql_db_call_callback(obj, globalCallBack, &mysqli, sql);
    zval *classCallback  = zend_read_property(xmysql_db_ce, obj, ZEND_STRL("_callback"), 0, &rv);
    xmysql_db_call_callback(obj, classCallback, &mysqli, sql);

    zval_ptr_dtor(&queryParam[0]);
    if(releaseSql) {
        zend_string_release(sql);
    }
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


PHP_METHOD(xmysql_db, exec){
    zend_string *sql = NULL;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &sql)) {
        RETURN_FALSE;
    }

    zval ret;
    xmysql_db_query(getThis(), sql, DB_TYPE_MASTER, 0, &ret);
    RETURN_ZVAL(&ret, 0, 0);
}

PHP_METHOD(xmysql_db, queryByCond){
    zval *cond;
    zend_long type = DB_TYPE_AUTO;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "z|l", &cond, &type)) {
        RETURN_FALSE;
    }

    zend_update_property(xmysql_db_ce, getThis(), ZEND_STRL("_sqlCond"), cond);

    zval ret;
    xmysql_db_query(getThis(), NULL, type, 0, &ret);
    RETURN_ZVAL(&ret, 0, 0);
}

PHP_METHOD(xmysql_db, queryRowByCond){
    zval *cond;
    zend_long type = DB_TYPE_AUTO;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "z|l", &cond, &type)) {
        RETURN_FALSE;
    }

    zend_update_property(xmysql_db_ce, getThis(), ZEND_STRL("_sqlCond"), cond);

    zval ret;
    xmysql_db_query(getThis(), NULL, type, 1, &ret);
    RETURN_ZVAL(&ret, 0, 0);
}

PHP_METHOD(xmysql_db, query){
    zend_string *sql = NULL;
    zend_long queryType = 0;
    zend_bool isRow = 0;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|Slb", &sql, &queryType, &isRow)) {
        RETURN_FALSE;
    }
    
    zval ret;
    xmysql_db_query(getThis(), sql, queryType, isRow, &ret);
    RETURN_ZVAL(&ret, 0, 0);
    zval_ptr_dtor(&ret);
}

PHP_METHOD(xmysql_db, queryRow){
    zend_string *sql = NULL;
    zend_long  type = DB_TYPE_AUTO;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|Sl", &sql, &type)) {
        RETURN_FALSE;
    }

    zval ret;
    xmysql_db_query(getThis(), sql, type, 1, &ret);
    RETURN_ZVAL(&ret, 0, 0);
}

// PHP_METHOD(xmysql_db, getQueryType){}

PHP_METHOD(xmysql_db, lastErrorCode){
    zval rv;
    zval *errNo = NULL;
    zval *lastQueryDb = zend_read_property(xmysql_db_ce, getThis(), ZEND_STRL("_lastQueryDb"), 0, &rv);
    if(X_IS_EMPTY_P(lastQueryDb)) {
        errNo = zend_read_property(xmysql_db_ce, getThis(), ZEND_STRL("_errNo"), 0, &rv);
    }else{
        errNo = zend_read_property(get_mysqli_class_ce(), lastQueryDb,  ZEND_STRL("errno"), 0, &rv);
    }
    RETURN_ZVAL(errNo, 1, 0);
}

PHP_METHOD(xmysql_db, lastErrorMsg){
    zval rv;
    zval *errMsg = NULL;
    zval *lastQueryDb = zend_read_property(xmysql_db_ce, getThis(), ZEND_STRL("_lastQueryDb"), 0, &rv);
    if(X_IS_EMPTY_P(lastQueryDb)) {
        errMsg = zend_read_property(xmysql_db_ce, getThis(), ZEND_STRL("_errMsg"), 0, &rv);
    }else{
        errMsg = zend_read_property(get_mysqli_class_ce(), lastQueryDb, ZEND_STRL("error"), 0, &rv);
    }
    RETURN_ZVAL(errMsg, 1, 0);
}

PHP_METHOD(xmysql_db, lastSql){
    zval rv;
    zval *lastSql = zend_read_property(xmysql_db_ce, getThis(), ZEND_STRL("_lastSql"), 0, &rv);
    RETURN_ZVAL(lastSql, 1, 0);
}

PHP_METHOD(xmysql_db, rowsAffected){
    zval rv;
    zval *lastQueryDb = zend_read_property(xmysql_db_ce, getThis(), ZEND_STRL("_lastQueryDb"), 0, &rv);
    if(X_IS_EMPTY_P(lastQueryDb)) {
        RETURN_LONG(0);
    }else{
        zval *ret = zend_read_property(xmysql_db_ce, getThis(), ZEND_STRL("affected_rows"), 0, &rv);
        RETURN_ZVAL(ret, 1, 0);
    }
}

PHP_METHOD(xmysql_db, lastInsertId){
    zval rv;
    zval *lastQueryDb = zend_read_property(xmysql_db_ce, getThis(), ZEND_STRL("_lastQueryDb"), 0, &rv);
    if(X_IS_EMPTY_P(lastQueryDb)) {
        RETURN_FALSE;
    }else{
        zval *ret = zend_read_property(get_mysqli_class_ce(), lastQueryDb, ZEND_STRL("insert_id"), 0, &rv);
        RETURN_ZVAL(ret, 1, 0);
    }
}

PHP_METHOD(xmysql_db, lastQueryTime){
    zval rv;
    zval *ret = zend_read_property(xmysql_db_ce, getThis(), ZEND_STRL("_lastQueryTime"), 0, &rv);
    RETURN_ZVAL(ret, 1, 0);
}

PHP_METHOD(xmysql_db, getDbName){
    zval rv;
    zval *ret = zend_read_property(xmysql_db_ce, getThis(), ZEND_STRL("_dbName"), 0, &rv);
    RETURN_ZVAL(ret, 1, 0);
}

PHP_METHOD(xmysql_db, isInTx){
    zval rv;
    zval *ret = zend_read_property(xmysql_db_ce, getThis(), ZEND_STRL("_inTx"), 0, &rv);
    RETURN_ZVAL(ret, 1, 0);
}

PHP_METHOD(xmysql_db, startTx){
    zval rv;
    zval *inTx = zend_read_property(xmysql_db_ce, getThis(), ZEND_STRL("_inTx"), 0, &rv);
    if (Z_TYPE_INFO_P(inTx) == IS_TRUE){
        RETURN_FALSE;
        return ;
    }

    zval mysqli;
    zval *dbName = zend_read_property(xmysql_db_ce, getThis(), ZEND_STRL("_dbName"), 0, &rv);
    zval params[2];
    ZVAL_ZVAL(&params[0], dbName, 0, 0);
    ZVAL_LONG(&params[1], DB_TYPE_MASTER);
    air_call_func("xmysql_loader::getDb", 2, params, &mysqli);

    if(Z_TYPE(mysqli) == IS_NULL){
        RETURN_FALSE;
        return ;
    }

    zval ret;
    air_call_object_method(&mysqli, get_mysqli_class_ce(), "begin_transaction", &ret, 0, NULL);
    zend_update_property(xmysql_db_ce, getThis(), ZEND_STRL("_lastQueryDb"), &mysqli);
    zend_update_property(xmysql_db_ce, getThis(),ZEND_STRL("_inTx"), &ret);
    RETURN_ZVAL(&ret, 1, 0);
    zval_ptr_dtor(&ret);
    RETURN_ZVAL(&ret, 0, 0);
}

PHP_METHOD(xmysql_db, commitTx){
    zval rv;
    zval *inTx = zend_read_property(xmysql_db_ce, getThis(), ZEND_STRL("_inTx"), 0, &rv);
    if (Z_TYPE_INFO_P(inTx) == IS_FALSE){
        RETURN_FALSE;
        return ;
    }

    zval *db = zend_read_property(xmysql_db_ce, getThis(), ZEND_STRL("_lastQueryDb"), 0, &rv);
    zval ret;
    air_call_object_method(db, get_mysqli_class_ce(), "commit", &ret, 0, NULL);
    zend_update_property_bool(xmysql_db_ce, getThis(), ZEND_STRL("_inTx"), 0);
    RETURN_ZVAL(&ret, 1, 0);
    zval_ptr_dtor(&ret);
}

PHP_METHOD(xmysql_db, rollbackTx){
     zval rv;
    zval *inTx = zend_read_property(xmysql_db_ce, getThis(), ZEND_STRL("_inTx"), 0, &rv);
    if (Z_TYPE_INFO_P(inTx) == IS_FALSE){
        RETURN_FALSE;
        return ;
    }

    zval *db = zend_read_property(xmysql_db_ce, getThis(), ZEND_STRL("_lastQueryDb"), 0, &rv);
    zval ret;
    air_call_object_method(db, get_mysqli_class_ce(), "rollback", &ret, 0, NULL);
    zend_update_property_bool(xmysql_db_ce, getThis(), ZEND_STRL("_inTx"), 0);
    RETURN_ZVAL(&ret, 1, 0);
    zval_ptr_dtor(&ret);   
}

static zend_function_entry xmysql_db_methods[] = {
    PHP_ME(xmysql_db, setGlobalCallBack, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(xmysql_db, enableGlobalProfile, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
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

    PHP_ME(xmysql_db, lastErrorCode, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, lastErrorMsg, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, lastSql, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, rowsAffected, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, lastInsertId, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_db, lastQueryTime, NULL, ZEND_ACC_PUBLIC)
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
    zend_declare_property_bool(xmysql_db_ce, ZEND_STRL("globalEnableProfile"), 0, ZEND_ACC_PRIVATE|ZEND_ACC_STATIC);


    zend_declare_property_long(xmysql_db_ce, ZEND_STRL("_errNo"), 0, ZEND_ACC_PRIVATE);
    zend_declare_property_string(xmysql_db_ce, ZEND_STRL("_errMsg"), "", ZEND_ACC_PRIVATE);
    zend_declare_property_string(xmysql_db_ce, ZEND_STRL("_dbName"), "", ZEND_ACC_PRIVATE);
    zend_declare_property_null(xmysql_db_ce, ZEND_STRL("_sqlCond"), ZEND_ACC_PRIVATE);
    
    zend_declare_property_null(xmysql_db_ce, ZEND_STRL("_callback"), ZEND_ACC_PRIVATE);
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