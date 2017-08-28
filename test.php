<?php
error_reporting(E_ALL);
try {
	$buffer = new linger\ArrayBuffer(256);
	var_dump($buffer);
	var_dump($buffer->length());

	$int32 = new linger\ArrayBufferView\Int32Array($buffer);
	var_dump($int32);
	$uint8 = new linger\ArrayBufferView\UInt8Array($buffer);
	var_dump($uint8);

	for ($i = 0; $i < 256; $i++) {
    		$uint8[$i] = $i;
	}
	var_dump($uint8);

    foreach ($int32 as $k => $v) {
        echo $k, "==>", $v, "\n";
    }

} catch (Exception $e) {
	echo $e->getMessage(), "\n";
	echo $e->getTraceAsString(), "\n";
}





