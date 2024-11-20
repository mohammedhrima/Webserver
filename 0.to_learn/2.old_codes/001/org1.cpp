// HEADERS
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
typedef struct HTTP HTTP;
using namespace std;

// MACROS
#define CLIENT_TIMEOUT 10
#define POLLTIMEOUT 10
#define LISTEN_LEN SOMAXCONN
#define BUFFSIZE 7000
#define END (char *)"\r\n"
#define HEADEREND (char *)"\r\n\r\n"
#define ADDRLEN 4096

// COLORS
#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define RESET "\033[0m"

// DEBUGING
// stupid c++ compiler
string to_string(size_t size);
string substr(size_t line, string &str, ssize_t s, ssize_t e);
bool compare(ssize_t start, string left, string right);
#define substr_(str, s, e) substr(__LINE__, str, s, e)
#define debug cout << GREEN "line " << __LINE__ << " "
#define end_ RESET << endl

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
    {"application/octet-stream", ".bin"},
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

enum Type
{
    SERVER_ = 15,
    CLIENT_,
    FILE_
};

enum Status
{
    SUCCESS = 200,
    BAD_REQUEST = 400,
    FORBIDEN = 403,
    NOT_FOUND = 404,
    NOT_ALLOWED = 405,
    TIMEOUT = 408,
    INSUPPORTED_MEMETYPE = 415,
    INTERNAL = 500,
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
    map<int, string> errors;
    map<Method, bool> methods;
    // map<string, string> Data;

    string root;
    string upload;
    string index;
    bool autoindex;
    ssize_t limit; // TODO: chould not be negative or 0
    map<string, string> cgi;

    Location()
    {
        limit = -1;
        autoindex = false;
        methods[GET_] = false;
        methods[POST_] = false;
        methods[DELETE_] = false;
    }
    void set_method(size_t line, string &value)
    {
        if (value == "GET")
            methods[GET_] = true;
        else if (value == "POST")
            methods[POST_] = true;
        else if (value == "DELETE")
            methods[DELETE_] = true;
        else
            throw Error(line, "Invalid method");
    }
    // operator
    bool &operator[](const Method &key)
    {
        return methods[key];
    }
    ~Location() {}
};

struct Server
{
    string listen;
    ssize_t port;
    string name;
    map<string, Location> location;

    Server()
    {
        // all locations should starts with /
        location[""] = Location();
    }
    // methods
    void update_host(size_t line)
    {
        struct hostent *hostInfo = gethostbyname(listen.c_str());
        if (hostInfo == NULL)
            throw Error(line, "Unable to resolve hostname");
        struct in_addr **addressList = (struct in_addr **)(hostInfo->h_addr_list);
        if (addressList[0] != NULL)
            listen = inet_ntoa(*addressList[0]);
        else
            throw Error(line, "Invalid IP address");
    }
    ~Server() {}
};

string to_string(Type type);

enum Transfer
{
    CHUNKED_ = 44,
    // BOUNDARY_,
};

struct HTTP
{
    Action action;
    Transfer trans;
    string readbuff;
    string respbuff;
    // string requbuff;

    Method method;
    string uri;

    // string resp;
    string host;
    Status status;

    string ctype;
    ssize_t clen; // could be content-Length or chunk-Length
    // ssize_t flen;
    // map<string, string> params;
    string queries;
    bool ready; // ready to response
    bool keep_alive;
    time_t timeout;

    bool is_cgi;
    string cgi_fname;
    pid_t pid;

    bool close_connection;

    // TODO: check all constractor, they should set all values with 0
    HTTP(Action action_)
    {
        trans = (Transfer)0;
        method = (Method)0;
        status = (Status)0;
        action = action_;
        clen = 0;
        ready = false;
        keep_alive = false;
        timeout = 0;
        pid = 0;
        is_cgi = false;
        close_connection = false;
    }
    bool is_error()
    {
        return status >= 400 && status <= 511;
    }
    bool check(int line, bool cond, Status status_) // TODO: check every place where is called if is valid
    {
        if (cond)
        {
            ready = true;
            status = status_;
        }
        return cond;
    }
    string cut(ssize_t s, ssize_t &e)
    {
        e = s;
        while (e < readbuff.length() && !isspace(readbuff[e]))
            e++;
        if (s == e)
            return "";
        return substr_(readbuff, s, e);
    }
    void update_timeout()
    {
        timeout = time(NULL);
        // if (pair)
        //     pair->timeout = timeout;
    }
    void refresh() // TOOD: use it to clean the request after finishing response
    {
        trans = (Transfer)0;
        method = (Method)0;
        status = (Status)0;
        clen = 0;
        ready = false;
        pid = 0;
        is_cgi = false;
        cgi_fname = "";
        action = READ_;
        respbuff = ""; // TODO: to be verified
        uri = "";
        host = "";
        ctype = "";
        queries = "";
        // timeout = update_timeout();
        is_cgi = false;
        cgi_fname = "";
        pid = 0;
        close_connection = false;
    }
    ~HTTP()
    {
        if (pid)
        {
            cout << "kill child process " << pid << endl;
            kill(pid, SIGKILL);
        }
    }
};

// GLOBALS
// extern map<int, Connection> socks;
extern size_t pos;
extern size_t line;
extern vector<pollfd> pfds;
extern map<int, Connection *> socks;

string con_state(Connection &sock);
struct Connection
{
    // TODO: use an enum to define wheter is parsing header, or something else...
    // Server *srv;
    Type type;
    HTTP *http;
    // TODO: set limit for accepted addresses
    char address[ADDRLEN];

    // Connection *pair;

    int fd;
    Connection(int fd_, Type type_, HTTP *http_)
    {
        // is_cgi = false;
        fd = fd_, type = type_, http = http_;
        // timeout = 0;
        // if (type == SERVER_)
        // {

        // }
        // else
        if (type == FILE_ || type == CLIENT_)
        {
            if (fd > 0)
            {
                if (socks[fd])
                    throw Error("weird error 1"); // TODO: to be removed
                pfds.push_back((pollfd){.fd = fd, .events = POLLIN | POLLOUT});
                if (type == CLIENT_)
                    set_non_blocking();
            }
            // if (pair)
            //     pair->pair = this;
        }
        socks[fd] = this;
        if (http)
            http->update_timeout();
    }

    // methods
    void set_non_blocking()
    {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1)
            throw Error(string("fcntl\n"));
        flags |= O_NONBLOCK;
        if (fcntl(fd, F_SETFL, flags) < 0)
            throw Error(string("fcntl\n"));
    }

    string &buff()
    {
        /*
            FILE + READ + GET   -> respbuff
            FILE + READ + POST  -> X
            FILE + WRITE + GET  -> X
            FILE + WRITE + POST -> readbuff

            CLIE + READ + GET   -> readbuff
            CLIE + READ + POST  -> readbuff
            CLIE + WRITE + GET  -> respbuff
            CLIE + WRITE + POST -> readuff
        */
        Method &method = http->method;
        Action &action = http->action;
        if (method == 0 || http->ready == false)
        {
            // cout << "get readbuff" << endl;
            return http->readbuff;
        }
        if (type == FILE_ && (action == READ_ || action == WRITE_) && method == GET_)
        {
            // cout << "get respbuff" << endl;
            return http->respbuff;
        }
        //////////////////////////////////////////////////////////
        if (type == FILE_ && (action == READ_ || action == WRITE_) && method == POST_)
        {
            // cout << "get readbuff" << endl;
            return http->readbuff;
        }
        if (type == CLIENT_ && action == READ_ && method == POST_)
        {
            // cout << "get readbuff" << endl;
            return http->readbuff;
        }
        /////////////////////////////////////////////////////////////
        if (type == CLIENT_ && action == READ_ && method == GET_)
        {
            // cout << "get respbuff" << endl;
            return http->respbuff;
        }
        if (type == CLIENT_ && action == WRITE_ && method == POST_)
        {
            // cout << "get respbuff" << endl;
            return http->respbuff;
        }
        if (type == CLIENT_ && action == WRITE_ && method == GET_)
        {
            // cout << "get respbuff" << endl;
            return http->respbuff;
        }
        cout << con_state(*this) << endl;
        throw Error("in buffer getter");
        return http->readbuff;
    }
    // destractor
    ~Connection()
    {
        cout << "destroy " << fd << " has type " << to_string(type) << endl;
        close(fd);
        for (vector<pollfd>::iterator it = pfds.begin(); it != pfds.end(); ++it)
        {
            if (it->fd == fd)
            {
                pfds.erase(it);
                break;
            }
        }
        if (type == CLIENT_)
            delete http; // to be removed
        // if (pair)
        // {
        //     pair->pair = NULL;
        //     pair->http.refresh();
        // }
        socks[fd] = NULL; // TODO: to be verified
    }
};

// UTILS
string to_string(size_t size);
string to_string(Action action);
string to_string(Method method);
string to_string(Type type);
bool _isspace(char c);
bool _isdigits(string &str);
string substr(size_t line, string &str, ssize_t s, ssize_t e);
bool starts_with(string str, string sub);
bool ends_with(string str, string sub);
bool compare(string left, string right);
bool compare(ssize_t start, string left, string right);
ssize_t find(string &str, char *to_find);
string parse_hexadecimal(string &value);
string clear_path(string path);
string rand_name(); // TODO: verify it's return

// DEBUGING
void plocation(Location &location, int space);
void pserver(Server &serv);
string con_state(Connection &sock);

// PARSING CONFIG
vector<string> tokenize(string &text);
void expect(vector<string> &arr, string val);
string &get_value(vector<string> &arr);
string &get_value(vector<string> &arr, string val);
bool parse_location(vector<string> &arr, Location &curr);
Server parse(vector<string> &arr);
void check(Server &serv);
void check_vector(vector<Server> &servs);
void parse_config(vector<Server> &servs, char *filename);

/*
TODOS:

*/

map<string, string> memetype;
map<int, Connection *> socks;
vector<pollfd> pfds;

void parse_header(Connection &sock)
{
    HTTP &http = *sock.http;
    ssize_t e, s;
    ssize_t pos = find(sock.buff(), END);
    while (pos != string::npos)
    {
        // remember: I'm cutting the buff inside if statement
        if (!http.method)
        {
            if (compare(sock.buff(), "POST "))
            {
                http.method = POST_;
                http.uri = http.cut(strlen((char *)"POST "), e);
                // TODO: to be checked
                if (http.check(__LINE__, http.uri.empty() || http.uri[0] != '/', BAD_REQUEST))
                    break;

                cout << "POST to uri <" << http.uri << ">" << endl;
                if (http.check(__LINE__, !compare(e, sock.buff(), " HTTP/1.1\r\n"), BAD_REQUEST))
                    break;
            }
            else if (compare(sock.buff(), "GET "))
            {
                http.method = GET_;
                http.uri = http.cut(strlen((char *)"GET "), e);
                if (http.check(__LINE__, http.uri.empty(), BAD_REQUEST))
                    break;
                http.uri = parse_hexadecimal(http.uri);
                // TODO: check multiple "?" -> something like "???"
                ssize_t q_pos = find(http.uri, (char *)"?");
                if (q_pos != string::npos) // TODO: add query to POST and DELETE also
                {
                    // found queries
                    cout << "found ?" << endl;
                    s = q_pos;
                    while (!isspace(http.uri[s]) && s < http.uri.length())
                        s++;
                    http.queries = substr_(http.uri, q_pos + 1, s);
                    http.uri = substr_(http.uri, 0, q_pos);
                    cout << "queries <" << http.queries << ">" << endl;
                }
                bool cond = !compare(e, sock.buff(), " HTTP/1.1\r\n") && !compare(e, sock.buff(), " HTTP/1.0\r\n");
                if (http.check(__LINE__, cond, BAD_REQUEST))
                    break;
                cout << "GET to uri <" << http.uri << ">" << endl;
            }
            else if (http.check(__LINE__, true, BAD_REQUEST)) // if no method
                break;
        }
        else if (compare(sock.buff(), "Connection: "))
        {
            string con = http.cut(strlen((char *)"Connection: "), e);
            cout << "found connction <" << con << ">" << endl;
            if (con == "keep-alive")
                http.keep_alive = true;
        }
        else if (compare(sock.buff(), "Host: "))
        {
            http.host = http.cut(strlen((char *)"Host: "), e);
            cout << "found Host <" << http.host << ">" << endl;
            if (http.check(__LINE__, http.host.empty(), BAD_REQUEST))
                break;
        }
        else if (compare(sock.buff(), "Content-Type: "))
        {
            // TODO: add some insuported memtype or something ...
            http.ctype = http.cut(strlen((char *)"Content-Type: "), e);
            cout << "found Content-type <" << http.ctype << ">" << endl;
            if (http.check(__LINE__, http.ctype.empty(), BAD_REQUEST))
                break;
            debug << " <" << http.ctype << ">" << end_;
        }
        else if (compare(sock.buff(), "Content-Length: "))
        {
            string val = http.cut(strlen((char *)"Content-Length: "), e);
            cout << "found Content-Length, val <" << val << "> check " << (val.empty() || !_isdigits(val)) << endl;
            if (http.check(__LINE__, val.empty() || !_isdigits(val), BAD_REQUEST))
                break;
            // TODO: check if content=len is valid, should be all digits and postive
            http.clen = atol(val.c_str());
        }
        else if (compare(sock.buff(), "Transfer-Encoding: "))
        {
            string trans = http.cut(strlen((char *)"Transfer-Encoding: "), e);
            cout << "found Transfer-Encoding, trans <" << trans << ">" << endl;
            if (http.check(__LINE__, trans.empty(), BAD_REQUEST))
                break;
            if (trans == "chunked")
            {
                http.trans = CHUNKED_;
                cout << "Transfer-Encoding: chunked " << endl;
            }
            else
            {
                // TODO: bad request maybe !!
                cout << "Transfer-Encoding: Unknown " << endl;
            }
        }
        else if (compare(sock.buff(), END))
        {
            if (http.trans == CHUNKED_)
                http.clen = 0;
            else
                sock.buff() = substr_(sock.buff(), strlen(END), sock.buff().length());
            http.ready = true;
            break;
        }
        sock.buff() = substr_(sock.buff(), pos + strlen(END), sock.buff().length());
        pos = find(sock.buff(), END);
    }
}

void fetch_config(Connection &sock, vector<Server> &srvs)
{
    HTTP &http = *sock.http;
    Method &method = http.method;
    string &uri = http.uri;

    string match;
    string remain_uri;
    Location *match_location = NULL;
    state state_;
    // TODO: handle if hostname is empty
    size_t i = 0;
    while (i < srvs.size())
    {
        Server &srv = srvs[i];
        cout << "fetch: " << uri << " from " << srv.name << endl;
        map<string, Location>::iterator it;
        for (it = srv.location.begin(); it != srv.location.end(); it++)
        {
            string location_key = it->first;
            if (starts_with(uri, location_key) && srv.name == http.host)
            {
                if (location_key.length() > match.length()) // TODO: optimize it
                {
                    match_location = &it->second;
                    match = location_key;
                    remain_uri = substr_(uri, match.length(), uri.length());
                }
            }
        }
        i++;
    }
    if (match_location)
    {
        cout << GREEN "found " << match << RESET << endl;
        plocation(*match_location, 0);
        if (method == GET_)
        {
            // TODO: check if method is allowed
            string path = clear_path(match_location->root + "/" + uri);
            cout << "GET file " << path << endl;
            if (stat(path.c_str(), &state_) == 0 && state_.st_mode & S_IFREG)
            {
                ssize_t dot_pos = path.find_last_of(".");
                string ext;
                // TODO: to be checked
                if (dot_pos == string::npos || (ext = path.substr(dot_pos, path.length())).length() == 0)
                {
                    throw Error("Insuported metype");
                    http.status = INSUPPORTED_MEMETYPE;
                    return fetch_config(sock, srvs);
                }
                int fd = open(path.c_str(), O_RDONLY, 0777);
                if (fd < 0)
                {
                    // TODO: internal error
                    http.status = FORBIDEN;
                    return fetch_config(sock, srvs);
                }
                Connection *fsock = new Connection(fd, FILE_, &http);
                fsock->http->clen = state_.st_size;
                fsock->buff() = "HTTP/1.1 200 OK\r\nContent-Type: " +
                                memetype[ext] + "\r\nContent-Length: " +
                                to_string(fsock->http->clen) +
                                "\r\n\r\n";
                fsock->http->clen += fsock->buff().length();
                fsock->http->status = SUCCESS;
                fsock->http->action = READ_;
                return;
            }
        }
        else if (method == POST_ && match == uri)
        {
            cout << "ctype <" << http.ctype << ">" << endl;
            string path = clear_path(match_location->root + "/" + rand_name() + memetype[http.ctype]);
            cout << "POST file " << path << endl;
            int fd = open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0777);
            if (fd < 0)
            {
                // TODO: internal error, or permission denied
                http.status = FORBIDEN;
                return fetch_config(sock, srvs);
            }
            Connection *fsock = new Connection(fd, FILE_, &http);
            fsock->http->status = SUCCESS;
            if (fsock->buff().length())
                fsock->http->action = WRITE_;
            else
                fsock->http->action = READ_;
        }
        else
        {
            throw Error("bad request");
        }
    }
    else
    {
        cout << RED "Not found" RESET << endl;
    }
    // throw Error("debuging");
}

int init_server_socket(ssize_t port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        throw Error(string("socket: "));
    int option = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)))
        throw Error(string("setsockopt: "));

    sockaddr_in addr = (sockaddr_in){0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(fd, (sockaddr *)&addr, sizeof(addr)))
        throw Error(string("bind: "));
    if (listen(fd, LISTEN_LEN))
        throw Error(string("listen: "));
    return fd;
}

void Webserve(vector<Server> &servs)
{
    // SIGNALS
    struct sigaction sa = {};
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);

    // CREATE SERVERS SOCKETS
    size_t i = 0;
    while (i < servs.size())
    {
        Connection *sock = new Connection(init_server_socket(servs[i].port), SERVER_, NULL);
        pfds.push_back((pollfd){.fd = sock->fd, .events = POLLRDNORM});
        i++;
    }

    // WEBSERVING
    ssize_t timing = 0;
    cout << "Start webserve pfds size " << pfds.size() << endl;
    while (1)
    {
        timing++;
        if (timing == 10000000)
        {
            timing = 0;
            cout << "pfds size " << pfds.size() << endl;
        }
        int ready = poll(pfds.data(), pfds.size(), -1);
        if (ready < 0)
            throw Error("poll failed"); // to be removed
        else if (ready > 0)
        {
            ssize_t i = 0;
            while (i < pfds.size())
            {
                Connection &sock = *socks[pfds[i].fd];
                if (socks[pfds[i].fd])
                {
                    cout << con_state(sock) << endl;
                }
                if (pfds[i].revents & POLLERR)
                {
                    cout << RED "POLLERR" RESET << endl;
                    // delete &sock;
                    continue;
                }
                else if (sock.type == SERVER_ && pfds[i].revents)
                {
                    sockaddr_storage client_addr;
                    socklen_t len = sizeof(client_addr);
                    int cfd = accept(pfds[i].fd, (sockaddr *)&client_addr, &len);
                    if (cfd < 0)
                    {
                        throw Error("accept"); // TODO: do an internal error maybe
                        continue;
                    }
                    else if (cfd > 0)
                    {
                        Connection *csock = new Connection(cfd, CLIENT_, new HTTP(READ_));
                        inet_ntop(AF_INET, &(((struct sockaddr_in *)&client_addr)->sin_addr), csock->address, INET_ADDRSTRLEN);
                        cout << "New connection from " << cfd << " to: " << pfds[i].fd << " has addr " << csock->address << endl;
                    }
                }
                else if (pfds[i].revents & POLLIN && sock.http->action == READ_)
                {
                    char buff[BUFFSIZE];
                    ssize_t s, e;
                    ssize_t r = 0;
                    HTTP &http = *(sock.http);
                    if (http.trans == CHUNKED_ && http.clen > 0 && http.clen < BUFFSIZE)
                        r = read(pfds[i].fd, buff, http.clen);
                    else
                        r = read(pfds[i].fd, buff, BUFFSIZE);
                    if (r < 0)
                    {
                        throw Error("read failed");
                        continue;
                    }
                    else if (r == 0)
                    {
                        // TODO: check if r== 0 and buff.length() == 0, then close connection
                        // bool close_connection = false;
                        if (sock.buff().length() == 0)
                            sock.http->close_connection = true;
                        // if (http.method == POST_ && sock.type == FILE_)
                        // {
                        //     // throw Error("close connection");
                        //     close_connection = true;
                        // }
                        // http.refresh();
                        // http.close_connection = close_connection;
                        // throw Error("did read 0 has buff " + sock.buff());
                        // continue;
                    }
                    else if (r > 0)
                    {
                        sock.buff().append(buff, r);
                        http.update_timeout();
                    }
                    if (r > 0 && sock.type == CLIENT_)
                    {
                        if (http.ready == false)
                        {
                            parse_header(sock);
                            if (http.ready)
                            {
                                cout << GREEN "ready" RESET << endl;
                                http.check(__LINE__, http.host.empty() || (http.method == POST_ && http.ctype.empty()), BAD_REQUEST);
                                if (http.is_error() || http.method == GET_) // TODO: set it in fetch_config
                                    http.action = WRITE_;
                                fetch_config(sock, servs);
                            }
                        }
                        else
                        {
                            http.action = WRITE_;
                        }
                    }
                    else if (r > 0 && sock.type == FILE_)
                    {
                        debug << "read to file" << end_;
                        // throw Error(__LINE__, "debuging");
                        http.action = WRITE_;
                        if (r == http.clen)
                        {
                            throw Error("did read 2 has buff " + sock.buff());
                        }
                    }
                }
                else if (pfds[i].revents & POLLOUT && sock.http->action == WRITE_)
                {
                    // cout << "POLLOUT" << endl;
                    HTTP &http = *sock.http;
                    string &buff = sock.buff();
                    // debug << " send <" << sock.buff().c_str() << ">" << endl;
                    ssize_t bytes = buff.length() > BUFFSIZE ? BUFFSIZE : buff.length();
                    bytes = write(sock.fd, buff.c_str(), bytes);
                    if (bytes < 0) // TODO: to be removed
                    {
                        throw Error(__LINE__, "write failed");
                        continue;
                    }
                    else if (bytes > 0)
                    {
                        buff = substr_(buff, bytes, buff.length());
                        http.clen -= bytes;
                        http.update_timeout();

                        if (http.clen <= 0) // TODO: to be checked
                        {
                            // throw Error("did read 1 has buff " + sock.buff());
                            cout << RED "reached the end of file" RESET << endl;

                            http.action = READ_;
                            continue;
                        }
                        if (buff.length() == 0)
                        {
                            if (http.method == POST_ && sock.type == FILE_)
                                http.action = READ_;
                            // http.refresh();
                            // http.close_connection = true;
                        }
                    }
                }
                if (sock.type != SERVER_ && sock.http->close_connection && sock.http->keep_alive == false)
                {
                    debug << "close connection" << end_;
                    delete &sock;
                }
                i++;
            }
        }
    }
}

size_t pos = 0;
size_t line = 1;
int main(int argc, char **argv)
{
    vector<Server> servs;
    try
    {
        if (argc != 2)
            throw Error(string("Invalid number of arguments"));
        parse_config(servs, argv[1]);
        cout << "\n\n";
        for (size_t i = 0; i < servs.size(); i++)
            pserver(servs[i]);
        for (size_t i = 0; exts_struct[i].type.length(); ++i)
        {
            memetype[exts_struct[i].type] = exts_struct[i].ext;
            memetype[exts_struct[i].ext] = exts_struct[i].type;
        }
        Webserve(servs);
    }
    catch (exception &err)
    {
        cerr << RED "Error: " << err.what() << RESET << endl;
    }
    map<int, Connection *>::iterator it;
    for (it = socks.begin(); it != socks.end(); it++)
        delete it->second;
}