<?php
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

xmysql_loader::registerDbs($dbs);
while(true){
$db = new xmysql_db("mall");
   $ret = $db->queryRow("select * from user limit 1");
    echo json_encode($ret)."\n";
break;
}
