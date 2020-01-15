<?php
namespace controllers;

class Index extends \Goda\Controller {
    use \Goda\Response;

    public function Index() {
        $this->redirect("/user");
    }

    public function aaa() {
        $this->render("index/index",[1=>4, "abc"=> 9009, 'h'=>'Hello World!']);
    }
}