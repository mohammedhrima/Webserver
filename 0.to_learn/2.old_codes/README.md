+ check RFC 2616

+ request: 
    + run one of the following commands:
        + nc example.com 80 < request.txt
        + socat tcp:example.com:80 - < request.txt
        + telnet example.com 80
        + curl -vvv http://example.com/

    + request.txt:
        GET / HTTP/1.0
        Host: example.com
        (new line)

+ response:
    HTTP/1.0 200 OK
    Age: 525410
    Cache-Control: max-age=604800
    Content-Type: text/html; charset=UTF-8
    Date: Thu, 20 Oct 2020 11:11:11 GMT
    Etag: "1234567890+gzip+ident"
    Last-Modified: Thu, 20 Oct 2019 11:11:11 GMT
    Vary: Accept-Encoding
    Content-Length: 1256
    Connection: close

+ let's break it down:
    + request:
        + GET / HTTP/1.1:
            + method:       GET
            + uri:          /
            + HTTP version: 1.1
    + response:
        + HTTP/1.1 200 OK:
            + HTTP version:   1.1
            + status code:    200
            + status message: OK
        + the following is put in this order key : value
        + request has:
            + Age: 525410
            + ...

SOCK_TREAM: TCP
SOCK_DGRAM: UDP
AI_PASSIVE: listen on any available network interface

+ kill processes that uses specific port:
    + kill -9 $(lsof -ti :17000)
    + netstat -tulpn (also usefull)

+ HTTP request:
    + TODO: handle keep alive: ??

+ URL: http://www.example.com:80/res/page1.php?user=bob#account
    + Protocol: http
        + http, ftp, https
    + Hostname: www.example.com
        + could be also an IP address
    + Port: 80
    + Path: res/page1.php?user=bob
    + Hash: #account
        + jump to #account location inside html page


+ check: 
    + https://unscriptedcoding.medium.com/multithreaded-server-in-c-using-epoll-baadad32224c
    + https://copyconstruct.medium.com/the-method-to-epolls-madness-d9d2d6378642
    + https://idndx.com/the-implementation-of-epoll-3/


+ webserve partition:
    + Try using EPOLE instead of select
    + Update timing for client and drop it (men ghire syal epole)
    + Requests:
        + GET:
        + DELETE:
            + recursive folders
            + permission !!
        + POST:
            + binary: (Content-Length)
                + one body
                POST / HTTP/1.1
                Host: localhost:17000
                Content-Type: text/plain
                Content-Length: 22

            + chuncked:
                + size of checked to the next check
                + file ends with 0
                HTTP/1.1 200 OK
                Content-Type: text/plain
                Transfer-Encoding: chunked

                25
                This is the first chunk
                1A
                and this is the second one
                3
                con
                8
                sequence
                0

            + boundary:
                + multiple file in one body 
                + sperated by specific seperator
                HTTP/1.1 200 OK
                Content-Type: multipart/form-data; boundary=boundary123

                --boundary123
                Content-Disposition: form-data; name="field1"

                value1
                --boundary123
                Content-Disposition: form-data; name="field2"

                value2
                --boundary123--

            + chuncked-boundry:
                HTTP/1.1 200 OK
                Content-Type: multipart/form-data; boundary=boundary123
                Transfer-Encoding: chunked

                11
                --boundary123
                Content-Disposition: form-data; name="field1"

                value1
                11
                --boundary123
                Content-Disposition: form-data; name="field2"

                value2
                11
                --boundary123--
                0
    + Response:
        + send a large file
    + CGI:
        + POST:
            + pass body to CGI script
            + handle Cookies ??
        + GET:
            + query string
            + handle Cookies ??
        + use PATH_INFO (in the subject)
        + handle chunked for CGI (subject)
        + if no content_length, keep reading till you reach the end (subject)








