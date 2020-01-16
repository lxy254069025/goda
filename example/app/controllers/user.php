<?php
namespace controllers;
use \Goda\Controller;

class User extends Controller {
    use \Goda\Response;
    
    public function Index() {
        var_dump($this);
        $this->renderJson();
        echo "Hello, Current controller is User, action is Index";

        
    }
}