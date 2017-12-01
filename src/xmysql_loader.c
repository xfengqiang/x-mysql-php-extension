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
#include "Zend/zend_interfaces.h"
#include "Zend/zend_smart_str.h"
#include "php_xmysql.h"

#include "src/xmysql_define.h"
#include "src/xmysql_loader.h"
#include "src/xmysql_common.h"

zend_class_entry *xmysql_loader_ce;

zval *xmysql_close_all_db() ;
//方法声明
PHP_METHOD(xmysql_loader, registerDb);
PHP_METHOD(xmysql_loader, registerDbs);
PHP_METHOD(xmysql_loader, getDbConfig);
PHP_METHOD(xmysql_loader, getAllConfig);
PHP_METHOD(xmysql_loader, getDb);
PHP_METHOD(xmysql_loader, test);

//参数声明
/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(xmysql_loader_getAllConfig, 0, 0, 0)
ZEND_END_ARG_INFO()

//ZEND_BEGIN_ARG_INFO_EX(xmysql_loader_getDbConfig, 0, 0, 0)
//ZEND_ARG_INFO(0, key)
//ZEND_END_ARG_INFO()
//
//ZEND_BEGIN_ARG_INFO_EX(xmysql_loader_registerDb, 0, 0, 1)
//ZEND_ARG_INFO(0, name)
//ZEND_ARG_INFO(0, config)
//ZEND_END_ARG_INFO()
/* }}} */

static zend_function_entry xmysql_loader_methods[] = {
	PHP_ME(xmysql_loader, registerDb, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(xmysql_loader, registerDbs, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(xmysql_loader, getDbConfig, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(xmysql_loader, getDb, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(xmysql_loader, test, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

ZEND_MINIT_FUNCTION(xmysql_loader){
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "xmysql_loader", xmysql_loader_methods);

    xmysql_loader_ce  = zend_register_internal_class(&ce TSRMLS_CC);

    //属性
    zend_declare_property_null(xmysql_loader_ce, "dbCache", strlen("dbCache"), ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
    zend_declare_property_null(xmysql_loader_ce, "dbConfig", strlen("dbConfig"), ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
 	zend_declare_property_null(xmysql_loader_ce, "dbSlaveNums", strlen("dbSlaveNums"), ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);

	//类常量
	zend_declare_class_constant_long(xmysql_loader_ce, "DB_TYPE_AUTO", strlen("DB_TYPE_AUTO"), XMYSQL_DB_TYPE_AUTO);
	zend_declare_class_constant_long(xmysql_loader_ce, "DB_TYPE_MASTER", strlen("DB_TYPE_MASTER"), XMYSQL_DB_TYPE_MASTER);
	zend_declare_class_constant_long(xmysql_loader_ce, "DB_TYPE_SLAVE", strlen("DB_TYPE_SLAVE"), XMYSQL_DB_TYPE_SLAVE);

    //方法
    return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(xmysql_loader) {
    //TODO 调整为请求生命周期
	php_printf("xmysql close all dbs\n");
	xmysql_close_all_db();
}

static zval *init_get_property_by_name(const char *field) {
	zval *configs = zend_read_static_property(xmysql_loader_ce,field, strlen(field), 1);
	if(!configs || Z_TYPE_P(configs) == IS_NULL) {
		zval emptyConfig;
		array_init(&emptyConfig);
		zend_update_static_property(xmysql_loader_ce, field, strlen(field), &emptyConfig);
		// zval_ptr_dtor(&emptyConfig); //添加数组成员，不需要减少引用计数
		configs = zend_read_static_property(xmysql_loader_ce, field, strlen(field), 1);
	}
	return configs;
} 

//读取xmysql_loader::$dbConfig, 自动初始化为空数组
zval *get_db_config() {
	return init_get_property_by_name("dbConfig");
}

zval *get_db_cache() {
	 return init_get_property_by_name("dbCache");
}

zend_ulong get_db_slave_num_from_cache(zend_string *dbName) {
	zval *slaveNumbers = init_get_property_by_name("dbSlaveNums");
	zval *number = zend_hash_str_find(Z_ARRVAL_P(slaveNumbers), ZSTR_VAL(dbName), ZSTR_LEN(dbName));
	if(!number) {
		return 0;
	}
	return Z_LVAL_P(number);
}

void set_db_slave_num_cache(zend_string *dbName, zend_long num) {
	zval *slaveNumbers = init_get_property_by_name("dbSlaveNums");
	add_assoc_long(slaveNumbers, ZSTR_VAL(dbName), num);
}

zval *xmysql_close_all_db() {
	zval *allCache = get_db_cache();

	zval *val;
	ulong idx;
	zend_string *key;
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(allCache), idx, key, val){
		zval *ival;
		ulong iidx;
		zend_string *ikey;
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(val), iidx, ikey, ival){
			air_call_object_method(ival, Z_OBJCE_P(ival), "close", NULL, 0, NULL);
		}ZEND_HASH_FOREACH_END();
		
	}ZEND_HASH_FOREACH_END();
}

zval *get_db_config_by_name(zend_string *dbName, zend_ulong  type) {
	//未缓存
	zval *dbConfig = zend_hash_str_find(Z_ARRVAL_P(get_db_config()), ZSTR_VAL(dbName), ZSTR_LEN(dbName));
	if(!dbConfig) {
		php_error(E_NOTICE, "xmysql dbNmae:%s not registered\n", dbName);
		return NULL;
	}

	zval *config = zend_hash_index_find(Z_ARRVAL_P(dbConfig), type);
	if(!config) {
		php_error(E_NOTICE, "xmysql dbName:%s not registered\n", dbName);
		return NULL;
	}

	if(type == DB_TYPE_SLAVE && Z_TYPE_P(config) == IS_ARRAY) {
		zend_long num = get_db_slave_num_from_cache(dbName);
		if(num <= 0) {
			return NULL;
		}
		zend_long idx = (num > 1) ? 0 : rand()%num;
		config = zend_hash_index_find(Z_ARRVAL_P(config), idx);
	}
	return config;
}

int xmysql_loader_get_db(zval **mysqli, zend_string *dbName, zend_ulong  type) {
	zval *dbCache = get_db_cache();
	zval *dbsCache = zend_hash_str_find(Z_ARRVAL_P(dbCache), ZSTR_VAL(dbName), ZSTR_LEN(dbName));
	 
    if(!dbsCache){
		zval emptyArray;
		array_init(&emptyArray);
		add_assoc_zval_ex(dbCache, ZSTR_VAL(dbName), ZSTR_LEN(dbName), &emptyArray);
		dbsCache = zend_hash_str_find(Z_ARRVAL_P(dbCache), ZSTR_VAL(dbName), ZSTR_LEN(dbName));
	}
	
	zend_ulong slaveNum = get_db_slave_num_from_cache(dbName);
	if(slaveNum == 0 && type == DB_TYPE_SLAVE) {
		type = DB_TYPE_MASTER; //没有注册从库时，访问从库，自动返回主库配置
	}

	zval *db = zend_hash_index_find(Z_ARRVAL_P(dbsCache), type);
	//已缓存
	if(db) {
		php_error(E_NOTICE, "read from cache db:%s type:%d.\n", ZSTR_VAL(dbName), type);
		*mysqli = db;
		return 1;
	}

	zval *dbConfig = get_db_config_by_name(dbName, type);
	if(!dbConfig) {
		php_error(E_NOTICE, "xmysql db %s-%d may not registered.\n", ZSTR_VAL(dbName), type);
		return  0;
	}

	
	zend_class_entry *mysqli_ce = (zend_class_entry *)zend_hash_str_find_ptr(EG(class_table), ZEND_STRL("mysqli"));
	if(mysqli_ce == NULL) {
		php_error(E_ERROR, "mysqli extestion may not installed.\n");
		return 0;
	}

	zval _mysqli;
	object_init_ex(&_mysqli, mysqli_ce);
//$db = new \mysqli($dbConfig['host'], $dbConfig['username'], $dbConfig['password'], $dbConfig['dbname'], $dbConfig['port']);
	zval params[5];
	zval *host = NULL, *user = NULL, *pass = NULL, * dbname = NULL, *port = NULL, nil;

	host = zend_hash_str_find(Z_ARRVAL_P(dbConfig), ZEND_STRL("host"));
	user = zend_hash_str_find(Z_ARRVAL_P(dbConfig), ZEND_STRL("username"));
	pass = zend_hash_str_find(Z_ARRVAL_P(dbConfig), ZEND_STRL("password"));
	dbname = zend_hash_str_find(Z_ARRVAL_P(dbConfig), ZEND_STRL("dbname"));
	
	port = zend_hash_str_find(Z_ARRVAL_P(dbConfig), ZEND_STRL("port"));

	params[0] = host ? *host : nil;
	params[1] = user ? *user : nil;
	params[2] = pass ? *pass : nil;
	params[3] = dbname ? *dbname : nil;
	params[4] = port ? *port : nil;
	
	air_call_object_method(&_mysqli, Z_OBJCE_P(&_mysqli), "__construct", NULL, 5, params);

	//TODO 错误码检查，代码有问题，暂不检查
	// zval *error_no = zend_read_property(mysqli_ce, EX(&_mysqli), ZEND_STRL("connect_errno"), 0, NULL);
	// if(Z_LVAL_P(error_no) != 0) {
	// 	php_printf("errno:%d\n", Z_LVAL_P(error_no));
	// 	// zval *error = zend_read_property(mysqli_ce, &_mysqli, ZEND_STRL("connect_error"), 0, NULL);
	// 	// php_error(E_ERROR, "[xmysql] mysql \"%s\" connected failed. errno:%d error:%s", ZSTR_VAL(dbName), Z_LVAL_P(error_no), Z_STRVAL_P(error));
	// 	 return 0;
	// }

	Z_TRY_ADDREF(_mysqli);
	add_index_zval(dbsCache,  type, &_mysqli);	
	*mysqli = &_mysqli;
	
	zval_ptr_dtor(&_mysqli);

	return  1;
}

//方法实现
PHP_METHOD(xmysql_loader, registerDb) {
	zend_string *key = NULL;
	zval *configVal = NULL;

	//判断
	if( zend_parse_parameters(ZEND_NUM_ARGS(), "Sa", &key, &configVal) == FAILURE ) {
		RETURN_FALSE;
	}
	zval *configs = get_db_config();

	add_assoc_zval_ex(configs, ZSTR_VAL(key), ZSTR_LEN(key), configVal);

	//设置slave hum
	zval *slaveConfig = zend_hash_index_find(Z_ARRVAL_P(configVal), DB_TYPE_SLAVE);
	zend_ulong slaveNum = 0;
	if(slaveConfig) {
		if(Z_TYPE_P(slaveConfig) == IS_ARRAY) {
			slaveNum = zend_array_count(Z_ARRVAL_P(slaveConfig));
		}
	}
	set_db_slave_num_cache(key, slaveNum);
	
	RETURN_TRUE;
}

PHP_METHOD(xmysql_loader, registerDbs) {
	zval *configVal = NULL;
	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &configVal) == FAILURE ) {
		RETURN_FALSE;
	}

	zval *configs = get_db_config();
	zend_hash_merge(Z_ARRVAL_P(configs), Z_ARRVAL_P(configVal), zval_add_ref, 1);

	//设置slave numbers
	zend_ulong    hashIndex;
	zval *hashData;
	zend_string *hashKey;
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARR_P(configVal), hashIndex, hashKey, hashData) {
		zval *slaveConfig = zend_hash_index_find(Z_ARRVAL_P(hashData), DB_TYPE_SLAVE);
		zend_ulong slaveNum = 0;
		if(slaveConfig) {
			if(Z_TYPE_P(slaveConfig) == IS_ARRAY) {
				slaveNum = zend_array_count(Z_ARRVAL_P(slaveConfig));
			}
		}
		php_printf("hash idx:%s num:%d\n", ZSTR_VAL(hashKey), slaveNum);
		set_db_slave_num_cache(hashKey, slaveNum);
	} ZEND_HASH_FOREACH_END();

	RETURN_TRUE;
}

PHP_METHOD(xmysql_loader, getDbConfig) {
	zend_string *key = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|S", &key) == FAILURE){
		RETURN_NULL();
	}

	zval *configs = zend_read_static_property(xmysql_loader_ce, ZEND_STRL("dbConfig"), 0);
	if(Z_TYPE_P(configs) == IS_NULL) {
		RETURN_NULL();
	}

	zval *config = zend_read_static_property(xmysql_loader_ce, ZEND_STRL("dbConfig"), 0);
	if(!key || ZSTR_LEN(key) == 0) {
		RETURN_ZVAL(config, 1, 0);
	}
	zval *val = zend_hash_str_find(Z_ARRVAL_P(configs), ZSTR_VAL(key), ZSTR_LEN(key));
	RETURN_ZVAL(val, 1, 0);
}


PHP_METHOD(xmysql_loader, getDb) {
	zend_string *dbName = NULL;
	zend_long type;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Sl", &dbName, &type) == FAILURE) {
		RETURN_NULL();
	}
	zval *db;

	if(!xmysql_loader_get_db(&db, dbName, type)) {
		RETURN_NULL();
	} 

	RETURN_ZVAL(db, 1, 0);
}


PHP_METHOD(xmysql_loader, test) {
	php_printf("this is test\n");
	RETURN_TRUE;
}