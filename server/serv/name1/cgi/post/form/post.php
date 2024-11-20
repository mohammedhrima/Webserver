<?php

header('Content-Type: text/html');

$msg = "<html><body>";
$msg .= "<h1>PHP response</h1>";
if (isset($_POST['name']) && isset($_POST['age'])) {
    header("HTTP/1.1 200 OK");
    $name = htmlspecialchars($_POST['name']);
    $age = htmlspecialchars($_POST['age']);
    $msg .= "<p>User has name: " . $name . "</p>";
    $msg .= "<p>And age: " . $age . "</p>";
} else {
    header("HTTP/1.1 400 NOK");
    $msg .= "<h2>Name and age are required!</h2>";
}

$msg .= "</body></html>";

header("Content-Length: " . strlen($msg));

echo $msg;

?>
