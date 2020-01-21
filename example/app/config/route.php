<?php
return [
    'POST' => [
        '/' => 'Index@Index'
    ],
    'GET' => [
        '/' => 'Index@Index',
        '/name' => 'User@Index',
        '/:a/c' => 'Index@aaa' 
    ],
    'ANY' => [
        '/' => '',
        '/:nae' => 'Index@Aaa',
    ],

    'ERROR' => [
        'notfound' => 'Index@notFound'
    ]
];