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

#include "php.h"
#include "php_ini.h"
#include "ext/standard/php_array.h"
#include "php_xmysql.h"
#include "src/xmysql_define.h"
#include "src/xmysql_common.h"
#include "src/xmysql_cond.h"

#define QUERY_TYPE_READ 1
#define QUERY_TYPE_WRITE 2

#define MAX_STR_LEN 128

zend_class_entry *xmysql_cond_ce;

//方法声明
PHP_METHOD(xmysql_cond, table) {
    zend_string *table;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), "S", &table) == FAILURE) {
        RETURN_NULL();
    }
    
    zval cond;
    zval params[1];
    ZVAL_NEW_STR(&params[0], table);
    object_init_ex(&cond, xmysql_cond_ce);
    
    air_call_object_method(&cond, xmysql_cond_ce, "__construct", NULL, 1, params);
    
    RETURN_ZVAL(&cond, 1, 0);
    
    zval_ptr_dtor(&params[0]);
    zval_ptr_dtor(&cond);
}

zend_class_entry *get_mysqli_class_ce() {
    return  (zend_class_entry *)zend_hash_str_find_ptr(EG(class_table), ZEND_STRL("mysqli"));
}

void init_db_excaped_string(zval *db, zval *src, zval *ret) {
    zend_string *strVal = zval_get_string(src);
    if(X_IS_EMPTY_P(db)) {
        // int callret = air_call_func("mysql_real_escape_string", 1, params, &ret);
        ZVAL_STR(ret, strVal);
        zval_add_ref(ret);
    }else{
         zval params[1];
         ZVAL_STR(&params[0], strVal);
         air_call_object_method(db, get_mysqli_class_ce(), "escape_string", ret, 1, params);
    }
}

void init_in_cond(zval *mysqli, zval *values, zval *ret) {
    zend_ulong hashIndex;
    zval *hashData;
    zend_string *hashKey;
    
    zval conds;
    array_init(&conds);
  
    zval escapeStr;

    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(values), hashIndex, hashKey, hashData){
        char *str = NULL;
        //TODO 处理而二进制
        zval  escapeStr;
        init_db_excaped_string(mysqli, hashData, &escapeStr);
        zend_ulong len = Z_STRLEN(escapeStr) + 2;
        spprintf(&str, len, "'%s'", Z_STRVAL(escapeStr));
        add_index_string(&conds, hashIndex, str);
        efree(str);
        zval_ptr_dtor(&escapeStr);
    } ZEND_HASH_FOREACH_END();

    zend_string *comma = zend_string_init(ZEND_STRL(","), 0);
    php_implode(comma, &conds, ret);
    zend_string_release(comma);

    zval_ptr_dtor(&conds);
}

void init_equal_cond(zval *mysqli, zval *params, zval *ret) {
    zend_ulong hashIndex;
    zval *hashData;
    zend_string *hashKey;
    
    zval conds;
    array_init(&conds);
     
    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(params), hashIndex, hashKey, hashData){
        char *str = NULL;
        //TODO 处理而二进制
        zval escapeStr;
        init_db_excaped_string(mysqli, hashData, &escapeStr);

        zend_ulong totalLen = Z_STRLEN(escapeStr)+ZSTR_LEN(hashKey)+5;
        spprintf(&str, totalLen, "`%s`='%s'", ZSTR_VAL(hashKey), Z_STRVAL(escapeStr));
        add_index_string(&conds, hashIndex, str);
        
        zval_ptr_dtor(&escapeStr);
        efree(str);
    } ZEND_HASH_FOREACH_END();

    
    zend_string *comma = zend_string_init(ZEND_STRL(","), 0);
    php_implode(comma, &conds, ret);
   
    zend_string_release(comma);
    zval_ptr_dtor(&conds);
}

void init_set_fields(zval *db, zval *fields, zval *ret) {
    init_equal_cond(db, fields, ret);
}

PHP_METHOD(xmysql_cond, equalCond) {
    zval *params = NULL;
    zval *mysqli = NULL;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|z", &params, &mysqli)) {
        RETURN_STRING("");
    }

    zval ret;
    init_equal_cond(mysqli, params, &ret);
    RETURN_ZVAL(&ret, 1, 0);
    zval_ptr_dtor(&ret);
}

PHP_METHOD(xmysql_cond, inCond) {
    zval *params = NULL;
    zval *mysqli = NULL;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|z", &params, &mysqli)) {
        RETURN_STRING("");
    }
    zval ret;
    init_in_cond(mysqli, params, &ret);
    RETURN_ZVAL(&ret, 1, 0);
    zval_ptr_dtor(&ret);    
}


//构造函数
PHP_METHOD(xmysql_cond, __construct) {
    zend_string *table = NULL;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), "S", &table) == FAILURE) {
        return ;
    }
    
    zend_update_property_string(xmysql_cond_ce, getThis(), ZEND_STRL("table"), ZSTR_VAL(table));

    zval condFd, condVal, orderFd, orderVal;
    ZVAL_STRING(&condFd, "conds");
    array_init(&condVal);

    ZVAL_STRING(&orderFd, "orders");
    array_init(&orderVal);

    zend_update_property_ex(xmysql_cond_ce, getThis(), Z_STR(condFd), &condVal);
    zend_update_property_ex(xmysql_cond_ce, getThis(), Z_STR(orderFd), &orderVal);

    zval_ptr_dtor(&condFd);
    zval_ptr_dtor(&condVal);
    zval_ptr_dtor(&orderFd);
    zval_ptr_dtor(&orderVal);
}

PHP_METHOD(xmysql_cond, select) {
    zval *fields;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "z", &fields)) {
        RETURN_FALSE;
    }
   
    zval *this = getThis();
    zend_update_property_string(xmysql_cond_ce, this, ZEND_STRL("oper"), "SELECT");
    zend_update_property_string(xmysql_cond_ce, this, ZEND_STRL("_sql"), "");
    zend_update_property_long(xmysql_cond_ce, this, ZEND_STRL("queryType"), QUERY_TYPE_READ);

    zval fd;
    ZVAL_STRING(&fd, "fields");
    zend_update_property_ex(xmysql_cond_ce, this,Z_STR(fd), fields);
    zval_ptr_dtor(&fd);

    X_RETURN_THIS;
}

PHP_METHOD(xmysql_cond, insert) {
    zval *fields;
    zend_ulong ignoreInsert = 0;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "z|l", &fields, &ignoreInsert)){
        RETURN_FALSE;
    }

    zval *this = getThis();
    zend_update_property_long(xmysql_cond_ce, this, ZEND_STRL("queryType"), QUERY_TYPE_WRITE);
    
    if(ignoreInsert) {
        zend_update_property_string(xmysql_cond_ce, this, ZEND_STRL("oper"), "INSERT IGNORE INTO");
    }else{
        zend_update_property_string(xmysql_cond_ce, this, ZEND_STRL("oper"), "INSERT INTO");
    }

    zval fd;
    ZVAL_STRING(&fd, "fields");
    zend_update_property_ex(xmysql_cond_ce, this, Z_STR(fd), fields);
    zval_ptr_dtor(&fd);

    zend_update_property_string(xmysql_cond_ce, this, ZEND_STRL("_sql"), "");

    X_RETURN_THIS;
}

PHP_METHOD(xmysql_cond, update) {
    zval *fields;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "z", &fields)) {
        RETURN_FALSE;
    }

    zval *this = getThis();
    zend_update_property_long(xmysql_cond_ce, this, ZEND_STRL("queryType"), QUERY_TYPE_WRITE);
    zend_update_property_string(xmysql_cond_ce, this, ZEND_STRL("_sql"), "");
    zend_update_property(xmysql_cond_ce, this, ZEND_STRL("fields"), fields);
    zend_update_property_string(xmysql_cond_ce, this, ZEND_STRL("oper"), "UPDATE");

    X_RETURN_THIS;
}


PHP_METHOD(xmysql_cond, del) {
    zval *this = getThis();
    zend_update_property_long(xmysql_cond_ce, this, ZEND_STRL("queryType"), QUERY_TYPE_WRITE);
    zend_update_property_string(xmysql_cond_ce, this, ZEND_STRL("_sql"), "");
    zend_update_property_string(xmysql_cond_ce, this, ZEND_STRL("oper"), "DELETE");
    zend_update_property_null(xmysql_cond_ce, this, ZEND_STRL("fields"));
    X_RETURN_THIS;
}

void addCondtion(zval *obj, zend_string *key, zval *val,  char *op,  char *condStr) {
    zval cond;
    array_init(&cond);
    zend_string_addref(key);
    add_assoc_str(&cond, "k", key);

    zval_add_ref(val);
    add_assoc_zval(&cond, "v", val);

    if(op && op!="") {
        add_assoc_string(&cond, "op", op);
    }else{
        add_assoc_string(&cond, "op", "=");
    }

    add_assoc_string(&cond, "cond", condStr);

    zval rv;
    zval *conds = zend_read_property(xmysql_cond_ce, obj,ZEND_STRL("conds"), 0, &rv TSRMLS_DC);
    zend_ulong num = zend_array_count(Z_ARRVAL_P(conds));
  
    add_index_zval(conds, num, &cond);
}

PHP_METHOD(xmysql_cond, andc) {
    zval *val;
    zend_string *key = NULL, *op = NULL;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sz|S", &key, &val, &op)) {
        RETURN_FALSE;
    }

    addCondtion(getThis(), key, val, op?ZSTR_VAL(op):"=", "AND");
    
    X_RETURN_THIS;
}

PHP_METHOD(xmysql_cond, orc) {
    zval *val;
    zend_string *key = NULL, *op = NULL;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sz|S", &key, &val, &op)) {
        RETURN_FALSE;
    }
    addCondtion(getThis(), key, val, op?ZSTR_VAL(op):"=", "OR");
    X_RETURN_THIS;
}

PHP_METHOD(xmysql_cond, equal) {
    zval *params;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "a", &params)) {
        RETURN_FALSE;
    }

    zend_ulong hashIndex;
    zval *hashData;
    zend_string *hashKey;
    zval *this  = getThis();
    ZEND_HASH_FOREACH_KEY_VAL(Z_ARR_P(params), hashIndex, hashKey, hashData) {
        addCondtion(this, hashKey, hashData, "=", "AND");
    }ZEND_HASH_FOREACH_END();
    X_RETURN_THIS;
}

PHP_METHOD(xmysql_cond, in) {
    zend_string *key = NULL, *cond = NULL;
    zval *val;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sa|S", &key, &val, &cond)) {
        RETURN_FALSE;
    }

    addCondtion(getThis(), key, val, "IN", cond ? ZSTR_VAL(cond) : "AND");

    X_RETURN_THIS;
}

PHP_METHOD(xmysql_cond, notIn) {
    zend_string *key = NULL, *cond = NULL;
    zval *val;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sa|S", &key, &val, &cond)) {
        RETURN_FALSE;
    }

    addCondtion(getThis(), key, val, "NOT IN", cond ? ZSTR_VAL(cond):"AND");

    X_RETURN_THIS;
}

PHP_METHOD(xmysql_cond, order) {
    zval *args;
    zend_ulong argc = 0;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "+", &args, &argc)) {
        RETURN_FALSE;
    }
    zval param;
    zval *orders = zend_read_property(xmysql_cond_ce, getThis(), ZEND_STRL("orders"), 0, &param);
    zend_ulong idx = zend_array_count(Z_ARR_P(orders));

    char orderStr[50] = "";
    int strLen = 47;
    switch(argc){
        case 1:
            if(Z_TYPE_P(&args[0]) == IS_ARRAY) {
                zval order;
                ZVAL_COPY(&order, &args[0]);
                
                zend_ulong hashIdx;
                zend_string *key;
                zval *val;

                ZEND_HASH_FOREACH_KEY_VAL(Z_ARR(args[0]), hashIdx, key, val) {
                    snprintf(orderStr, strLen, "`%s` %s", ZSTR_VAL(key), Z_STRVAL_P(val));
                    add_index_string(orders, idx++, orderStr);
                }ZEND_HASH_FOREACH_END();
            }else if(Z_TYPE_P(&args[0]) == IS_STRING){
                snprintf(orderStr, strLen, "`%s`", Z_STRVAL(args[0]));
                add_index_string(orders, idx, orderStr);
            }else{
                php_error(E_NOTICE, "xmysql_cond->xmysql_cond wrong argument \n");
            }
        break;
        case 2:{
            snprintf(orderStr, strLen, "`%s` %s", Z_STRVAL(args[0]), Z_STRVAL(args[1]));
            add_index_string(orders, idx, orderStr);
        }
        break;
        default:
        php_error(E_NOTICE, "xmysql_cond->xmysql_cond wrong argument number:%d\n", argc);
    }

    X_RETURN_THIS;
}

PHP_METHOD(xmysql_cond, limit) {
    zval *args;
    zend_ulong argc = 0;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "+", &args, &argc)) {
        RETURN_FALSE;
    }
    char str[50] = "";
    int len = 49;
   
    switch(argc){
        case 1:
           snprintf(str, len, " LIMIT %d",  zval_get_long(&args[0]));
           php_printf("limit str:%s \n", str);
        break;
        case 2:
            snprintf(str, len," LIMIT %d,%d", zval_get_long(&args[0]), zval_get_long(&args[1]));
        break;
        default:
        php_error(E_NOTICE, "xmysql_cond->limit wrong argument number:%d\n", argc);
        break;
    }

    zend_update_property_string(xmysql_cond_ce, getThis(), ZEND_STRL("page"), str);
    X_RETURN_THIS;
}

PHP_METHOD(xmysql_cond, getQueryType) {
    zval rv;

    zval *queryType = zend_read_property(xmysql_cond_ce, getThis(), ZEND_STRL("queryType"), 0, &rv);
    RETURN_ZVAL(queryType, 1, 0);
}

PHP_METHOD(xmysql_cond, sql) {
    zval *mysqli = NULL;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|z", &mysqli)) {
        RETURN_FALSE;
    }
    
    zval rv;
    zval *this = getThis();
    zval *sql = zend_read_property(xmysql_cond_ce, this, ZEND_STRL("_sql"), 0, &rv);
    if(Z_STRLEN_P(sql) > 0) {
        RETURN_ZVAL(sql, 1, 0);
        return ;
    }

    zval *oper = zend_read_property(xmysql_cond_ce, this, ZEND_STRL("oper"), 0, &rv);
    zval *fields = zend_read_property(xmysql_cond_ce, this, ZEND_STRL("fields"), 0, &rv);
    zval *conds = zend_read_property(xmysql_cond_ce, this, ZEND_STRL("conds"), 0, &rv);
    zval *orders = zend_read_property(xmysql_cond_ce, this, ZEND_STRL("orders"), 0, &rv);
    zval *page = zend_read_property(xmysql_cond_ce, this, ZEND_STRL("page"), 0, &rv);
    zval *table = zend_read_property(xmysql_cond_ce, this, ZEND_STRL("table"), 0, &rv);
   
    zend_ulong idx = 0;
    zval parts;
    array_init(&parts);
    char *ret = NULL;

    if(X_ZSTR_EQUAL(oper, "SELECT") || X_ZSTR_EQUAL(oper, "DELETE")) {
        int len = Z_STRLEN_P(oper)+Z_STRLEN_P(fields)+Z_STRLEN_P(table)+9;
        char *str = NULL;
        spprintf(&str, len, "%s %s FROM `%s`", Z_STRVAL_P(oper), Z_STRVAL_P(fields), Z_STRVAL_P(table));
        add_index_string(&parts, idx++, str);
        efree(str);
    }else if(X_ZSTR_EQUAL(oper, "UPDATE")) {
        zval setFields;
        init_set_fields(mysqli, fields, &setFields);
    
        int len = Z_STRLEN_P(oper)+Z_STRLEN_P(table)+Z_STRLEN_P(fields)+8;
        char *str = NULL;
        
        spprintf(&str, len, "%s `%s` SET %s", Z_STRVAL_P(oper), Z_STRVAL(setFields));
        add_index_string(&parts, idx++, str);

        zval_ptr_dtor(&setFields);
        efree(str);
    }else{ //INSERT
        zval setFields;
        init_set_fields(mysqli, fields, &setFields);
        int len = Z_STRLEN_P(oper)+Z_STRLEN_P(table)+Z_STRLEN_P(fields)+8;
        char *str = NULL;
        spprintf(&str, len, "%s `%s` SET %s", Z_STRVAL(setFields));
        add_index_string(&parts, idx++, str);

        zval_ptr_dtor(&setFields);
        efree(str);
    }

    uint32_t condCnt = zend_array_count(Z_ARRVAL_P(conds));
    if(condCnt) {
        add_index_string(&parts, idx++, " WHERE ");
        zend_ulong hashIdx = 0;
        zend_string *hashKey;
        zval *hashData;
        int len;
        ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(conds), hashIdx, hashKey, hashData){
            char *strCond = NULL;
            zval *k = zend_hash_str_find(Z_ARRVAL_P(hashData), ZEND_STRL("k"));
            zval *op = zend_hash_str_find(Z_ARRVAL_P(hashData), ZEND_STRL("op"));
            zval *cond = zend_hash_str_find(Z_ARRVAL_P(hashData), ZEND_STRL("cond"));
            zval *v = zend_hash_str_find(Z_ARRVAL_P(hashData), ZEND_STRL("v"));
           
            if(hashIdx == 0) {
                 len =  Z_STRLEN_P(k) + Z_STRLEN_P(op) + 2;
                spprintf(&strCond, len, "`%s`%s", Z_STRVAL_P(k), Z_STRVAL_P(op));
                add_index_string(&parts, idx++, strCond);
            }else{
                int len =  Z_STRLEN_P(k) + Z_STRLEN_P(op) + Z_STRLEN_P(cond) + 4;
                spprintf(&strCond, len, " %s `%s`%s ", Z_STRVAL_P(cond), Z_STRVAL_P(k), Z_STRVAL_P(op));
                add_index_string(&parts, idx++, strCond);
            }

            if(X_ZSTR_EQUAL(op, "IN") || X_ZSTR_EQUAL(op, "NOT IN")) {
                zval inValues;
                init_in_cond(mysqli, v, &inValues);
                len = Z_STRLEN(inValues)+3;

                spprintf(&strCond, len, " (%s)", Z_STRVAL(inValues));
                add_index_string(&parts, idx++, strCond);

                zval_ptr_dtor(&inValues);
            }else{
                zval escapedV;
                init_db_excaped_string(mysqli, v, &escapedV);
                len = Z_STRLEN(escapedV)+2;

                spprintf(&strCond, len, "'%s'", Z_STRVAL(escapedV));
                add_index_string(&parts, idx++, strCond);

                zval_ptr_dtor(&escapedV);
            }

            if(strCond) {
                efree(strCond);
            }

        }ZEND_HASH_FOREACH_END();
    }

    
    
    int orderCnt = zend_array_count(Z_ARR_P(orders));
    if(orderCnt > 0) {
        zval orderVal;
        zend_string *comma = zend_string_init(ZEND_STRL(","), 0);

        php_implode(comma, orders, &orderVal);

        add_index_string(&parts, idx++, " ORDER BY ");
        add_index_zval(&parts, idx++, &orderVal);
        //  zval_ptr_dtor(&orderVal); //加入数组，不需要释放

        zend_string_release(comma);
    }

    if(!X_ZSTR_EQUAL(page, "")){
        add_index_zval(&parts, idx++, page);
    }

    zend_string *c = zend_string_init("", 0, 0);
    zval zvalRet;
    
    php_implode(c, &parts, &zvalRet);
    zend_update_property(xmysql_cond_ce, this, ZEND_STRL("_sql"), &zvalRet);
   
    RETURN_ZVAL(&zvalRet, 1, 0);
    zend_string_release(c);
    php_debug_zval_dump(zvalRet, 0);

    zval_ptr_dtor(&zvalRet);
    zval_ptr_dtor(&parts);
}

// PHP_METHOD(xmysql_cond, escapeValue) {}



static zend_function_entry xmysql_cond_methods[] = {
	PHP_ME(xmysql_cond, table, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(xmysql_cond, equalCond, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(xmysql_cond, inCond, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(xmysql_cond, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(xmysql_cond, select, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_cond, insert, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_cond, update, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_cond, del, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_cond, andc, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_cond, orc, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_cond, equal, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_cond, in, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_cond, notIn, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_cond, order, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_cond, limit, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_cond, sql, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(xmysql_cond, getQueryType, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

//模块初始化方法
ZEND_MINIT_FUNCTION(xmysql_cond)
{
        zend_class_entry ce;
        //方法注册
        INIT_CLASS_ENTRY(ce, "xmysql_cond", xmysql_cond_methods);
        
        xmysql_cond_ce = zend_register_internal_class(&ce TSRMLS_CC);

        //属性
        zend_declare_property_string(xmysql_cond_ce, ZEND_STRL("table"), "",  ZEND_ACC_PUBLIC TSRMLS_CC);
        zend_declare_property_null(xmysql_cond_ce, ZEND_STRL("conds"), ZEND_ACC_PUBLIC TSRMLS_CC);
        zend_declare_property_null(xmysql_cond_ce, ZEND_STRL("orders"), ZEND_ACC_PUBLIC TSRMLS_CC);
        zend_declare_property_string(xmysql_cond_ce, ZEND_STRL("_sql"), "", ZEND_ACC_PUBLIC TSRMLS_CC);
        zend_declare_property_long(xmysql_cond_ce, ZEND_STRL("queryType"), 0, ZEND_ACC_PUBLIC TSRMLS_CC);
        zend_declare_property_null(xmysql_cond_ce, ZEND_STRL("page"), ZEND_ACC_PUBLIC TSRMLS_CC);
        zend_declare_property_string(xmysql_cond_ce, ZEND_STRL("oper"), "", ZEND_ACC_PUBLIC TSRMLS_CC);
        zend_declare_property_string(xmysql_cond_ce, ZEND_STRL("fields"), "*", ZEND_ACC_PUBLIC TSRMLS_CC);

        //类常量
        zend_declare_class_constant_long(xmysql_cond_ce, ZEND_STRL("QUERY_TYPE_READ"), QUERY_TYPE_READ);
        zend_declare_class_constant_long(xmysql_cond_ce, ZEND_STRL("QUERY_TYPE_WRITE"), QUERY_TYPE_WRITE);
       
        //方法
        return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(xmysql_cond) {
        //TODO
}