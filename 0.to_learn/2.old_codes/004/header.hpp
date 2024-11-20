#ifndef HEADER
#define HEADER

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
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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
#define SOCK_TIMEOUT 5
#define KEEP_ALIVE_TIMEOUT 30

#define POLLTIMEOUT 10
#define LISTEN_LEN SOMAXCONN
#define BUFFSIZE 16000
#define LIMIT_HOSTNAME 63
#define LIMIT_URI 4096
#define CR "\r"
#define LF "\n"
#define CRLF (char *)"\r\n"
#define CRLFCRLF (char *)"\r\n\r\n"
#define ADDRLEN 4096
#define MACRO_TO_STRING(f) #f
#define LINE __LINE__
#define FILE __FILE__
#define FUNC __func__

// AUTOINDEX
#define none 0
#define on 1
#define off 2

// STATS
#define IS_ACCESS S_IRUSR
#define IS_FILE S_IFREG
#define IS_DIR S_IFDIR

// COLORS
#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define CYAN "\033[0;36m"
#define RESET "\033[0m"

// DEBUGING
#define DEBUG cout << GREEN << "line " << LINE << " "
#define INFO cout << CYAN << __func__ << ", line " << LINE << ": "
#define END RESET << endl;
#define ERROR cerr << RED

// STRUCTS, CLASSES, ENUMS

struct Type_ext
{
    string type;
    string ext;
};

enum Action
{
    READ_ = 1,
    WRITE_,
};

enum Method
{
    GET_ = 5,
    POST_,
    DELETE_
};

enum HTTP_version
{
    HTTP1_0 = 10,
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
    HTTP_OK = 200,
    HTTP_CREATED = 201,
    // Redirection
    HTTP_MOVE_PERMANENTLY = 301,
    HTTP_TEMPORARY_REDIRECT = 307,
    HTTP_PERMANENT_REDIRECT = 308,
    // Errors
    HTTP_BAD_REQUEST = 400,
    HTTP_FORBIDEN = 403,
    HTTP_NOT_FOUND = 404,
    HTTP_METHOD_NOT_ALLOWED = 405,
    HTTP_TIMEOUT = 408,
    HTTP_LENGTH_REQUIRED = 411,
    HTTP_CONTENT_TO_LARGE = 413,
    HTTP_URI_TO_LARGE = 414,
    HTTP_HEADER_TO_LARGE = 431,
    // Server Errors
    HTTP_INTERNAL_SERVER = 500,
    HTTP_METHOD_NOT_IMPLEMENTED = 501,
    HTTP_BAD_GATEWAY = 502,
    HTTP_GATEWAY_TIMEOUT = 504,
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

struct CGI
{
    int in_fd;
    int out_fd;
    int src_fd;

    string input;
    string output;
    pid_t pid;

    string exec;
};

struct Location
{
    string location;

    string root;
    map<Method, int> methods;
    string src;
    string dest;

    bool is_ret;
    Status ret_status;
    string ret_location;

    int autoindex;

    string index;
    bool has_limit;
    ssize_t limit;

    map<string, string> cgi;

    map<int, string> errors;
};

struct Server
{
    string listen;
    ssize_t port;
    string name;
    // string address;

    map<string, Location> locations;
};

struct Data
{
    Method method;
    string uri;
    vector<string> hosts;
    HTTP_version http_version;
    string queries;
    string ctype;
    ssize_t clen;
    ssize_t flen;
    string connection;
    bool keep_alive;
    Status status;
    string cause;
    Transfer trans;
    state st;
    Location *matched_location;

    bool header_parsed;
    bool trans_found;
    bool clen_found;
    bool ctype_found;
    bool host_found;
    bool connection_found;
};

struct Connection
{
    // server who accepted connection
    ssize_t srv_port;
    string srv_listen;
    string address;

    Type type;
    int fd;
    // add void pointer
    int read_fd;
    int write_fd;

    Action action;
    Data data;
    // Data Request
    // Data response

    string readbuff;
    string writbuff;

    // ssize_t read_size;
    ssize_t write_size;
    time_t timeout;
};

typedef vector<Server> vServer;
typedef map<size_t, vServer> mServers;

// GLOBALS
// struct Global
// {
extern map<int, int> pairs;
extern map<int, Connection> cons;
extern vector<pollfd> pfds;
extern map<int, Type> types;
extern map<size_t, vServer> servers;
extern map<string, string> memetype;
extern string machine_ip_address;
// };

// DEBUG
ostream &operator<<(ostream &out, Connection &con);
ostream &operator<<(ostream &out, Server &serv);
ostream &operator<<(ostream &out, Location &loc);

// FUNCTIONS
int create_server_connection(Server &srv);
void connection_accept(int serv_fd);

bool _isdigits(string &str);
string substr(size_t line, string &str, ssize_t s, ssize_t e);
bool starts_with(string str, string sub);
bool ends_with(string str, string sub);
string clear_path(string path);
string rand_name();
mServers parse_config(char *filename);
void init_memetypes();
bool parse_header(Connection &con);
string parse_hexadecimal(string &value);
ssize_t find(string &str, char *to_find);
bool is_error(Data &data);
bool is_move_dir(Data &data);
string to_lower(string &str);
string to_string(HTTP_version version);
string to_string(Method method);
string to_string(size_t size);
string to_string(Type type);
state get_state(string &path);
bool check(int line, Connection &con, bool cond, Status status_, string cause_);
void see_pfds_size();
void connection_close(int line, int fd);

#endif