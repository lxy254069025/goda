--TEST--
Check if goda is loaded
--SKIPIF--
<?php
if (!extension_loaded('goda')) {
	echo 'skip';
}
?>
--FILE--
<?php
echo 'The extension "goda" is available';
?>
--EXPECT--
The extension "goda" is available
