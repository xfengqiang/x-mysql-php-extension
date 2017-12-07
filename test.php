<?php
error_reporting(E_ALL);

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

function testCond($dbs) {
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

    $db = xmysql_loader::getDb('mall', xmysql_loader::DB_TYPE_SLAVE);
    echo "sql in Cond:".xmysql_cond::inCond([1,'a','#'], $db)."\n";
    echo "sql equal Cond:".xmysql_cond::equalCond(["id"=>'abc', "name"=>"fank"],NULL)."\n";
    echo "sql equal Cond:".xmysql_cond::equalCond(["id"=>1, "name"=>"fank ' /*"], $db)."\n";
     echo "sql select: ".xmysql_cond::table("user")->select("*")->andc("id", 1)->andc("name", "fank")->order('id')->order('name', 'desc')->order(['create_time'=>'ASC'])->limit('1', 2)->sql();
    //$cond = xmysql_cond::table("user")->select("*")->select('*')->select('*')->select('*');
     echo "sql insert: ".xmysql_cond::table("user")->insert(['name'=>'fank'])->sql()."\n";
     echo "sql update: ".xmysql_cond::table("user")->update(['name'=>'fank'])->andc("id", 1)->sql()."\n";
     echo "sql del: ".xmysql_cond::table("user")->del("user")->andc('name','fankxu-rollback-tx')->orc('name','fankxu-commit-tx')->sql()."\n"; 
}

function printRet($msg, $data) {
    echo "$msg ".json_encode($data)."\n";
}

function xmysql_callback(\xmysql_db $mysql, \mysqli $db, $sql){
    echo "Global callback [SQL] {$sql} TimeUsed:".$mysql->lastQueryTime()."ms errno:".$mysql->lastErrorCode()." errmsg:".$mysql->lastErrorMsg()."\n";
}

class A {
    public function callback(\xmysql_db $mysql, \mysqli $db, $sql)
    {
        echo "Class callback [SQL] {$sql} TimeUsed:".$mysql->lastQueryTime()."ms errno:".$mysql->lastErrorCode()." errmsg:".$mysql->lastErrorMsg()."\n";
    }
}
function testDb($dbs){
    xmysql_db::setGlobalCallBack('xmysql_callback');
    xmysql_db::enableGlobalProfile(true);
    $a = new A();
    // xmysql_db::setGlobalCallBack(array($a, 1));
   $db = new xmysql_db("mall");  
//    $db->callback('xmysql_callback');
    // xmysql::setGlobalCallBack(function (xmysql $mysql, mysqli $db, $sql) {
    //     echo "[SQL] {$sql} TimeUsed:".$mysql->lastQueryTime()."ms errno:".$mysql->lastErrorCode()." errmsg:".$mysql->lastErrorMsg()."\n";
    // });

    //test raw
    $ret = $db->query("SELECT * FROM user");
    printRet("query", $ret);

    $ret = $db->queryRow('SELECT * FROM user WHERE id=1');
    printRet("query row", $ret);
  
    $ret = $db->select('user')->limit(1)->limit(1, 10)->query();
    printRet("page cond", $ret);

    //and condition
    $ret = $db->select('user', 'name')->andc('id', 1)->queryRow();
    printRet("test cond", $ret);
    //equal condition
    $ret = $db->select("user")->equal(['id'=>1])->queryRow();
    printRet("test equal", $ret);

    //in condition
    $ret = $db->select("user")->in('id', [1])->queryRow();
    printRet("test in", $ret);
   
    //insert
    $ret = $db->insert("user", ['name'=>'test-'.time(), 'ctime'=>time()])->query();
    echo "last insert id ".$db->lastInsertId()."\n";
    printRet("test insert ", $ret);

    //update
    $ret = $db->update("user", ['ctime'=>time()])->andc('id', 1)->query();
    printRet("test update", "ret: ".$ret." sql:".$db->lastSql());
    //error
    $ret = $db->insert("user", ['id'=>1])->query();
    printRet("test error", $ret);
    printRet("test error info", "errNo:".$db->lastErrorCode()." errMsg:".$db->lastErrorMsg());

    //count
    $ret = $db->select("user", "count(*) as cnt")->queryRow();
    printRet("test count", $ret);

    //delete
    $ret = $db->del("user")->andc('name','fankxu-rollback-tx')->orc('name','fankxu-commit-tx')->query();
    printRet("test del", $ret);

    $ret = $db->startTx();
    echo "start tx ret:".$ret."\n";
    $db->insert("user", ['name'=>"fankxu-rollback-tx"])->query();
    $ret = $db->rollbackTx();
    printRet("rollback ret", $ret);

    $db->startTx();
    $db->insert("user", ['name'=>"fankxu-commit-tx"])->query();
    $ret = $db->commitTx();
    printRet("commit ret", $ret);
}

xmysql_loader::registerDb('mall', $dbs['mall']);
//  xmysql_loader::getDb("mall", 1);
// $db2 = xmysql_loader::getDb("mall", 2);
xmysql_loader::registerDb('mall', $dbs['mall']);
xmysql_loader::registerDbs($dbs);
xmysql_loader::registerDbs($dbs);
// $ret = $db1->query("SELECT * from user");
// var_dump($db1);
// getDb($dbs);
// testCond($dbs);
testDb($dbs);

xmysql_loader::closeAll();
