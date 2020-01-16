<?php
return [
    'POST' => [
        '/' => 'Index@Index'
    ],
    'GET' => [
        '/' => 'Index@Index',
        '/user' => 'User@Index'
    ],
    'ANY' => [
        '/' => '',
        '/{name}-a' => 'Index@Aaa',
    ]
];