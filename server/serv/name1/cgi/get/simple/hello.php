<?php
$msg = "<html>\n\t<body>\n\t\t<h1>Hello from PHP CGI</h1>\n\t</body>\n</html>";

header("HTTP/1.0 200 OK");
header("Content-Type: text/html");
header("Content-Length: " . strlen($msg));

echo $msg;
?>
