<?php
error_reporting(E_ALL);

//function classTest() {
//$db = new xmysql();
//var_dump($db);
//
//$cond = new xmysql_cond();
//var_dump($cond);
//
//$loader = new xmysql_loader();
//var_dump($loader);
//xmysql_loader::$dbCache = array('a');
//var_dump(xmysql_loader::$dbCache);
//echo "master :".xmysql_loader::DB_TYPE_MASTER."\n";
//}

//$ret = xmysql_loader::registerDb("mall", ["a"=>"b"]);
//$mallConfig = xmysql_loader::getDbConfig("mall");
//var_dump($mallConfig);
function testRawMysql(){
    $db = new mysqli('127.0.0.1', 'root','z', 'mall', '3307');
    var_dump($db);
}
//testRawMysql();
//return ;
$dbs = ['mall'=> [
    xmysql_loader::DB_TYPE_MASTER=>[
        'host'=>'127.0.0.1',
        'port'=>'3306',
        'username'=>'root',
        'password'=>'z',
        'dbname'=>'mall',
        'charset'=>'utf8',
    ],
    xmysql_loader::DB_TYPE_SLAVE=>[
        [
            'host'=>'127.0.0.1',
            'port'=>3306,
            'username'=>'root',
            'password'=>'z',
            'dbname'=>'mall',
            'charset'=>'utf8',
        ],
        [
            'host'=>'127.0.0.1',
            'port'=>3306,
            'username'=>'root',
            'password'=>'z',
            'dbname'=>'mall',
            'charset'=>'utf8',
        ]
    ]
    ],
];

function getDb($dbs){
//xmysql_loader::registerDb('mall', $dbs['mall']);
xmysql_loader::registerDbs($dbs);
$db = xmysql_loader::getDb("mall", xmysql_loader::DB_TYPE_SLAVE);
$ret = $db->query("SELECT * FROM user");
$data  = $ret->fetch_all();
var_dump($data);
$db->close();
}

function testError($dbs){
$dbs['mall'][xmysql_loader::DB_TYPE_SLAVE][0]['port']='3307';
$dbs['mall'][xmysql_loader::DB_TYPE_SLAVE][0]['host']='';

xmysql_loader::registerDbs($dbs);
$dbConfig = xmysql_loader::getDbConfig();
var_dump($dbConfig);
$db = xmysql_loader::getDb("mall", xmysql_loader::DB_TYPE_SLAVE);
var_dump($db->connect_errno);
}
getDb($dbs);
//testError($dbs);
