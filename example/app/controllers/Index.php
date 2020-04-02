<?php
namespace controllers;
use models;
use Goda\Application;

class Index extends \Goda\Controller {

    public function Index() {
        $this->render("index/index",['hi'=>'Hello World!']);
    }

    public function Aaa() {
        $this->redirect("/");
    }

    public function notFound() {
        echo "NOT FOUND";
    }
}