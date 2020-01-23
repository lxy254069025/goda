<?php
namespace controllers;
use models;
use Goda\Application;

class Index extends \Goda\Controller {

    public function Index() {
        $this->renderText("HHHHH");
        $this->renderJson([1=>2]);
        $this->render("index/index",['h'=>'Hello World!']);
    }

    public function Aaa() {
        echo "Hello Any";
        $this->redirect("/");
    }

    public function notFound() {

    }
}