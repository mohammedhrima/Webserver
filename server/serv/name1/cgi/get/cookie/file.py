#!/usr/bin/env python3
import cgi
import http.cookies
import os

# Parse incoming cookies
cookies = http.cookies.SimpleCookie(os.environ.get('HTTP_COOKIE', ''))

# Print the Content-Type header
print("Content-Type: text/html")
print()

# Print HTML content
print("<html>")
print("<head>")
print("<title>Cookie Test</title>")
print("</head>")
print("<body>")

# Print greeting message
print("<h1>Hello from Python</h1>")
print("<p>Here are the cookies sent with the request:</p>")
print("<ul>")

# Print the names and values of all available cookies
for cookie in cookies.values():
    name = cookie.key
    value = cookie.value
    print(f"<li>{name}: {value}</li>")

print("</ul>")
print("</body>")
print("</html>")
