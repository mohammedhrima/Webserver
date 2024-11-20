+ TODO:
	+ verify page 12: token, comment
	+ go page by page and check every possible case
	+ duplicated key : value
	+ verify page 33, for the status codes
	+ Location in header attributes ??, page 44,135
	+ test URL with no abs_path
	+ test uri with an % + ascci on it: 
		+ http://abc.com:80/~smith/home.html
		+ http://ABC.com/%7Esmith/home.html
		+ http://ABC.com:/%7esmith/home.html
	+ test send a file without trasnfer encoding
	+ test mutiple requests on same same http request
	+ test a location in config that has name same to file name in curr directory
	+ if URI is an http link redirect to it (non absolut uri), else is an absolute uri
	+ check page 40: to get error code with the value
	+ transfer-encoding required content length

	+ POST:
		+ check the conflict between 200 and 201
	+ GET:
		+ getting multiple files in one request
		+ check if I should add trasnfer-encoding
		+ if there is body in a header does not required it, ignore it

+ HTTP request:
	+ HEADER:
		name = definition
		name: no '<' '>'
		 '|': you took only the first or the second
		 ()	: elems inside () -> one elem
		  *	: 2*elem -> 2 elems, 1*2elemnt -> 1 or 2 elems, TODO: check it
		 []	: [elem foo] -> 1 * (elem foo)
		  N	: 2DIGIT -> 2 digits , 3ALPHA -> 3 alphas
		  # : <n>#<m>elem , at least n elems, at most m elems, seprated by ',' TODO: check it
		  ;	: comments
		+ examples:
			GET /path/to/ressource HTTP/1.0 -> simple request
			GET //example.net/path -> http://example.com/path
			GET http://www.example.com HTTP/1.0
	+ URI:
		+ % HEX 
	+ URL:
		+ "http:" + "//" + "host" + ":" + "port" + "abs_path" + "?" + "query"
		+ port is empty use 80
	+ BODY:
	-  pause: page 24

+ max header size
+ max body size
+ request uri (remove ..) after converting hex stuff
    - /.. -> bad request
    - /%2E%2E -> bad request (2E = hex of '.')
    - /a/../.. -> bad request
    - /%00 -> bad request

+ add return in config file

 + POST /upload: create a file write into it the body
    and return in response Location of it

+ cookies:
    recv Cookie value from request header line
    send it to cgi by setting environement HTTP_COOKIE=value
    get Set-Cookie value from cgi header output and send it to client

+ cgi:
    + accept both \r\n and \n in header output
    + the header lines you should use from cgi header:
        content-type, status, content-length, location, set-cookie
    + if the cgi actual output (file size minus header size) is less than content-length
        then resest it to the correct value
    + if anything goes wrong return error code internal error
    + add timeout

    + add environement "REDIRECT_STATUS=CGI" for php to work

    + environements:
        QUERY_STRING=query value in uri exactly as the reqeuest don't replace %20 with space for example
        PATH_INFO=full path of the script file (after appending root)

        CONTENT_LENGTH= body size after removing chunked
                    and set it only in POST method
        CONTENT_TYPE= value of content type in client request
                    and set it only in POST method
        REQUEST_METHOD=GET or POST
        GATEWAY_INTERFACE=CGI/1.1
        REMOTE_ADDR=client address (the one you get after calling accept)
        SCRIPT_NAME=uri of script (ex: /cgi-bin/test.py)
        SERVER_NAME= ??
        SERVER_PORT=which port the client connected to
        SERVER_PROTOCOL=HTTP/1.1
        SCRIPT_FILENAME=same as PATH_INFO
        REDIRECT_STATUS=CGI
        HTTP_COOKIE=value of Cookie in client request

    + set a max events
    + check all constractor, they should set all values with 0
    + remove all throw except from parsing
    + use limit body-size
    + test one client from two servers
    + multiple keyword on http request -> bad request
    + finish the first request then handle the second one
    + max header size
    + max body size
    + timeout thing
    + CGI: accept only .py .php
    + use realpath function, specially in DELETE
    + handle also HTTP/1.0
    + CGI: check RFC for all value that I have to set
    + CGI: check if output contains header if not add it
    + CGI: add query to POST and DELETE also
    + reset HTTP after finishing reponse
    + arguement should ends with .conf
    + ip adress should be d.d.d.d -> 0 <= d <= 255
    + check all TODOS in all (.cpp and hpp) files, and to be removed
    + read body in GET based on content-length
    + empty file cause the program to hang, fix it
    + check accesibility with access, if open,read,write failed return internal error
    + check that data get clear after finishing response
    + handle return
    + check limit hostname
    + ../ in location in config file -> error
    + handle nc request with new lines instead of \r\n
    + test nginx request with no host
    + check Status enums if it has all it's error messages in generate response
    + check all errors one by one
    + support only php and python CGI
    + test 5 servers with different port and listen keys
    + handle DELETE
    + all function should take param as const
    + check max_body size value in config file should be > 0
    + use limit uri
    + content-type could be like: text/html; charset=ISO-8859-4, handle it
    + when parsing uri:
        + if uri is like scheme://host[:port]/path?query#fragment:
            scheme could be http in may case
        + if host found in uri ignore Host: header value
    + if no host: bad request
    + send Connection close, if you are willing to close connection
    + check if you have to add .. and . in directory listing
    + check file not found POST http://localhost:17000/cgi/file.py
    + check all possible errors and failure
    + GET file not found in CGI
    + if format is bad in cgi the file kept opened -> close it
    + I guess cgi, only needs content-type and content-length
    + if index file is cgi execute it
    + POST to location not found

+ CMD:
    + ls /proc/$(ps -aux | grep "a.out" | awk 'NR==1{print $2}')/fd | wc -w

+ GET
+ POST

+ CGI
+ test directory listing and autoindex, index file when asking for directory
+ handle multiple requests on same request
+ DELETE
+ chunked
+ cookies


+ /abcd
+ /python

localhost:8080/python.py
+ /ab


+ to test:
    printf "" | nc 127.0.0.1 17000

    # b for benchmark time in s or m
    siege -t 10s -b http://127.0.0.1:17000 

    mkdir homebrew && curl -L https://github.com/Homebrew/brew/tarball/master | tar xz --strip 1 -C homebrew
    eval "$(homebrew/bin/brew shellenv)"
    brew update --force --quiet