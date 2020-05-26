#### 快速开始demo
拷贝example至你的web目录下。运行 ``` php -S localhost:8848 -t web/ ```

#### 目录布局

```
├── app
│   ├── config   //配置文件目录
│   │   ├── config.php
│   │   └── route.php   //路由
│   ├── controllers   控制器目录
│   │   ├── Index.php
│   │   └── user.php
│   ├── models     //model目录
│   │   └── test.php
│   └── views   //视图目录
│       └── index
│           └── index.php
└── web
    └── index.php  //程序入口
```


### 文档根目录
您应该设置DocumentRoot为application/web，因此用户只能访问公用文件夹

#### index.php
index.php在公共目录中是应用程序的唯一入口
```
<?php
$appPath = __DIR__ . "/../app";

$app = new Goda\Application($appPath); //加载配置文件和路由 

$app->bootstrap();

$app->run();  //运行 
```

### 路由规则
#### nginx
```
server {
  listen ****;
  server_name  domain.com;
  root   document_root;
  index  index.php index.html index.htm;
 
  if (!-e $request_filename) {
    try_files $uri/ /index.php?$query_string;
  }
}
```