#ifndef HEADER_HPP
#define HEADER_HPP

#include <poll.h>
#include <fcntl.h>
#include <sys/types.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <sys/time.h>
#include <cassert>
#include <signal.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <set>
#include <dirent.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// TYPEDEFS
typedef struct sockaddr_storage sockaddr_storage;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef struct addrinfo addrinfo;
typedef struct timeval timeval;
typedef struct dirent dirent;
typedef struct pollfd pollfd;
typedef struct stat state;
typedef struct Location Location;
typedef struct Server Server;
typedef struct Connection Connection;
typedef struct Data Data;
using namespace std;

// MACROS
#define SOCK_TIMEOUT 15
#define CGI_TIMEOUT 10
#define KEEP_ALIVE_TIMEOUT 30
#define LISTEN_LEN SOMAXCONN
#define READ_BUFFSIZE 16000
#define LIMIT_HOSTNAME 63
#define LIMIT_URI 4096
#define ADDRLEN 4096
#define CRLF (char *)"\r\n"
#define CRLFCRLF (char *)"\r\n\r\n"
#define LINE __LINE__
#define FILE __FILE__

// AUTOINDEX
#define on 1
#define off 2
#define none 3

// FILE STATS
#define IS_ACCESS S_IRUSR
#define IS_FILE S_IFREG
#define IS_DIR S_IFDIR

// COLORS
#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define RESET "\033[0m"

// DEBUGING
#define DEBUG cout << "line " << LINE << " "
#define INFO cout << __func__ << ", line " << LINE << ": "
#define END RESET << endl

struct
{
    string type;
    string ext;
} exts_struct[] = {
    {"audio/aac", ".aac"},
    {"application/x-abiword", ".abw"},
    {"image/apng", ".apng"},
    {"application/x-freearc", ".arc"},
    {"image/avif", ".avif"},
    {"video/x-msvideo", ".avi"},
    {"application/vnd.amazon.ebook", ".azw"},
    {"application/octet-stream", ""},
    {"image/bmp", ".bmp"},
    {"application/x-bzip", ".bz"},
    {"application/x-bzip2", ".bz2"},
    {"application/x-cdf", ".cda"},
    {"application/x-csh", ".csh"},
    {"text/css", ".css"},
    {"text/csv", ".csv"},
    {"application/msword", ".doc"},
    {"application/vnd.openxmlformats-officedocument.wordprocessingml.document", ".docx"},
    {"application/vnd.ms-fontobject", ".eot"},
    {"application/epub+zip", ".epub"},
    {"application/gzip", ".gz"},
    {"image/gif", ".gif"},
    {"text/html", ".html"},
    {"image/vnd.microsoft.icon", ".ico"},
    {"text/calendar", ".ics"},
    {"application/java-archive", ".jar"},
    {"image/jpeg", ".jpeg"},
    {"image/jpeg", ".jpg"},
    {"text/javascript", ".js"},
    {"application/json", ".json"},
    {"application/ld+json", ".jsonld"},
    {"audio/midi", ".mid"},
    {"audio/midi", ".midi"},
    {"text/javascript", ".mjs"},
    {"audio/mpeg", ".mp3"},
    {"video/mp4", ".mp4"},
    {"video/mpeg", ".mpeg"},
    {"application/vnd.apple.installer+xml", ".mpkg"},
    {"application/vnd.oasis.opendocument.presentation", ".odp"},
    {"application/vnd.oasis.opendocument.spreadsheet", ".ods"},
    {"application/vnd.oasis.opendocument.text", ".odt"},
    {"audio/ogg", ".oga"},
    {"video/ogg", ".ogv"},
    {"application/ogg", ".ogx"},
    {"audio/opus", ".opus"},
    {"font/otf", ".otf"},
    {"image/png", ".png"},
    {"application/pdf", ".pdf"},
    {"application/x-httpd-php", ".php"},
    {"application/vnd.ms-powerpoint", ".ppt"},
    {"application/vnd.openxmlformats-officedocument.presentationml.presentation", ".pptx"},
    {"application/vnd.rar", ".rar"},
    {"application/rtf", ".rtf"},
    {"application/x-sh", ".sh"},
    {"image/svg+xml", ".svg"},
    {"application/x-tar", ".tar"},
    {"image/tiff", ".tif"},
    {"image/tiff", ".tiff"},
    {"video/mp2t", ".ts"},
    {"font/ttf", ".ttf"},
    {"text/plain", ".txt"},
    {"application/vnd.visio", ".vsd"},
    {"audio/wav", ".wav"},
    {"audio/webm", ".weba"},
    {"video/webm", ".webm"},
    {"image/webp", ".webp"},
    {"font/woff", ".woff"},
    {"font/woff2", ".woff2"},
    {"application/xhtml+xml", ".xhtml"},
    {"application/vnd.ms-excel", ".xls"},
    {"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", ".xlsx"},
    {"application/xml", ".xml"},
    {"application/vnd.mozilla.xul+xml", ".xul"},
    {"application/zip", ".zip"},
    {"video/3gpp; audio/3gpp", ".3gp"},
    {"video/3gpp2; audio/3gpp2", ".3g2"},
    {"application/x-7z-compressed", ".7z"},
    {"", ""},
};

// STRUCTS, CLASSES, ENUMS
enum Action
{
    READ_ = 1,
    WRITE_,
};

enum Method
{
    GET_ = 10,
    POST_,
    DELETE_
};

enum HTTP_V
{
    HTTP1_0,
    HTTP1_1
};

enum Type
{
    SERVER_ = 15,
    CLIENT_,
    FILE_
};

enum Transfer
{
    CHUNKED_ = 20,
};

enum Status
{
    HTTP_SUCCESS = 200,
    HTTP_CREATED = 201,
    HTTP_MOVE_PERMANENTLY = 301,
    HTTP_TEMPORARY_REDIRECT = 307,
    HTTP_PERMANENT_REDIRECT = 308,
    HTTP_BAD_REQUEST = 400,
    HTTP_FORBIDEN = 403,
    HTTP_NOT_FOUND = 404,
    HTTP_METHOD_NOT_ALLOWED = 405,
    HTTP_TIMEOUT = 408,
    HTTP_LENGTH_REQUIRED = 411,
    HTTP_TO_LARGE = 413,
    HTTP_URI_TO_LARGE = 415,
    HTTP_INTERNAL_SERVER = 500,
    HTTP_METHOD_NOT_EMPLEMENTED = 501,
    HTTP_INSUPPORTED_HTTP = 505,
};

string to_string(size_t size);
class Error : public exception
{
private:
    string message;

public:
    Error(string msg) throw() : message(msg) {}
    Error(ssize_t line, string msg) throw()
    {
        message = "line " + to_string(line) + " " + msg;
    }
    virtual ~Error() throw() {}
    const char *what() const throw() { return message.c_str(); }
};

struct Location
{
    string key;
    map<int, string> errors;
    map<Method, bool> methods;

    string root;
    string dest;
    string src;

    bool is_return;
    ssize_t return_status;
    string return_location;

    string index;
    int autoindex;
    ssize_t limit_body;
    map<string, string> cgi;

    Location();
    void set_method(size_t line, string &value);
    bool &operator[](const Method &key);
    ~Location();
};

struct Server
{
    string listen;
    ssize_t port;
    string name;
    map<string, Location> location;

    Server();
    void update_host(size_t line);
    ~Server();
};

struct Data
{
    string requbuff;
    string respbuff;

    Action action;
    Transfer trans;
    string cause;

    Method method;
    // string listen;
    string domain;
    string uri;
    string host;
    HTTP_V http_v;
    string queries;
    string ctype;
    ssize_t clen;
    ssize_t chunk_len;
    ssize_t flen;
    bool keep_alive;

    size_t port;
    Status status;
    string srv_address;

    ssize_t read_size;
    ssize_t write_size;
    state st;
    bool header_parsed;
    bool finish_connection;
    bool response_header_sent;
    time_t last_activity;

    // CGI
    bool is_cgi;
    bool cgi_finished;
    bool cgi_header_parsed;
    bool cgi_read_first_line;
    string cgi_header;
    // size_t cgi_read_round;
    // size_t cgi_read_size;
    // size_t cgi_write_size;
    pid_t cgi_pid;
    string cgi_fname;
    string cgi_output;
    string cgi_input;
    string cgi_exec;

    Location *location;

    // header parsing
    bool connection_found;
    // bool host_found_in_uri;
    bool host_found;
    bool ctype_found;
    bool clen_found;
    bool trans_found;
    bool method_found;

    bool is_error();
    bool check(int line, bool cond, Status status_, string cause_);
    // string cut(ssize_t s, ssize_t &e);
    void init();
    void refresh();
};

struct Connection
{
    Type type;
    Data data;
    string address;

    int fd; // don't use fd
    int read_fd;
    int write_fd;
    void init();
    int open_error_page(string &path);
    // bool file_is_cgi(string &path, Location &location);
    // int open_cgi_file(string &path);
    int open_file(string &path);
    int readbuff(int src_fd);
    int writebuff(int dest_fd);
    int open_dir(string &path, string &uri);
    void close_connection(int line, int close_fd);

    void update_timeout();
    bool timeout();
};

// GLOBALS
extern size_t pos;
extern size_t line;
extern map<string, string> memetype;
extern vector<pollfd> pfds;
extern map<int, Connection> cons;
extern map<int, int> pairs;
extern map<int, Type> types;

// FUNCTIONS
string clear_path(string path);
string con_state(Connection &con);
bool ends_with(string str, string sub);
void parse_config(vector<Server> &servs, char *filename);
void plocation(Location &location, int space);
void pserver(Server &serv);
string con_state(Connection &con);
bool _isdigits(string &str);
ssize_t find(string &str, char *to_find);
bool compare(string left, string right);
bool compare(ssize_t start, string left, string right);
string substr(size_t line, string &str, ssize_t s, ssize_t e);
string parse_hexadecimal(string &value);
string clear_path(string path);
bool starts_with(string str, string sub);
string rand_name();
string generate_header(Data &data, string ctype, size_t size);
void generate_response(int line, Connection &con);
string to_lower(string &str);
bool parse_header(Connection &con);
bool http_version_is_valid(string line, string &value);
state get_state(string &path);
string to_string(Action action);
string to_string(Method method);
string to_string(Type type);
string to_string(size_t size);
string to_string(HTTP_V version);
string update_request_host(string listen);

#endif