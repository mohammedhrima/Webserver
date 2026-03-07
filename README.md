# Webserv - HTTP/1.1 Web Server 🌐

A lightweight, high-performance HTTP/1.1 web server written in C++98, inspired by Nginx. Serve static files, execute CGI scripts, and handle multiple virtual hosts with a simple configuration file. Built as part of the 42 School curriculum.

## 🎯 What Does It Do?

Webserv is a fully functional web server that can:

- **Serve Static Websites**: Host HTML, CSS, JavaScript, images, and other static files
- **Execute CGI Scripts**: Run Python and PHP scripts dynamically
- **Multiple Virtual Hosts**: Host multiple websites on different ports or domains
- **Custom Error Pages**: Display branded error pages for 404, 500, etc.
- **Directory Listing**: Automatically generate directory indexes (autoindex)
- **File Uploads**: Accept file uploads via POST requests
- **HTTP Methods**: Support GET, POST, and DELETE operations
- **Request Body Limits**: Protect against large uploads
- **Configuration File**: Easy setup with Nginx-style configuration

## 👤 Who Is It For?

- Web developers learning server architecture
- Students studying HTTP protocols
- Developers building custom web servers
- System administrators understanding web hosting
- Anyone interested in low-level web technologies

## 🚀 How to Use

### Prerequisites

- **C++ Compiler**: g++ with C++98 support
- **Make**: Build automation tool
- **Unix-based OS**: Linux, macOS, or WSL on Windows

### Installation

#### 1. Clone the Repository

```bash
git clone https://github.com/mohammedhrima/Webserver.git
cd webserv
```

#### 2. Build the Server

```bash
make
```

This compiles the source code and creates the `webserv` executable.

### Running the Server

#### Start with Default Configuration

```bash
./webserv ./conf/default.conf
```

The server will start and listen on the ports specified in the configuration file.

#### Start with Custom Configuration

```bash
./webserv /path/to/your/config.conf
```

### Accessing Your Website

Once the server is running:

1. **Open your browser**
2. **Navigate to**: `http://localhost:17000` (or your configured port)
3. **Browse your website**

## 🌐 Using the Web Server

### Serving Static Files

**Setup:**
1. Place your HTML, CSS, and JavaScript files in the configured root directory
2. Access them via browser: `http://localhost:17000/index.html`

**Example:**
```
server/serv/name1/
├── index.html
├── style.css
├── script.js
└── images/
    └── logo.png
```

Access: `http://localhost:17000/index.html`

### Directory Listing (Autoindex)

When autoindex is enabled and no index file exists:

1. Navigate to a directory: `http://localhost:17000/images/`
2. See automatic directory listing with all files
3. Click files to view or download

### Uploading Files

**Via HTML Form:**
```html
<form action="/upload" method="POST" enctype="multipart/form-data">
  <input type="file" name="file">
  <button type="submit">Upload</button>
</form>
```

**Via curl:**
```bash
curl -X POST -F "file=@document.pdf" http://localhost:17000/upload
```

Files are saved to the configured destination directory.

### Running CGI Scripts

**Python Script Example:**

Create `script.py`:
```python
#!/usr/bin/env python3
print("Content-Type: text/html\n")
print("<h1>Hello from Python CGI!</h1>")
```

Access: `http://localhost:17000/script.py`

**PHP Script Example:**

Create `info.php`:
```php
<?php
phpinfo();
?>
```

Access: `http://localhost:17000/info.php`

### Deleting Files

```bash
curl -X DELETE http://localhost:17000/uploads/file.txt
```

### Custom Error Pages

When errors occur (404, 500, etc.), the server displays your custom error pages:
- `http://localhost:17000/nonexistent` → Shows custom 404 page
- Server error → Shows custom 500 page

## ⚙️ Configuration

### Configuration File Format

The server uses a block-style configuration similar to Nginx:

```nginx
{
    listen      : localhost:8080;
    name        : my-website;
    root        : /var/www/html;
    autoindex   : on;
    
    errors {
        404: /errors/404.html;
        500: /errors/500.html;
    }
    
    location /api {
        methods     : GET POST;
        index       : index.html;
        source      : /var/www/api;
        destination : /var/www/uploads;
        limit       : 10485760;
        cgi         : .py /usr/bin/python3;
        cgi         : .php /usr/bin/php-cgi;
    }
}
```

### Configuration Options

#### Server Block

| Directive | Description | Example |
|-----------|-------------|---------|
| `listen` | Host and port to listen on | `localhost:8080` |
| `name` | Server name (virtual host) | `example.com` |
| `root` | Document root directory | `/var/www/html` |
| `autoindex` | Enable directory listing | `on` or `off` |

#### Error Pages

```nginx
errors {
    404: /errors/404.html;
    405: /errors/405.html;
    500: /errors/500.html;
}
```

#### Location Block

| Directive | Description | Example |
|-----------|-------------|---------|
| `methods` | Allowed HTTP methods | `GET POST DELETE` |
| `index` | Default index file | `index.html` |
| `source` | Source directory for this location | `/var/www/api` |
| `destination` | Upload destination directory | `/var/www/uploads` |
| `limit` | Max request body size (bytes) | `10485760` (10MB) |
| `cgi` | CGI interpreter for file extension | `.py /usr/bin/python3` |

### Example Configurations

#### Simple Static Website

```nginx
{
    listen      : localhost:8080;
    name        : my-site;
    root        : ./website;
    autoindex   : off;
    
    location / {
        methods : GET;
        index   : index.html;
    }
}
```

#### Multiple Virtual Hosts

```nginx
# Site 1
{
    listen : localhost:8080;
    name   : site1;
    root   : ./site1;
}

# Site 2
{
    listen : localhost:8081;
    name   : site2;
    root   : ./site2;
}
```

#### CGI-Enabled Server

```nginx
{
    listen : localhost:8080;
    root   : ./cgi-site;
    
    location / {
        methods : GET POST;
        cgi     : .py /usr/bin/python3;
        cgi     : .php /usr/bin/php-cgi;
    }
}
```

## 📁 Project Structure

```
webserv/
├── Makefile                 # Build instructions
├── webserv                  # Compiled binary
├── conf/
│   └── default.conf         # Example configuration
├── server/
│   ├── main.cpp            # Entry point
│   ├── Server.cpp          # Server class
│   ├── Request.cpp         # HTTP request parser
│   ├── Response.cpp        # HTTP response builder
│   ├── Config.cpp          # Configuration parser
│   └── CGI.cpp             # CGI handler
└── server/serv/
    ├── name1/              # Virtual host 1 files
    └── name2/              # Virtual host 2 files
```

## 🛠️ Technical Stack

- **C++98**: Core programming language
- **Socket Programming**: TCP/IP networking
- **HTTP/1.1 Protocol**: Request/response handling
- **CGI**: Common Gateway Interface for dynamic content
- **File I/O**: Static file serving
- **Make**: Build system

## 🔧 HTTP Methods Supported

### GET
- Retrieve static files
- Execute CGI scripts
- Directory listing (if autoindex enabled)

**Example:**
```bash
curl http://localhost:17000/index.html
```

### POST
- Upload files
- Submit form data
- Execute CGI scripts with input

**Example:**
```bash
curl -X POST -d "name=John&age=30" http://localhost:17000/form.py
```

### DELETE
- Remove files from server

**Example:**
```bash
curl -X DELETE http://localhost:17000/uploads/file.txt
```

## 🧪 Testing

### Run Test Suite

```bash
make tests
```

This runs automated tests for:
- HTTP request parsing
- Response generation
- CGI execution
- File operations
- Configuration parsing

### Manual Testing

**Test Static Files:**
```bash
curl http://localhost:17000/index.html
```

**Test CGI:**
```bash
curl http://localhost:17000/test.py
```

**Test Upload:**
```bash
curl -X POST -F "file=@test.txt" http://localhost:17000/upload
```

**Test Delete:**
```bash
curl -X DELETE http://localhost:17000/uploads/test.txt
```

**Test Error Pages:**
```bash
curl http://localhost:17000/nonexistent
```

### Load Testing

```bash
# Using Apache Bench
ab -n 1000 -c 10 http://localhost:17000/

# Using wrk
wrk -t4 -c100 -d30s http://localhost:17000/
```

## 📊 Features in Detail

### HTTP/1.1 Compliance
- Persistent connections (Keep-Alive)
- Chunked transfer encoding
- Content-Type detection
- Status codes (200, 404, 405, 500, etc.)

### CGI Support
- Environment variable passing
- Standard input/output handling
- Python and PHP interpreters
- Custom interpreter configuration

### Security Features
- Request body size limits
- Method restrictions per location
- Path traversal prevention
- Input validation

### Performance
- Non-blocking I/O
- Efficient file serving
- Connection pooling
- Minimal memory footprint

## 🐛 Troubleshooting

**Port already in use:**
```bash
# Find process using port
lsof -i :17000

# Kill the process
kill -9 <PID>

# Or change port in config file
```

**Permission denied:**
```bash
# Ensure files are readable
chmod 644 server/serv/name1/*

# Ensure directories are executable
chmod 755 server/serv/name1/
```

**CGI not working:**
```bash
# Check interpreter path
which python3
which php-cgi

# Update config with correct path
cgi : .py /usr/bin/python3;

# Make script executable
chmod +x script.py
```

**Configuration errors:**
```bash
# Check syntax
./webserv ./conf/default.conf

# Server will report parsing errors
```

## 📈 Future Enhancements

- [ ] HTTPS/TLS support
- [ ] HTTP/2 protocol
- [ ] WebSocket support
- [ ] Load balancing
- [ ] Caching mechanisms
- [ ] Compression (gzip)
- [ ] Access logging
- [ ] Authentication (Basic, Digest)
- [ ] Rate limiting
- [ ] IPv6 support

## 🤝 Contributing

Contributions welcome! Great project for learning:
- HTTP protocol internals
- Socket programming
- C++ systems programming
- Web server architecture

## 📚 Resources

**Learn More:**
- [HTTP/1.1 RFC 2616](https://www.rfc-editor.org/rfc/rfc2616)
- [CGI Specification](https://www.w3.org/CGI/)
- [Nginx Documentation](https://nginx.org/en/docs/)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

## 📄 License

This project is open source and available for educational purposes.

## 🙏 Acknowledgments

- 42 School for the project specifications
- Nginx for configuration inspiration
- HTTP/1.1 specification authors