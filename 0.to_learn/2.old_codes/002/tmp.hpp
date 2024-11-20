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
#define SOCK_TIMEOUT 5
#define KEEP_ALIVE_TIMEOUT 30

#define POLLTIMEOUT 10
#define LISTEN_LEN SOMAXCONN
#define READ_BUFFSIZE 16000
#define LIMIT_HOSTNAME 63
#define LIMIT_URI 4096
#define CRLF (char *)"\r\n"
#define CRLFCRLF (char *)"\r\n\r\n"
#define ADDRLEN 4096
#define MACRO_TO_STRING(f) #f
#define LINE __LINE__
#define FILE __FILE__

// AUTOINDEX
#define on 1
#define off 2
#define none 3
// STATS
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

string substr(size_t line, string &str, ssize_t s, ssize_t e);
bool compare(ssize_t start, string left, string right);

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
// TODO: check all placess where use create header
// and see the http version
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

// TODO: add all messages for those errors
enum Status
{
    HTTP_SUCCESS = 200,
    HTTP_CREATED = 201, // TODO: use it
    // NO_CONTENT = 204,
    HTTP_MOVE_PERMANENTLY = 301,
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

string to_string(Action action);
string to_string(Method method);
string to_string(Type type);
string to_string(size_t size);
string to_string(HTTP_V version);

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

    Location()
    {
        limit_body = -1;
        autoindex = none;
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

struct Data
{
    string requbuff;
    string respbuff;

    Action action;
    Transfer trans;
    string cause;

    Method method;
    string listen;
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

    bool is_error()
    {
        return (status >= 400 && status <= 511);
    }
    bool check(int line, bool cond, Status status_, string cause_)
    {
        if (cond)
        {
            cout << "check did found error in line " << line << endl;
            cause = cause_;
            status = status_;
            action = WRITE_;
            header_parsed = true;
        }
        return cond;
    }
    string cut(ssize_t s, ssize_t &e)
    {
        e = s;
        while (e < requbuff.length() && !isspace(requbuff[e]))
            e++;
        if (s == e)
            return "";
        return substr(LINE, requbuff, s, e);
    }
    void update_timeout()
    {
        last_activity = time(NULL);
    }
    bool timeout()
    {
        bool res = (!keep_alive && (time(NULL) - last_activity > SOCK_TIMEOUT)) ||
                   (keep_alive && (time(NULL) - last_activity > KEEP_ALIVE_TIMEOUT));
        if (res)
            cout << "timeout " << endl;
        return (res);
    }
    void init()
    {
        *this = (Data){};

        action = READ_;
        ctype = "application/octet-stream";
    }
    void refresh()
    {
        *this = (Data){};
        action = READ_;
        ctype = "application/octet-stream";
    }
};

// GLOBALS
extern size_t pos;
extern size_t line;
extern map<string, string> memetype;
extern vector<pollfd> pfds;
extern map<int, Connection> cons;
extern map<int, int> pairs;
extern map<int, Type> types;

string clear_path(string path);
string con_state(Connection &con);
bool ends_with(string str, string sub);
// FUNCTIONS
void parse_config(vector<Server> &servs, char *filename);
void plocation(Location &location, int space);
void pserver(Server &serv);
string con_state(Connection &con);
bool _isdigits(string &str);
ssize_t find(string &str, char *to_find);
bool compare(string left, string right);
bool compare(ssize_t start, string left, string right);
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

struct Connection
{
    Type type;
    Data data;
    char address[ADDRLEN];
    // int index;

    int fd; // don't use fd
    int read_fd;
    int write_fd;

    void init()
    {
        *this = (Connection){};
        data.init();
        fd = -1;
        read_fd = -1;
        write_fd = -1;
    }
    int open_error_page(string &path)
    {
        int new_fd = -1;
        new_fd = open(path.c_str(), O_RDONLY, 0777);
        if (new_fd < 0)
        {
            cerr << RED << "opening error page" RESET << endl;
            return new_fd;
        }
        ssize_t dot = path.find_last_of(".");
        string ext;
        if (dot != string::npos && memetype.count((ext = substr(LINE, path, dot, path.length()))))
            data.ctype = memetype[ext];
        else
            data.ctype = "application/octet-stream";

        cerr << RED "error page has fd: " RESET << new_fd << endl;
        pfds.push_back((pollfd){.fd = new_fd, .events = POLLIN | POLLOUT});

        string header = generate_header(data, data.ctype, data.st.st_size);
        data.clen = data.st.st_size;

        data.respbuff = header;
        data.flen = data.clen + data.respbuff.length();

        data.action = WRITE_;
        read_fd = new_fd;
        write_fd = fd;
        types[new_fd] = FILE_;
        pairs[new_fd] = fd;
        data.read_size = 0;
        data.write_size = 0;

        return new_fd;
    }
    int open_cgi_file(string &path)
    {
        int new_fd = -1;

        cout << "open file: " << path << endl;
        if (data.method == GET_)
        {
            ssize_t dot = path.find_last_of(".");
            string ext;
            if (dot != string::npos && memetype.count((ext = substr(LINE, path, dot, path.length()))))
                data.ctype = memetype[ext];
            new_fd = open(path.c_str(), O_RDONLY, 0777);
            if (new_fd < 0)
            {
                cerr << RED << "openning " << path << RESET << endl;
                data.status = HTTP_INTERNAL_SERVER;
                return new_fd;
            }
            pfds.push_back((pollfd){.fd = new_fd, .events = POLLIN | POLLOUT});
            // data.respbuff = generate_header(data, data.ctype, data.st.st_size);
            data.clen = data.st.st_size;
            data.flen = data.clen;
            data.action = READ_;
            read_fd = new_fd;
            write_fd = fd;
            data.cgi_header_parsed = false;
            // data.cgi_read_round = 0;
            cout << "New cgi file connection has fd " << new_fd << " from client " << fd << " size " << data.flen << endl;
            types[new_fd] = FILE_;
            pairs[new_fd] = fd;
        }
        return new_fd;
    }
    int open_file(string &path)
    {
        int new_fd = -1;

        cout << "open file: " << path << endl;
        if (data.method == GET_)
        {
            ssize_t dot = path.find_last_of(".");
            string ext;
            if (dot != string::npos && memetype.count((ext = substr(LINE, path, dot, path.length()))))
                data.ctype = memetype[ext];
            new_fd = open(path.c_str(), O_RDONLY, 0777);
            if (new_fd < 0)
            {
                cerr << RED << "openning " << path << RESET << endl;
                data.status = HTTP_INTERNAL_SERVER;
                return new_fd;
            }
            pfds.push_back((pollfd){.fd = new_fd, .events = POLLIN | POLLOUT});
            data.respbuff = generate_header(data, data.ctype, data.st.st_size);
            data.clen = data.st.st_size;
            data.flen = data.clen + data.respbuff.length();
            data.action = READ_;
            read_fd = new_fd;
            write_fd = fd;
            cout << "New file connection has fd " << new_fd << " from client " << fd << endl;
            types[new_fd] = FILE_;
            pairs[new_fd] = fd;
        }
        else if (data.method == POST_)
        {
            new_fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
            cout << "POST to file " << path << " has fd " << new_fd << endl;
            if (new_fd < 0)
            {
                data.status = HTTP_INTERNAL_SERVER;
                return new_fd;
            }
            data.status = HTTP_SUCCESS;
            pfds.push_back((pollfd){.fd = new_fd, .events = POLLIN | POLLOUT});
            if (data.requbuff.length())
                data.action = WRITE_;
            else
                data.action = READ_;
            // data.clen is given in header
            read_fd = fd;
            write_fd = new_fd;
            types[new_fd] = FILE_;
            pairs[new_fd] = fd;
        }
        return new_fd;
    }
    bool file_is_cgi(string &path, Location &location)
    {
        map<string, string>::iterator it = location.cgi.begin();
        while (it != location.cgi.end())
        {
            if (ends_with(path, it->first))
            {
                data.is_cgi = true;
                data.cgi_fname = path;
                data.cgi_exec = it->second;
                return true;
            }
            it++;
        }
        return false;
    }

    int readbuff(int src_fd)
    {
        if (src_fd != read_fd)
            throw Error(__LINE__, "readbuff");
        char buffer[READ_BUFFSIZE];
        ssize_t r = 0;
        // TODO: don't use if its error because you can read from external error pages
        r = read(src_fd, buffer, READ_BUFFSIZE);
        if (data.method == (Method)0)
            data.requbuff.append(buffer, r);
        else if (data.method == GET_)
        {
            if (r > 0)
            {
                if (!data.header_parsed)
                    data.requbuff.append(buffer, r);
                else // read file
                {
                    data.respbuff.append(buffer, r);
                    data.action = WRITE_;
                }
            }
            // else if(r == 0 && src_fd != fd && data.st.st_size == 0) // case reading empty file
            // {
            //     data.action = WRITE_;
            // }
            else if (r == 0)
            {
                cout << "line " << LINE << ": clen: " << data.clen << " flen: " << data.flen << endl;
                cout << "line " << LINE << ": read 0 from " << src_fd << " " << to_string(type) << endl;
                close_connection(LINE, src_fd);
                if (data.is_cgi) // TODO: remove wehn parsing the result
                {
                    close_connection(LINE, write_fd);
                }
            }
            else // close it
            {
                cerr << RED << "read failed in " + con_state(*this) << RESET << endl;
                close_connection(LINE, src_fd);
            }
        }
        else if (data.method == POST_)
        {
            if (r > 0)
            {
                data.update_timeout();
                data.read_size += r;
                data.requbuff.append(buffer, r);
                // if header not parsed keep reading
                if (data.header_parsed)
                    data.action = WRITE_;
            }
            else if (r == 0)
            {
                // response to client
                cerr << "line " << LINE << RED ": read 0 from  " << src_fd << RESET << endl;
                data.finish_connection = true;
                cout << "read 0 from " << src_fd << " " << to_string(type) << endl;
                close_connection(LINE, src_fd);
            }
            else // close it
            {
                cerr << RED << "read failed in " + con_state(*this) << RESET << endl;
                close_connection(LINE, src_fd);
            }
        }
        return r;
    }
    int writebuff(int dest_fd)
    {
        if (dest_fd != write_fd)
            throw Error(__LINE__, "wtf");
        ssize_t w = 0;
        if (data.method == GET_)
        {
            string &buff = data.respbuff;
            w = write(dest_fd, buff.c_str(), buff.length());
            if (w > 0)
            {
                data.update_timeout();
                data.write_size += w;
                cout << "line: " << LINE << " write res: " << w << " clen: " << data.clen
                     << " flen: " << data.flen << " wlen: " << data.write_size << " to fd: " << dest_fd << endl;
                // cout << GREEN"write " << buff <<RESET <<endl;
                // cout << "write: " << w <<" flen: " << data.flen << " clen: " << data.clen << " write_len: " << data.write_size << " bufflen: " << buff.length() << endl;
                buff = substr(LINE, buff, w, buff.length());
                if (data.write_size == data.flen)
                {
                    cout << "GET: write reach end of file" << endl;
                    cout << "remaining body <" << data.requbuff << ">" << endl;
                    // TODO: check if there is a remaining body in GET and parse it, refresh data
                    if (data.is_cgi)
                    {
                        cout << "delete CGI output" << endl;
                        remove(data.cgi_output.c_str());
                    }
                    if (data.requbuff.empty())
                        close_connection(LINE, dest_fd);
                    else
                        cout << RED "GET finished but connection won't close because of ramaining body" RESET << endl;
                }
                else if (buff.empty())
                    data.action = READ_;
            }
            else if (w == 0)
            {
                cerr << RED << "write 0 in " + con_state(*this) << RESET << endl;
                close_connection(LINE, dest_fd);
            }
            else // close it
            {
                cerr << RED << "write failed in " + con_state(*this) << RESET << endl;
                close_connection(LINE, dest_fd);
            }
        }
        else if (data.method == POST_)
        {
            string &buff = !data.finish_connection ? data.requbuff : data.respbuff;
            w = write(dest_fd, buff.c_str(), buff.length());
            if (w > 0)
            {
                cout << "line: " << LINE << " write res: " << w << " buff is: " << buff << " clen: " << data.clen
                     << " flen: " << data.flen << " wlen: " << data.write_size << endl;
                data.update_timeout();
                data.write_size += w;
                // string tmp = buff;
                buff = substr(LINE, buff, w, buff.length());
                // cout << " the written buff " << tmp << endl;
                // I change write_size in generate header
                // keep it clen
                if (data.write_size == data.flen && !data.finish_connection && buff.empty())
                {
                    generate_response(LINE, *this);
                    cout << "line " << LINE << " write_fd " << write_fd << " curr fd " << fd << endl;
                    close_connection(LINE, write_fd);
                    write_fd = fd;
                    cout << "line " << LINE << " write_fd " << write_fd << " curr fd " << fd << endl;
                }
                // don't touch it, test with POST long file, before changing it
                else if (data.flen && data.write_size == data.flen && data.finish_connection
                         // && !data.keep_alive
                )
                {
                    close_connection(LINE, dest_fd);
                    // throw Error("response is sent");
                }
                else if (buff.empty())
                    data.action = READ_;
            }
            else if (w == 0 && data.finish_connection) // used for POST response, don't change it
                close_connection(LINE, dest_fd);
            else // close it
            {
                cerr << RED << "line: " << LINE << " write failed in " + con_state(*this) << RESET << endl;
                close_connection(LINE, dest_fd);
            }
        }
        return w;
    }
    int open_dir(string &path, string &uri)
    {
        if (data.method == GET_)
        {
            cout << "GET dir " << path << endl;
            dirent *entry;
            DIR *dir = opendir(path.c_str());
            if (dir == NULL)
                throw Error("open dir failed");
            data.respbuff = "<ul>\n";
            while ((entry = readdir(dir)) != NULL)
            {
                // TODO: in clear path use current address instead of localhost
                string url = "<a target=\"_blank\" href=\"http://" +
                             clear_path("localhost:" +
                                        to_string(data.port) + "/" + uri + "/" + string(entry->d_name)) +
                             "\">" +
                             string(entry->d_name) + "</a><br>\n";
                data.respbuff += url;
            }
            closedir(dir);
            data.respbuff += "</ul>";
            data.clen = data.respbuff.length();
            data.ctype = "text/html";
            data.action = WRITE_;
            data.respbuff = generate_header(data, data.ctype, data.clen) + data.respbuff;
            data.flen = data.respbuff.length();
            write_fd = fd;
            data.write_size = 0;
        }
        return 1;
    }
    void close_connection(int line, int close_fd)
    {
        cout << "close " << close_fd << " from " << fd << " called from " << line << endl;
        if (close_fd == fd && data.finish_connection)
            cout << GREEN "response is finished" RESET << endl;
        if (close_fd != read_fd && close_fd != write_fd && close_fd != fd)
            throw Error(LINE, "in close connection");
        close(close_fd);
        if (close_fd == fd)
        {
            if (read_fd != fd && read_fd > 0)
                close_connection(LINE, read_fd);
            if (write_fd != fd && write_fd > 0)
                close_connection(LINE, write_fd);
        }
        else if (close_fd == write_fd)
        {
            write_fd = -1;
            data.write_size = 0; // used when listing directory
        }
        else if (close_fd == read_fd)
        {
            read_fd = -1;
            data.read_size = 0; // used when listing directory
        }
        for (vector<pollfd>::iterator it = pfds.begin(); it != pfds.end(); ++it)
        {
            if (it->fd == close_fd)
            {
                pfds.erase(it);
                break;
            }
        }
        for (map<int, int>::iterator it = pairs.begin(); it != pairs.end(); ++it)
        {
            if (it->first == close_fd)
            {
                pairs.erase(it);
                break;
            }
        }
        if (cons.count(close_fd) && close_fd == fd)
            cons.erase(close_fd);
        cout << " pfds size " << pfds.size() << endl;
    }
};

#endif