<?php
// Parse incoming cookies
$cookies = isset($_SERVER['HTTP_COOKIE']) ? $_SERVER['HTTP_COOKIE'] : '';
parse_str($cookies, $parsed_cookies);

// Print the Content-Type header
header('Content-Type: text/html');
echo "\n";

// Print HTML content
echo "<html>\n";
echo "<head>\n";
echo "<title>Cookie Test</title>\n";
echo "</head>\n";
echo "<body>\n";

// Print greeting message
echo "<h1>Hello from PHP</h1>\n";
echo "<p>Here are the cookies sent with the request:</p>\n";
echo "<ul>\n";

// Print the names and values of all available cookies
foreach ($parsed_cookies as $name => $value) {
    echo "<li>$name: $value</li><br>";
}

echo "</ul>\n";
echo "</body>\n";
echo "</html>\n";
?>
