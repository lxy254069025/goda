<?php
return [
    'POST' => [
        '/' => 'Index@Index'
    ],
    'GET' => [
        '/' => 'Index@Index',
        // "/a" => 'Index@aaa',
        '/user' => 'User@Index'
    ],
    'ANY' => [
        '/' => '',
        '/{name}-a' => 'Index@Aaa',
    ]
];