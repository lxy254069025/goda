<?php
namespace controllers;
use models;

class Index extends \Goda\Controller {
    use \Goda\Response;

    public function Index() {
        $test = new models\Test();
        $this->render("index/index",['h'=>'Hello World!']);
    }

    public function aaa() {
        echo "Hello Any";
        var_dump($this);
    }
}