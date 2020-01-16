--TEST--
goda_test1() Basic test
--SKIPIF--
<?php
if (!extension_loaded('goda')) {
	echo 'skip';
}
?>
--FILE--
<?php
$ret = goda_test1();

var_dump($ret);
?>
--EXPECT--
The extension goda is loaded and working!
NULL
