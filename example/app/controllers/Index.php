<?php
namespace controllers;

class Index extends \Goda\Controller {
    use \Goda\Response;

    public function Index() {
        $this->render("index/index",['h'=>'Hello World!']);
    }

    public function aaa() {
        echo "Hello Any";
        var_dump($this);
    }
}