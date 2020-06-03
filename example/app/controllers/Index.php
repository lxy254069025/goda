<?php
namespace controllers;
use Goda\Application;

class Index extends \Goda\Controller {

    public function Index() {
        Application::app()->setObject("this",$this);

        var_dump(Application::app()->getObject("this"));
        echo $this->render("index/index",['hi'=>'Hello World!']);
    }

    public function Aaa() {
        $this->redirect("/");
    }

    public function notFound() {
        echo "NOT FOUND";
    }
}