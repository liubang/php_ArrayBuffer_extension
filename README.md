#  A simple ArrayBuffer implementation for php.


## Demo

```php
<?php

$buffer = new linger\ArrayBuffer(256);
var_dump($buffer);

$int32 = new linger\ArrayBufferView\Int32Array($buffer);
var_dump($int32);
$uint8 = new linger\ArrayBufferView\UInt8Array($buffer);
var_dump($uint8);

for ($i = 0; $i < 255; $i++) {
    $uint8[$i] = $i;

}

for ($i = 0; $i < 255; $i++) {
    echo $int32[$i], "\n";

}

```
