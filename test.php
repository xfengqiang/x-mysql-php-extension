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
//getDb($dbs);
//testError($dbs);

function testCond($dbs) {
   xmysql_loader::registerDb('mall', $dbs['mall']);
   $db = xmysql_loader::getDb('mall', xmysql_loader::DB_TYPE_SLAVE);
    //select
    $sql = xmysql_cond::table('gjj_invite_activity_user')
        ->select('id,team_id')
        ->equal(['user_id'=>1])
        ->andc('query_status', 0, '>')
        ->limit(10, 10)
        ->order(['create_time'=>'DESC'])
        ->sql();

    echo "[SELECT] {$sql}\n";

    //insert
    $sql = xmysql_cond::table('gjj_invite_activity_user')
        ->insert(['user_id'=>1,'user_name'=>'fankxu'])
        ->sql();

    echo "[INSERT] {$sql}\n";

    //update
    $sql = xmysql_cond::table('gjj_invite_activity_user')
        ->update(['user_id'=>1,'user_name'=>'fankxu'])
        ->andc('id', 1)
        ->sql();
    echo "[UPDATE] {$sql}\n";
}


function testCond1($dbs) {
   xmysql_cond::inCond([]);
return ;
   xmysql_loader::registerDb('mall', $dbs['mall']);
   $db = xmysql_loader::getDb('mall', xmysql_loader::DB_TYPE_SLAVE);
   echo "sql in Cond:".xmysql_cond::inCond([1,'a','#'], $db)."\n";
   echo "sql equal Cond:".xmysql_cond::equalCond(["id"=>'abc', "name"=>"fank"],NULL)."\n";
   echo "sql equal Cond:".xmysql_cond::equalCond(["id"=>1, "name"=>"fank ' /*"], $db)."\n";
   $cond = xmysql_cond::table("user")->select("*")->andc("id", 1)->andc("name", "fank")->order('id')->order('name', 'desc')->order(['create_time'=>'ASC'])->limit('1', 2);
   //$cond = xmysql_cond::table("user")->select("*")->select('*')->select('*')->select('*');
   //$cond = xmysql_cond::table("user")->insert(['name'=>'fank'], 1);
   //$cond = xmysql_cond::table("user")->update(['name'=>'fank']);
debug_zval_dump($cond);
   var_dump($cond->page);
echo  $cond->sql()."\n";
return ;
  
}

function testDb($dbs){
   xmysql_loader::registerDb('mall', $dbs['mall']);
   $db = new xmysql_db("mall");
   $db->select('user', '*')->andc("id", 1);
   $mysqli = $db->db(1);
    var_dump($mysqli);
   //var_dump($db);
}

testDb($dbs);
//getDb($dbs);
//return ;
//testCond1($dbs);
