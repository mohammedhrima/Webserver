<?php
ini_set('display_errors', 1);
error_reporting(E_ALL);

$name = isset($_GET['name']) ? $_GET['name'] : null;

$msg = "";

if ($name === null) {
    header("HTTP/1.1 400 NOK");
    $msg = "<h1>name query is required</h1>";
} else {
    header("HTTP/1.1 200 OK");
    $name = htmlspecialchars($name, ENT_QUOTES, 'UTF-8');
    $msg = "<h1>hello {$name}</h1>";
}

header("Content-Type: text/html");
header("Content-Length: " . strlen($msg));

echo $msg;
?>
