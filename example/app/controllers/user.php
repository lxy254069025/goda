<?php
namespace controllers;
use \Goda\Controller;

class User extends Controller {
    use \Goda\Response;
    
    public function Index() {
        $this->renderJson([
            "info" => "",
            "msg" => ""
        ]);

        
    }
}