<?php

/**
 * Created by PhpStorm.
 * User: xufengqiang
 * Date: 2017/8/18
 * Time: 12:46
 */

class xmysql_loader
{
    const DB_TYPE_AUTO = 0;
    const DB_TYPE_MASTER = 1;
    const DB_TYPE_SLAVE = 2;

    /**
     * 注册多个数据库实例
     * @param $config $config = ['maill'=>[
                xmysql_loader::DB_TYPE_MASTER=>[
                    'host'=>'127.0.0.1',
                    'port'=>3306,
                    'username'=>'root',
                    'password'=>'root',
                    'dbname'=>'mall',
                    'charset'=>'utf8',
                ],
                xmysql_loader::DB_TYPE_SLAVE=>[
                [
                    'host'=>'127.0.0.1',
                    'port'=>3306,
                    'username'=>'root',
                    'password'=>'root',
                    'dbname'=>'mall',
                    'charset'=>'utf8',
                ]
            ]
         ]
     * ];
     */
    public static function registerDbs($config){
        foreach ($config as $name => $cfg) {
            self::$dbConfig[$name] = $cfg;
        }
    }

    /**
     * 注册一个数据库实例
     * @param $config $config = [
            xmysql_loader::DB_TYPE_MASTER=>[
                'host'=>'127.0.0.1',
                'port'=>3306,
                'username'=>'root',
                'password'=>'root',
                'dbname'=>'mall',
                'charset'=>'utf8',
            ],
            xmysql_loader::DB_TYPE_SLAVE=>[
                [
                'host'=>'127.0.0.1',
                'port'=>3306,
                'username'=>'root',
                'password'=>'root',
                'dbname'=>'mall',
                'charset'=>'utf8',
                ]
        ]
    ];
     */
    public static function registerDb($name, $config){
        self::$dbConfig[$name] = $config;
    }

    /**
     * 获取数据库配置信息
     * @param $name
     * @return null|array
     */
    public static function getDbConfig($name) {
    }

    /**
     * 获取一个数据库连接
     * @param $name
     * @param $type  xmysql::QUERY_MASTER|xmysql::QUERY_SLAVE
     * @return bool|\mysqli
     * @throws Exception
     */
    public static function getDb($name, $type) {

    }

    /**
     * 关闭一个数据库连接
     * @param $name
     */
    public static function closeDb($name){
    }

    /**
     * 关闭所有已打打开的数据库连接
     */
    public static function closeAll(){
    }
}

class xmysql_cond{
    const QUERY_TYPE_READ = 1;
    const QUERY_TYPE_WRITE = 2;

    /**
     * @param $table
     * @return xmysql_cond
     */
    public static function table($table) {
    }

    /**
     * @param $params
     * @return string
     */
    public static function equalCond($params, \mysqli $escapeDb=null) {
    }

    /**
     * @param $vals
     * @return string
     */
    public static function inCond($vals, \mysqli $escapeDb=null) {
    }

    /**
     * xmysql_cond constructor.
     * @param $table
     */
    public function __construct($table){
    }

    /**
     * @param string $fields
     * @return $this
     */
    public function select($fields='*') {
    }

    /**
     * @param bool $ignoreInsert
     * @return $this
     */
    public function insert($data, $ignoreInsert=false){
    }

    /**
     * @param $fields
     * @return $this
     */
    public function update($fields) {
    }

    /**
     * @return $this
     */
    public function del(){
    }

    /**
     * @param $k
     * @param $v
     * @param string $op
     * @return $this
     */
    public function andc($k, $v, $op='='){
    }

    /**
     * @param $k
     * @param $v
     * @param string $op
     * @return $this
     */
    public function orc($k, $v, $op='='){
    }

    /**
     * @param $k
     * @param $v
     * @param string $op
     * @return $this
     */
    public function equal($params){
    }

    /**
     * @param $k
     * @param $vals
     * @param string $cond
     * @return $this
     */
    public function in($k, $vals, $cond='AND') {
    }

    /**
     * @param $k
     * @param $vals
     * @param string $cond
     * @return $this
     */
    public function notIn($k, $vals, $cond='AND') {
    }

    /**
     * @param $k
     * @param string $type
     * @return $this
     */
    public function order() {
    }

    /**
     * @param $_
     * @return $this
     */
    public function limit($_) {
    }


    /**
     * @return string
     */
    public function sql(\mysqli $db=null) {
    }

    /**
     * @return int
     */
    public function getQueryType() {
    }
}


class xmysql_db
{
    /**
     * @param callable function(\xmysql_db $db, \mysqli $mysqli, $sql)
     */
    public static function setGlobalCallBack($callback) {
    }

    public static function enableGlobalProfile($enable) {
    }

    public function __construct($dbName){
    }

    /**
     * @param $func
     * @return $this
     */
    public function callback($func) {
    }

    /**
     * @param string $fields
     * @return $this
     */
    public function select($table, $fields='*') {
    }

    /**
     * @param bool $ignoreInsert
     * @return $this
     */
    public function insert($table,  $data, $ignoreInsert=false){
    }

    /**
     * @param $fields
     * @return $this
     */
    public function update($table, $data) {
    }


    /**
     * @param $fields
     * @return $this
     */
    public function del($table) {
    }


    /**
     * @return $this
     */
    public function where() {
    }

    /**
     * @param $k
     * @param $v
     * @param string $op
     * @return $this
     */
    public function andc($k, $v, $op='='){
    }

    /**
     * @param $k
     * @param $v
     * @param string $op
     * @return $this
     */
    public function orc($k, $v, $op='='){
    }

    /**
     * @param $k
     * @param $v
     * @param string $op
     * @return $this
     */
    public function equal($params){
    }

    /**
     * @param $k
     * @param $vals
     * @param string $cond
     * @return $this
     */
    public function in($k, $vals, $cond='AND') {
    }

    /**
     * @param $k
     * @param $vals
     * @param string $cond
     * @return $this
     */
    public function notIn($k, $vals, $cond='AND') {
    }

    /**
     * @param $order ['id'=>'desc']
     * @return $this
     */
    public function order($order) {
    }

    /**
     * @param $offset
     * @param $limit
     * @return $this
     */
    public function limit($offset, $limit) {
    }

    /**
     * @param $type
     * @return bool|\mysqli
     */
    public function db($type){
    }

    /**
     * 写操作
     * @param $sql
     * @return bool|mixed|\mysqli_result
     */
    public function exec($sql) {
    }

    /**
     * @param xmysql_cond $cond
     * @param $type
     * @return bool|mixed|\mysqli_result
     */
    public function queryByCond(xmysql_cond $cond, $type=xmysql_loader::DB_TYPE_AUTO) {
    }

    /**
     * @param xmysql_cond $cond
     * @param $type
     * @return array|bool|mixed|mysqli_result
     */
    public function queryRowByCond(xmysql_cond $cond, $type=xmysql_loader::DB_TYPE_AUTO) {
    }

    /**
     * @param string $sql
     * @param $type
     * @param bool $isRow
     * @return array|bool|mixed|mysqli_result
     */
    public function query($sql='', $type=xmysql_loader::DB_TYPE_AUTO, $isRow=false) {
    }

    /**
     * @param string $sql
     * @param $type
     * @return array|bool|mixed|mysqli_result
     */
    public function queryRow($sql='', $type=xmysql_loader::DB_TYPE_AUTO){
        return $this->query($sql, $type, true);
    }

    /**
     * @return int|bool
     */
    public function lastErrorCode(){
    }

    /**
     * @return string
     */
    public function lastErrorMsg(){
    }

    /**
     * @return string
     */
    public function lastSql() {
    }

    /**
     * @return int
     */
    public function rowsAffected(){
    }

    /**
     * @return bool
     */
    public function lastInsertId(){
    }

    /**
     * 上次查询的耗时，需要启用enableProfile, 单位毫秒
     * @return double
     */
    public function lastQueryTime() {
    }


    /**
     * @return string
     */
    public function getDbName() {
    }

    /**
     * @return bool
     */
    public function isInTx() {
    }

    /**
     * @return bool
     */
    public function startTx(){
    }

    /**
     * @return bool
     */
    public function commitTx(){
    }

    /**
     * @return bool
     */
    public function rollbackTx(){
    }
}