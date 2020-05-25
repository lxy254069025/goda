<?php
namespace controllers;
use \Goda\Controller;

class User extends Controller {
    
    public function Index() {
        echo $this->renderJson([
            "info" => "",
            "msg" => ""
        ]);
        
    }
}