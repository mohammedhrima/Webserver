#!/bin/bash

# Set CGI environment variables
export REQUEST_METHOD="GET"
export SCRIPT_FILENAME=$(pwd)/hello.php
export SCRIPT_NAME=/hello.php
export QUERY_STRING=""
export CONTENT_TYPE="text/html"
export CONTENT_LENGTH=0
export GATEWAY_INTERFACE="CGI/1.1"
export SERVER_PROTOCOL="HTTP/1.1"
export SERVER_SOFTWARE="my-cgi-script/1.0"
export REMOTE_ADDR="127.0.0.1"
export SERVER_NAME="localhost"
export SERVER_PORT=80
export REDIRECT_STATUS=CGI

# Run the php-cgi binary
php-cgi -f hello.php
