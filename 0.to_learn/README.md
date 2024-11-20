+ socket: 
    + low level endpoint
    + way to connect 2 computers
    + networking protocols build on the top of socket HTTP, FTP

+ socket syscall:
    + args:
        + domain
        + connection socket / tcp or udp protocol
        + 0 stand for default protocol (TCP)

+ server socker workflow:
    + create socket
    + bind the IP and the port ??
        + bind ??
        + from where to listen
    + listen ??
    + to get another socket ??

+ accept:
    + args:
        + server socket
        + pointer to struct socketaddr_in
            + use it it to feed it with the client informations

+ TCP:
    + do a comunication between server and client

+ HTTP:
    + send a request:
        + specifying method and resource

+ inet_aton ??

+ all request ends width \r\n\n
+ check RFC 2616

+ see EPOLE: https://copyconstruct.medium.com/the-method-to-epolls-madness-d9d2d6378642
+ see recv FLAGS: https://man7.org/linux/man-pages/man2/recv.2.html

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

+ TCP/IP model:
    + network access layer:
        + physical connection and data framing
    + internet layer:
        + addressing packets
        + routing them over multiple interconnection networks
        + defin IP address
    + host-to-host layer:
        + TCP or UDP, 
    + Aplication layer:
        + HTTP, SMTP, and FTP
            + FTP: File trasnfer protocol

+ Sockets:
    + types:
        + connection oriented use TCP 
        + connection less use UDP
    
+ HTTP: Hyper Text Transfert Protocol
+ FTP : File Transfer Protocol
+ SSH : 
+ SMTP: simple email transfer protocol
+ TCP :
+ UDP:
    + used for real-time applications
    + streaming video
    + send message without expecting a response

+ C functions:
    + socket: 
        + create and initializes a new socket
    + bind: 
        + associates socket with a local IP address and port number
    + listen: 
        + to listen on new connections
    + connect: 
        + establish a connection
    + accept: 
        + create a new socket for incoming TCP connection
    + send/recv: 
        + send and receive
    + sendto/recvfrom: 
        + send and receive from sockets without bind remote address
    + close/closesocket:
        + terminate connection
    + shutdown:
        + close one side of a TCP connection
    + select:
        + wait for an event on one or more sockets
    + getnameinfo / getaddrinfo: ??
    + setsockopt: set socket options
    + fcntl/ioctlsocket: get and set socket options

SOCK_TREAM: TCP
SOCK_DGRAM: UDP
AI_PASSIVE: listen on any available network interface

+ kill processes that uses specific port:
    + kill -9 $(lsof -ti :17000)
    + netstat -tulpn (also usefull)

+ FD_ZERO : zero out the fd_set before use
+ FD_SET  : add a fd to the set
+ FD_CLR  : remove a fd from set
+ FD_ISSET: check presence of socket in the set

+ getaddrinfo:
    + char *node: hostname / address as string
    + char *service: service or port number as a string
    + addrinfo *hints:
        + ai_family (int):
            + AF_INET / AF_INET6/ AF_UNSPEC
        + ai_socktype (int):
            + SOCK_STREAM (TCP)
            + SOCK_DGRAM  (UDP)
            + 0 (TCP/UDP)
        + ai_protocol (int):
            + ??
        + ai_flags (int):
            + AI_NUMERICHOST: expect host with numerical value: yes for 127.0.0.1 no for google.com
            + AI_NUMERICSERV: accept online the port number from service name
            + AI_ALL: accept both IPv4 and IPv6 addresses
            + AI_ADDRCONFIG: ??
            + AI_PASSIVE: used with node = 0, thie local address accept connection on any of the
                host's network addreses
        + ai_addrlen (sockelen_t):
        + ai_addr (sockaddr*)
        + ai_cononname (char*)
        + ai_next (sockaddr*)
            + linked list
    + addrinfo **res:
+ getnameinfo:
    + sockaddr *addr:
    + socklen_t addrlen:
    + char *host: hostname or IP as text
    + socklen_t hostlen:
    + char *serv: hostname or IP as text of service
    + socklen_t servlen:
    + int flags:
        + NI_NAMEREQD: getnameinfo will return only hostname
        + NI_DGRAM:     using UDP
        + NI_NUMERIGHOST: getnameinfo will return an IP address as number
        + NI_NUMERISERV: getnameinfo will return port number not a service name 80 not http
    
+ HTTP request:
    + handle keep alive: ??

+ URL: http://www.example.com:80/res/page1.php?user=bob#account
    + Protocol: http
        + http, ftp, https
    + Hostname: www.example.com
        + could be also an IP address
    + Port: 80
    + Path: res/page1.php?user=bob
    + Hash: #account
        + jump to #account location inside html page

+ config file:
    server {
        listen  80 // PORT
        host    127... // IP address 
        server_name name
        error_pages { 
            // error pages could be given in config file
            404: file.html
            401: index.html
        }
        limit_body_size 700 // the max limit of full content

        location /path_name1 { (root)
            allowed-methods:
                GET, POST, DELETE
            redirection: path in machine directory (example /user/bin)
            Turn on or off directory listing. ???
        }

        location /path_name1 { (root)
            autoindex: (file to open in default case)
        }

        cgi .php program_that_execute_the_script (python3, php-fpm ...) ?    
    }

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








