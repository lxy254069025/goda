<?php
namespace controllers;
use models;
use Goda\Application;

class Index extends \Goda\Controller {

    public function Index() {
        
        $this->render("index/index",['h'=>'Hello World!']);
    }

    public function Aaa() {
        echo "Hello Any";
        var_dump($this);
    }

    public function notFound() {

    }
}