# Webserv - HTTP/1.1 Web Server

This project is a lightweight implementation of an HTTP/1.1 web server written in C++98, inspired by Nginx. It supports serving static files, basic CGI handling, and handling multiple configurations through a simple configuration file format. Developed as part of the 42 School curriculum.

---

## Features
- **HTTP/1.1 Support**: Implements GET, POST, and DELETE methods.
- **Static File Serving**: Serves files from specified directories.
- **CGI Support**: Executes `.py` and `.php` scripts with configurable interpreters.
- **Custom Configuration**: Easy-to-define server and location blocks.
- **Error Handling**: Custom error pages for common HTTP errors.
- **Autoindex**: Directory listing for requests without an index file.
- **Body Size Limit**: Restricts the size of request bodies.

---

## Requirements
- **Compiler**: `g++` with C++98 support.
- **Operating System**: Unix-based (Linux, macOS, etc.).

---

## Installation

### Clone the Repository
```bash
git clone https://github.com/mohammedhrima/Webserver.git
```
```bash
cd webserv
```
+ Build the Server
+ Run the make command to compile the project:
```bash
    make
```
### Usage
+ Start the Server
+ Run the compiled server binary with a configuration file:
```
    ./webserv ./conf/default.conf
```
- Example Configuration File (./conf/default.conf)
- This configuration file defines two servers (name1 and name2) with various settings like root directories, methods, and error pages. 
+ Below is a summary:


Server: name1
- Listen: localhost:17000
- Root: ./server/serv/name1
- Methods: GET, POST, DELETE
- Autoindex: Enabled
- CGI:
  - Python: /usr/bin/python3
  - PHP: /usr/bin/php-cgi

Server: name2
- Listen: 127.0.0.1:17000
- Root: ./server/serv/name2
- Methods: GET, POST, DELETE
- Autoindex: Enabled
- Custom Error Pages: Defined for 405, 408, 415, 500

For more details, refer to the comments in the default.conf file.

Directory Structure
```
.
├── Makefile                 # Build instructions
├── webserv                  # Compiled binary
├── conf/                    # Configuration files
│   └── default.conf         # Example configuration file
├── server/                  # Source code
└── scripts/                 # Helper scripts
```

Configuration File Format
The configuration file uses a block-style syntax. Below is a general breakdown:

Server Block
Defines server-level settings:

```
    {
        listen      : <host>:<port>;
        name        : <server_name>;
        root        : <document_root>;
        autoindex   : on | off;
        errors {
            <code>: <error_page_path>;
        }
        location <path> {
            methods     : <HTTP_methods>;
            index       : <index_file>;
            source      : <source_directory>;
            destination : <upload_directory>;
            limit       : <body_size_limit>;
            cgi         : <extension> <interpreter_path>;
        }
    }
```
+ Testing: to run the tests:
```
    make tests
```