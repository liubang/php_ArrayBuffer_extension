<?php

$buffer = new linger\ArrayBuffer(1024);
var_dump($buffer);

$v = new linger\ArrayBufferView\Int32Array($buffer);
var_dump($v);

$v[1] = 12;
var_dump($v[1]);

for ($i = 0; $i < 10; $i++) {
    $v[$i] = $i;
}

for ($i = 0; $i < 10; $i++) {
    var_dump($v[$i]);
}




