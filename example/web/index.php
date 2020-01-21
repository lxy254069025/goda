<?php
$appPath = __DIR__ . "/../app";

$app = new Goda\Application($appPath); //加载配置文件和路由 

// $app->swoole();
$app->bootstrap();
var_dump(Goda\Application::app());
$app->run();  //运行 


