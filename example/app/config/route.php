<?php
return [
    'GET' => [
        '/' => 'Index@Index',
        '/name' => 'User@Index',
        '/:a/c' => 'Index@aaa' 
    ],
    'ANY' => [
        '/abc/:name' => 'Index@Aaa',
    ],

    'ERROR' => [
        'notfound' => 'Index@notFound'
    ]
];