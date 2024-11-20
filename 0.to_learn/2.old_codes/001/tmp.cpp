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
#define BUFFSIZE 7000
#define END (char *)"\r\n"
#define HEADEREND (char *)"\r\n\r\n"
#define ADDRLEN 4096
#define MACRO_TO_STRING(f) #f
#define LINE __LINE__
#define FILE __FILE__

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

// #define debug cout << GREEN "line " << LINE << " "
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

enum Transfer
{
    CHUNKED_ = 20,
};

enum Status
{
    SUCCESS = 200,
    NO_CONTENT = 204,
    MOVE_LOCATION = 301,
    BAD_REQUEST = 400,
    FORBIDEN = 403,
    NOT_FOUND = 404,
    NOT_ALLOWED = 405,
    TIMEOUT = 408,
    // INSUPPORTED_MEMETYPE = 415,
    INTERNAL = 500,
    NOT_IMPLEMENTED = 501,
};

string to_string(Action action);
string to_string(Method method);
string to_string(Type type);
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
        // TODO: all locations should starts with /
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

    Method method;
    string uri;

    size_t port;
    string host;
    Status status;

    string ctype;
    ssize_t clen; // could be content-Length or chunk-Length

    ssize_t read_size;
    size_t write_size;
    state st;

    string queries;
    bool ready; // ready to response
    bool keep_alive;
    time_t last_activity;

    bool is_cgi;
    string cgi_output;
    string cgi_fname;
    string cgi_exec;
    pid_t   cgi_pid;

    bool close_connection;
    Location *location;

    // TODO: check all constractor, they should set all values with 0
    bool is_error()
    {
        return status >= 400 && status <= 511;
    }
    bool check(int line, bool cond, Status status_)
    {
        if (cond)
            status = status_;
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
    void refresh()
    {
        trans = (Transfer)0;
        method = (Method)0;
        status = (Status)0;
        clen = 0;
        pid = 0;
        is_cgi = false;
        cgi_fname = "";
        action = READ_;
        uri = "";
        host = "";
        ctype = "application/octet-stream";
        queries = "";
        is_cgi = false;
        cgi_fname = "";
        pid = 0;
        close_connection = false;
        ready = false;
        read_size = 0;
        write_size = 0;
        location = NULL;
    }
};

// GLOBALS
extern size_t pos;
extern size_t line;
extern map<string, string> memetype;

extern vector<pollfd> pfds;

extern map<int, Type> fds;
extern map<int, Connection> cons;
extern map<int, int> pairs;

string clear_path(string path);
string con_state(Connection &con);
bool ends_with(string str, string sub);
struct Connection
{
    Type type;
    Data data;
    int index;

    // TODO: set limit for accepted addresses
    char address[ADDRLEN];
    string srv_address;

    int fd;
    int read_fd;
    int write_fd;

    bool file_is_cgi(string &path, Location &location)
    {
        if (location.cgi.size())
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
        }
        return false;
    }

    int readbuff(int src_fd)
    {
        if (src_fd != read_fd)
            throw Error(__LINE__, "wtf");

        char buffer[BUFFSIZE];
        ssize_t r = 0;
        if (data.is_error())
        {
            string &buff = data.respbuff;
            r = read(src_fd, buffer, BUFFSIZE);
            cout << "read from error page " << endl;
            if (r > 0)
            {
                data.update_timeout();
                buff.append(buffer, r);
                data.read_size += r;
                data.action = WRITE_;
            }
            else if (r == 0)
            {
                close_connection(LINE, src_fd);
            }
            else if (r < 0)
            {
                cerr << RED << "read failed in " + con_state(*this) << RESET << endl;
                // throw Error("write failed");
            }
        }
        else if (data.method == (Method)0)
        {
            string &buff = data.requbuff;
            r = read(src_fd, buffer, BUFFSIZE);
            if (r > 0)
            {
                data.update_timeout();
                buff.append(buffer, r);
                data.read_size += r;
                data.action = WRITE_;
            }
            else if (r == 0)
            {
                cerr << "line " << LINE << RED ": read 0 from  " << src_fd << RESET << endl;
                close_connection(LINE, src_fd);
            }
            else if (r < 0)
            {
                cerr << RED << "line: " << LINE << ": read failed " RESET << endl;
                close_connection(LINE, src_fd);
            }
        }
        else if (data.method == GET_)
        {
            string &buff = data.ready ? data.respbuff : data.requbuff;
            r = read(src_fd, buffer, BUFFSIZE);
            if (r > 0)
            {
                data.update_timeout();
                buff.append(buffer, r);
                data.read_size += r;
                data.action = WRITE_;
                if (data.read_size == data.st.st_size)
                {
                    cout << "line " << LINE << GREEN ": GET reached the end of file  " << src_fd << RESET << endl;
                }
            }
            // else if (r == 0)
            // {
            //     cerr << "line " << LINE << RED ": read 0 from  " << src_fd << RESET << endl;
            //     if (src_fd != fd)
            //     {
            //         // close_connection(LINE, src_fd);
            //         // else
            //         close_connection(LINE, src_fd);
            //         // close_connection(LINE, fd);
            //         // read_fd = fd;
            //     }
            // }
            else if (r < 0)
            {
                cerr << RED << "line: " << LINE << ": read failed " << RESET << endl;
                close_connection(LINE, src_fd);
            }
        }
        else if (data.method == POST_)
        {
            string &buff = data.requbuff;
            if (data.trans == CHUNKED_)
                r = read(src_fd, buffer, data.clen);
            else
                r = read(src_fd, buffer, BUFFSIZE);
            if (r > 0)
            {
                buff.append(buffer, r);
                data.read_size += r;
                data.update_timeout();

                data.action = WRITE_;
            }
            else if (r == 0)
            {
                cerr << "line " << LINE << RED ": read 0 from  " << src_fd << RESET << endl;
                close_connection(LINE, src_fd);
            }
            else if (r < 0)
            {
                // TODO: may be you should send internal error
                cerr << RED << "line: " << LINE << ": read failed " << RESET << endl;
                close_connection(LINE, src_fd);
            }
        }
        return r;
    };
    int writebuff(int dest_fd)
    {
        if (dest_fd != write_fd)
            throw Error(__LINE__, "wtf");
        ssize_t r = 0;
        if (data.method == (Method)0)
        {
            throw Error(LINE, "weird error");
        }
        else if (data.is_error())
        {
            string &buff = data.respbuff;
            r = write(dest_fd, buff.c_str(), buff.length());
            if (r > 0)
            {
                data.update_timeout();
                buff = substr(LINE, buff, r, buff.length());
                data.write_size += r;
                if (buff.empty())
                    data.action = READ_;
            }
            if (r == 0 || data.write_size == data.clen)
                close_connection(LINE, dest_fd);
            else if (r < 0)
                cerr << RED << "write failed in " + con_state(*this) << RESET << endl;
        }
        else if (data.method == GET_)
        {
            string &buff = data.ready ? data.respbuff : data.requbuff;
            r = write(dest_fd, buff.c_str(), buff.length());
            if (r > 0)
            {
                data.update_timeout();
                buff = substr(LINE, buff, r, buff.length());
                data.write_size += r;
                if (buff.empty())
                    data.action = READ_;
            }
            if (r < 0)
            {
                cerr << RED << "write failed in " + con_state(*this) << RESET << endl;
                close_connection(LINE, dest_fd);
                // throw Error("write failed");
            }
        }
        else if (data.method == POST_)
        {
            string &buff = (data.ready && data.write_size != data.clen) ? data.requbuff : data.respbuff;
            r = write(dest_fd, buff.c_str(), buff.length());
            cout << "write " << r << " bytes " << endl;
            if (r > 0)
            {
                data.update_timeout();
                buff = substr(LINE, buff, r, buff.length());
                data.write_size += r;
                if (buff.empty() && data.write_size == data.clen)
                {
                    cout << RED "prepare POST response" RESET << endl;
                    // POST response
                    data.action = WRITE_;
                    data.respbuff = "<div style=\"display:flex; justify-content:center;"
                                    "align-items:center; color:blue; font-size: large;\">"
                                    "FILE created succefully"
                                    "</div>";
                    data.respbuff = "HTTP/1.1 200 OK\r\n"
                                    "Content-type: text/html\r\n"
                                    "Content-Length: " +
                                    to_string(data.respbuff.length()) + "\r\n\r\n" + data.respbuff;
                    close_connection(LINE, write_fd);
                    write_fd = fd;
                }
                else if (buff.empty() && data.write_size > data.clen)
                {
                    cout << "POST response is sent" << endl;
                }
                else if (buff.empty())
                    data.action = READ_;
            }
            else if (r == 0)
            {
                cerr << RED "write 0" RESET << endl;
                close_connection(LINE, dest_fd);
            }
        }

        return r;
    };

    int open_file(string &path)
    {
        cout << "open file: " << path << endl;
        int file_fd = -1;
        if (data.is_error())
        {
            file_fd = open(path.c_str(), O_RDONLY, 0777);
            if (file_fd < 0)
            {
                // perror("");
                // throw Error("\nerror page: openning file: " + path);
                cerr << RED << "opening error page" RESET << endl;
                return file_fd;
            }
            cerr << RED "error page has fd: " RESET << file_fd << endl;
            pfds.push_back((pollfd){.fd = file_fd, .events = POLLIN | POLLOUT});
            // TODO: use function to generate response
            data.respbuff = "HTTP/1.1 " + to_string((int)data.status) + " NOK\r\n" +
                            "Content-Type: text/html\r\n"
                            "Content-Length: " +
                            to_string(data.st.st_size) +
                            "\r\n\r\n";
            data.clen = data.st.st_size + data.respbuff.length();
            data.action = READ_;
            read_fd = file_fd;
            write_fd = fd;
            pairs[file_fd] = fd;
            data.read_size = 0;
        }
        else
        {
            if (data.method == GET_)
            {
                cout << "GET file " << path << endl;
                ssize_t dot = path.find_last_of(".");
                string ext;
                if (dot != string::npos && memetype.count((ext = substr(LINE, path, dot, path.length()))))
                    data.ctype = memetype[ext];
                file_fd = open(path.c_str(), O_RDONLY, 0777);
                if (file_fd < 0)
                {
                    // TODO set error stataus here ... and call fetch_cinfig recurcively
                    perror("");
                    throw Error("\nGET: openning file: " + path);
                }
                pfds.push_back((pollfd){.fd = file_fd, .events = POLLIN | POLLOUT});
                // TODO: use function to generate response
                data.respbuff = "HTTP/1.1 " + to_string((int)data.status) + " OK\r\n"
                                                                            "Content-Type: " +
                                data.ctype +
                                "\r\nContent-Length: " + to_string(data.st.st_size) +
                                "\r\n\r\n";
                data.clen = data.st.st_size + data.respbuff.length();

                data.action = READ_;

                cout << "New file connection has fd " << file_fd << " from client " << fd << endl;
                read_fd = file_fd;
                write_fd = fd;

                fds[file_fd] = FILE_;
                pairs[file_fd] = fd;
                data.read_size = 0;
            }
            else if (data.method == POST_)
            {
                cout << "POST to file " << path << endl;
                file_fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
                if (file_fd < 0)
                {
                    data.status = INTERNAL;
                    return file_fd;
                }
                pfds.push_back((pollfd){.fd = file_fd, .events = POLLIN | POLLOUT});
                data.action = WRITE_;
                read_fd = fd;
                write_fd = file_fd;

                fds[file_fd] = FILE_;
                pairs[file_fd] = fd;
                // data.write_size = 0;
            }
        }
        return file_fd;
    };
    int open_dir(string &path, string &uri) // TODO: check if open or opendir failed !!!
    {
        if (data.method == GET_)
        {
            cout << "GET dir " << path << endl;
            dirent *entry;
            DIR *dir = opendir(path.c_str());
            data.respbuff = "<ul>\n";
            while ((entry = readdir(dir)) != NULL)
            {
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
            data.respbuff = "HTTP/1.1 " + to_string((int)data.status) + " OK\r\n"
                                                                        "Content-Type: " +
                            data.ctype +
                            "\r\nContent-Length: " +
                            to_string(data.clen) +
                            "\r\n\r\n" + data.respbuff;
            write_fd = fd;
        }
        return 1;
    }
    int close_connection(int line, int close_fd)
    {
        cout << "close " << close_fd << " from " << fd << " called from " << line << endl;
        if (close_fd != read_fd && close_fd != write_fd && close_fd != fd)
            throw Error(LINE, "wtf");
        close(close_fd);
        if (close_fd == fd)
        {
            if (read_fd != fd && read_fd > 0)
                close_connection(LINE, read_fd);
            if (write_fd != fd && write_fd > 0)
                close_connection(LINE, write_fd);
            read_fd = -1;
            write_fd = -1;
        }
        if (close_fd == read_fd)
            read_fd = -1;
        if (close_fd == write_fd)
            write_fd = -1;
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
        return 1;
    };
};

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


map<string, string> memetype;
map<int, Connection *> tmp_cons;

map<int, int> pairs;
map<int, Connection> cons;
vector<Connection> cons_vec;
vector<pollfd> pfds;
size_t pos = 0;
size_t line = 1;
map<int, Type> fds;

string update_host(string &host)
{
    cout << "call update host" << endl;
    struct hostent *hostInfo = gethostbyname(host.c_str());
    if (hostInfo == NULL)
        throw Error(line, "Unable to resolve hostname");
    struct in_addr **addressList = (struct in_addr **)(hostInfo->h_addr_list);
    if (addressList[0] != NULL)
        return inet_ntoa(*addressList[0]);
    return host;
}

bool parse_header(Connection &con)
{
    // bool res = false;
    ssize_t e, s;
    Data &data = con.data;
    string &buff = data.requbuff;
    ssize_t pos = find(buff, END);
    while (pos != string::npos)
    {
        // remember: I'm cutting the buff inside if statement
        if (!data.method)
        {
            if (compare(buff, "POST "))
            {
                data.method = POST_;
                data.uri = data.cut(strlen((char *)"POST "), e);
                // TODO: to be checked
                if (data.check(LINE, data.uri.empty() || data.uri[0] != '/', BAD_REQUEST))
                    break;

                cout << "POST to uri <" << data.uri << ">" << endl;
                if (data.check(LINE, !compare(e, buff, " HTTP/1.1\r\n"), BAD_REQUEST))
                    break;
            }
            else if (compare(buff, "GET "))
            {
                data.method = GET_;
                data.uri = data.cut(strlen((char *)"GET "), e);
                if (data.check(LINE, data.uri.empty() || data.uri[0] != '/', BAD_REQUEST))
                    break;
                data.uri = parse_hexadecimal(data.uri);
                ssize_t q_pos = find(data.uri, (char *)"?");
                if (q_pos != string::npos) // TODO: add query to POST and DELETE also
                {
                    // found queries
                    cout << "found ?" << endl;
                    s = q_pos;
                    while (!isspace(data.uri[s]) && s < data.uri.length())
                        s++;
                    data.queries = substr(LINE, data.uri, q_pos + 1, s);
                    data.uri = substr(LINE, data.uri, 0, q_pos);
                    cout << "queries <" << data.queries << ">" << endl;
                }
                bool cond = !compare(e, buff, " HTTP/1.1\r\n") && !compare(e, buff, " HTTP/1.0\r\n");
                if (data.check(LINE, cond, BAD_REQUEST))
                    break;
                cout << "GET to uri <" << data.uri << ">" << endl;
            }
            else if (data.check(LINE, true, BAD_REQUEST)) // if no method
                break;
        }
        else if (compare(buff, "Connection: "))
        {
            string con = data.cut(strlen((char *)"Connection: "), e);
            cout << "found connction <" << con << ">" << endl;
            if (con == "keep-alive")
                data.keep_alive = true;
        }
        else if (compare(buff, "Host: "))
        {
            data.host = data.cut(strlen((char *)"Host: "), e);
            // TODO: to be checked
            // s = find(data.host, (char*)":");
            // if (s != string::npos)
            //     data.host = substr(LINE,data.host, 0, s);
            // data.host = update_host(data.host);
            cout << "found Host <" << data.host << ">" << endl;
            if (data.check(LINE, data.host.empty(), BAD_REQUEST))
                break;
        }
        else if (compare(buff, "Content-Type: "))
        {
            // TODO: add some insuported memtype or something ...
            data.ctype = data.cut(strlen((char *)"Content-Type: "), e);
            cout << "found Content-type <" << data.ctype << ">" << endl;
            if (data.check(LINE, data.ctype.empty(), BAD_REQUEST))
                break;
            DEBUG << " <" << data.ctype << ">" << end_;
        }
        else if (compare(buff, "Content-Length: "))
        {
            string val = data.cut(strlen((char *)"Content-Length: "), e);
            cout << "found Content-Length, val <" << val << "> check " << (val.empty() || !_isdigits(val)) << endl;
            if (data.check(LINE, val.empty() || !_isdigits(val), BAD_REQUEST))
                break;
            // TODO: check if content=len is valid, should be all digits and postive
            data.clen = atol(val.c_str());
        }
        else if (compare(buff, "Transfer-Encoding: "))
        {
            string trans = data.cut(strlen((char *)"Transfer-Encoding: "), e);
            cout << "found Transfer-Encoding, trans <" << trans << ">" << endl;
            if (data.check(LINE, trans.empty(), BAD_REQUEST))
                break;
            if (trans == "chunked")
            {
                data.trans = CHUNKED_;
                cout << "Transfer-Encoding: chunked " << endl;
            }
            else
            {
                // TODO: bad request maybe !!
                cout << "Transfer-Encoding: Unknown " << endl;
            }
        }
        else if (compare(buff, END))
        {
            if (data.trans == CHUNKED_)
                data.clen = 0;
            else
                buff = substr(LINE, buff, strlen(END), buff.length());
            data.ready = true;
            break;
        }
        buff = substr(LINE, buff, pos + strlen(END), buff.length());
        pos = find(buff, END);
    }
    return data.ready;
}

state get_state(string &path)
{
    state state_;
    stat(path.c_str(), &state_);
    return state_;
}

// TODO: check if location has errors pages, use them
void generate_response(Connection &con)
{
    Data &data = con.data;

    // TODO: check all status and set here the values
    if (data.is_error()) // to be verified
    {
        data.action = WRITE_;
        con.write_fd = con.fd;

        cout << LINE << " is error status: " << (int)data.status << endl;
        plocation(*data.location, 15);
        Status status = data.status;
        if (data.location && data.location->errors.count((int)data.status))
        {
            cout << LINE << " open error page: " << data.location->errors[data.status] << endl;
            con.data.st = get_state(data.location->errors[data.status]);
            if (con.open_file(data.location->errors[data.status]) > 0)
                return;
        }
        cout << LINE << " generate error page: " << endl;
        string header = "HTTP/1.1 " + to_string(data.status) + " NOK\r\n" +
                        "Content-Type: text/html\r\n";
        switch (status)
        {
        case BAD_REQUEST:
            data.respbuff = "BAD REQUEST";
            break;
        case FORBIDEN:
            data.respbuff = "FORBIDEN";
            break;
        case NOT_FOUND:
            data.respbuff = "NOT FOUND";
            break;
        case NOT_ALLOWED:
            data.respbuff = "METHOD NOT ALLOWED";
            break;
        case TIMEOUT:
            data.respbuff = "TIMEOUT";
            break;
        case INTERNAL:
            data.respbuff = "INTERNAL ERROR";
            break;
        case NOT_IMPLEMENTED:
            data.respbuff = "NOT IMPLEMENTED";
            break;
        default:
            throw Error(LINE, "handle this error " + to_string(data.status));
            break;
        }
        data.respbuff = "<div style=\"display:flex; justify-content:center;"
                        "align-items:center; color:blue; font-size: large;\">" +
                        data.respbuff + "</div>";

        data.respbuff = header + "Content-Length: " + to_string(data.respbuff.length()) +
                        "\r\n\r\n" + data.respbuff;
    }
}

Status set_status(Location &location, string &path, state &state_, Method method)
{
    cout << "set status from " << path << endl;
    Status status = NOT_FOUND;
    if (!location.methods[method])
        return NOT_ALLOWED;
    if (stat(path.c_str(), &state_) == 0)
    {
        if (!(state_.st_mode & IS_ACCESS))
            status = FORBIDEN;
        else if (state_.st_mode & IS_FILE)
        {
            status = SUCCESS;
            if (access(path.c_str(), method == POST_ ? W_OK : R_OK))
                status = FORBIDEN;
        }
        else if (state_.st_mode & IS_DIR && path[path.length() - 1] == '/')
            status = SUCCESS;
        else if (state_.st_mode & IS_DIR)
            status = MOVE_LOCATION;
    }
    return status;
}

void handle_cgi(Connection &con, string path)
{
    con.data.cgi_output = rand_name();
    cout << "cgi has queries " << con.data.queries << endl;
    // throw Error("debuging");
    con.data.pid = fork();
    if (con.data.pid == 0)
    {
        int status = 0;
        if (con.data.pid == 0)
        {
            int fd = open(con.data.cgi_output.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
            if (fd < 0)
                exit(1);
            std::vector<std::string> env;
            env.push_back("QUERY_STRING=" + con.data.queries);

            env.push_back("PATH_INFO=" + con.data.uri);
            if (con.data.method == POST_)
            {
                // env.push_back("CONTENT_LENGTH=" + to_string(request.body_bytes_read));
                // if (!request.content_type.empty())
                //     env.push_back("CONTENT_TYPE=" + request.content_type);
            }
            env.push_back("REQUEST_METHOD=GET");
            env.push_back("GATEWAY_INTERFACE=CGI/1.1");
            env.push_back("REMOTE_ADDR=" + string(con.address));
            env.push_back("REMOTE_HOST=");
            // env.push_back("SCRIPT_NAME=" + request.parsed_uri.path_encoded);

            // env.push_back("SERVER_NAME=" + client.interface + ":" + to_string(client.server_port));
            // TODO: fix it
            env.push_back("SERVER_PORT=" + to_string(con.data.port));

            env.push_back("SERVER_PROTOCOL=HTTP/1.1");
            env.push_back("SERVER_SOFTWARE=");
            env.push_back("SCRIPT_FILENAME=" + con.data.uri);
            env.push_back("REDIRECT_STATUS=CGI");

            // TODO: bonus
            // if (!request.cookie.empty())
            // env.push_back("HTTP_COOKIE=" + request.cookie);
            std::vector<char *> envp(env.size() + 1);
            for (size_t i = 0; i < env.size(); i++)
                envp[i] = (char *)env[i].c_str();
            envp[env.size()] = 0;

            cout << "has exec path " << con.data.cgi_exec << endl;
            cout << "execute       " << path << endl;
            cout << "cgi filename " << con.data.cgi_fname << endl;
            cout << "env: " << endl;
            for (size_t i = 0; i < envp.size(); i++)
                cout << envp[i] << endl;

            // child process
            if (dup2(fd, 1) > 0)
            {
                char *arg[3];
                arg[0] = (char *)con.data.cgi_exec.c_str();
                arg[1] = (char *)path.c_str();
                arg[2] = NULL;
                execve(con.data.cgi_exec.c_str(), arg, envp.data());
            }
            close(fd);
            exit(-1);
        }
        else if (con.data.pid < 0)
        {
            con.data.status = INTERNAL;
            return generate_response(con);
        }
    }
}

void fetch_config(Connection &con, vector<Server> &srvs)
{
    Data &data = con.data;
    Method &method = data.method;
    string &uri = data.uri;

    string match;
    Location *match_location = NULL;

    vector<Location *> locations;
    bool second = false;

    // state state_;
    size_t i = 0;
    while (i < srvs.size())
    {
        Server &srv = srvs[i];
        if ((srv.name == data.host || second) && srv.port == data.port)
        {
            cout << "fetch: " << uri << " from " << srv.name << endl;
            map<string, Location>::iterator it;
            for (it = srv.location.begin(); it != srv.location.end(); it++)
            {
                string loc_key = it->first;
                if (starts_with(uri, loc_key) && loc_key.length() > match.length())
                {
                    match = loc_key;
                    match_location = &it->second;
                }
            }
            break;
        }
        i++;
        if (second == false && i == srvs.size() && match_location == NULL)
        {
            second = true;
            i = 0;
        }
    }
    Status status;
    bool is_dir;
#if 1
    if (match_location)
    {
        Location &loc = *match_location;
        cout << to_string(method) << ": location key <" << loc.key;
        string match_uri = substr(LINE, uri, loc.key.length(), uri.length());
        string path;
        if (method == GET_)
        {
            cout << "> has <" << loc.src << "> look for " << match_uri << endl;
            path = clear_path(loc.src + "/" + match_uri);
            cout << "GET: " << path << endl;
            data.location = &loc;
            data.status = set_status(loc, path, data.st, method); // TODO: handle dir redirection
            if (data.st.st_mode & IS_DIR && data.status == SUCCESS)
            {
                cout << "found dir " << path << endl;
                if (loc.autoindex && data.st.st_mode & IS_DIR && data.status == SUCCESS)
                {
                    string index_path = clear_path(path + "/" + (loc.index.length() ? loc.index : "index.html"));
                    state tmp_state;
                    Status status = set_status(loc, index_path, tmp_state, GET_);
                    if (status == SUCCESS)
                    {
                        data.status = SUCCESS;
                        path = index_path;
                        con.data.st = tmp_state;
                    }
                    else if (con.open_dir(path, uri) > 0)
                    {
                        return;
                    }
                }
                else if (con.open_dir(path, uri) > 0)
                {
                    return;
                }
            }
            if (data.st.st_mode & IS_FILE && data.status == SUCCESS)
            {
                cout << "found file " << path << endl;
                data.clen = data.st.st_size;
                if (data.clen == 0)
                {
                    data.status = NOT_FOUND;
                    generate_response(con);
                    return;
                }
                else if (con.file_is_cgi(path, loc))
                {
                    handle_cgi(con, path);
                    return;
                }
                else if (con.open_file(path) > 0)
                {
                    return;
                }
            }
        }
        else if (method == POST_)
        {
            cout << "> has <" << loc.dest << "> look for " << match_uri << endl;
            string ext;
            if (memetype.count(data.ctype))
                ext = memetype[data.ctype];
            path = clear_path(loc.dest + "/" + match_uri + "/" + rand_name() + ext);
            cout << "POST: " << path << endl;
            con.data.location = &loc;
            if (con.open_file(path) > 0)
            {
                return;
            }
        }
        else if (method == DELETE_)
        {
        }
    }
#endif

    cout << RED "Not found" RESET << endl;

    generate_response(con);
    // if (method == GET_) // check if is error
    // {
    //     // becarefull clen shoudld not include header len
    //     // TODO: set HTTP host in response
    //     string response = generate_error(con);
    //     data.respbuff = response;
    //     data.clen = response.length();

    //     con.data.action = WRITE_;
    //     data.ready = true;
    //     con.read_fd = con.fd;
    //     con.write_fd = con.fd;
    //     pairs[con.fd] = con.fd;
    // }
}

Connection new_connection(int fd_, Type type_, size_t port_, string &srv_address)
{
    Connection con = {};
    con.data = (Data){};
    con.data.action = READ_;
    //  =
    // con.http(READ_);
    con.data.refresh();
    memset(con.address, 0, sizeof(con.address));
    con.type = type_;
    con.data.port = port_;
    if (con.type == SERVER_)
    {
        con.fd = fd_;
        pfds.push_back((pollfd){.fd = con.fd = fd_, .events = POLLIN | POLLOUT});
        cout << "New server connection has fd " << con.fd << " " << endl;
        con.read_fd = con.fd;
        pairs[con.fd] = con.fd;
    }
    else if (con.type == CLIENT_)
    {
        sockaddr_storage addr;
        socklen_t len = sizeof(addr);
        con.fd = accept(fd_, (sockaddr *)&addr, &len);
        con.srv_address = srv_address;
        if (con.fd < 0)
        {
            throw Error("accept");
            if (pfds.size() == 1) // TODO: if there is no connected client, delete this
                throw Error("accept");
            else
                cerr << RED << "Failed to accept neq connection";
        }
        else if (con.fd > 0)
        {
            inet_ntop(AF_INET, &(((struct sockaddr_in *)&addr)->sin_addr), con.address, INET_ADDRSTRLEN);
            pfds.push_back((pollfd){.fd = con.fd, .events = POLLIN | POLLOUT});
            int flags = fcntl(con.fd, F_GETFL, 0);
            if (flags == -1)
                throw Error(string("fcntl"));
            flags |= O_NONBLOCK;
            if (fcntl(con.fd, F_SETFL, flags) < 0)
                throw Error(string("fcntl"));
            cout << "New client connection has fd " << con.fd << " has addr " << con.address << endl;
            con.read_fd = con.fd;
            con.data.action = READ_;
            pairs[con.fd] = con.fd;
        }
    }
    con.data.update_timeout();
    return con;
}

void Webserve(vector<Server> &conf_servs)
{
    // SIGNALS
    struct sigaction sa = {};
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);

    // CREATE SERVERS SOCKETS
    size_t i = 0;
    size_t servs_size = 0;
    while (i < conf_servs.size())
    {
        // Create a new Connection object
        Connection con = new_connection(init_server_socket(conf_servs[i].port),
                                        SERVER_, conf_servs[i].port, conf_servs[i].listen);

        // Check if the file descriptor is valid
        if (con.fd > 0)
        {
            cons_vec.push_back(con);
            cons[con.fd] = cons_vec[cons_vec.size() - 1];
            servs_size++;
        }
        i++;
    }
    cout << "servs size " << cons_vec.size() << endl;

    // WEBSERVING
    ssize_t timing = -1;
    cout << "Start webserve" << endl;
    while (1)
    {
        timing = (timing + 1) % 1000000;
        if (!timing)
            cout << "pfds size " << pfds.size() << endl;
        int ready = poll(pfds.data(), pfds.size(), -1);
        if (ready < 0)
            continue;
        else if (ready > 0)
        {
            ssize_t i = -1;
            ssize_t s, e, r;
            while (++i < pfds.size())
            {
                if (!pairs.count(pfds[i].fd) || pairs.count(pfds[i].fd) > 1)
                    throw Error("pair not found");
                Connection &con = cons[pairs[pfds[i].fd]];
                if (pfds[i].revents & POLLERR)
                {
                    cout << RED "POLLERR in " << pfds[i].fd << RESET << endl;
                    con.close_connection(LINE, con.fd);
                    continue;
                }
                if (i < servs_size && pfds[i].revents & POLLIN && con.type == SERVER_)
                {
                    cout << "receive event in " << pfds[i].fd << endl;
                    Connection client = new_connection(pfds[i].fd, CLIENT_, con.data.port, con.srv_address);
                    cons_vec.push_back(client);
                    cons[client.fd] = cons_vec[cons_vec.size() - 1];
                }
                else if (i >= servs_size)
                {
                    if (con.data.is_cgi)
                    {
                        // throw Error("is cgi");
                        int status = 1;
                        int val = waitpid(con.data.pid, &status, WNOHANG);
                        if (val < 0 || status < 0)
                        {
                            cout << "did exit with " << status << endl;
                            // throw Error("debuging");
                            con.data.status = INTERNAL;
                            fetch_config(con, conf_servs);
                        }
                        else if (val > 0)
                        {
                            state state_;
                            int fd = open(con.data.cgi_fname.c_str(), O_RDONLY, 0777);
                            if (fd < 0)
                                continue;
                            pfds.push_back((pollfd){.fd = fd, .events = POLLIN | POLLOUT});
                            con.read_fd = fd;
                            stat(con.data.cgi_fname.c_str(), &state_);

                            // TODO: check if it contains header if not add it
                            cout << "state.size: " << state_.st_size << endl;
#if 1
                            string header;
                            header = string("HTTP/1.1 200 OK\r\n") +
                                     "Content-Type: text/html\r\n" +
                                     "Content-Length: " + to_string(state_.st_size) + "\r\n\r\n";
                            con.data.clen = state_.st_size + header.length();
                            con.data.respbuff = header;
#endif
                            cout << GREEN "CGI: Success" RESET << endl;
                            con.data.status = SUCCESS;
                            // sock->action = WRITE_;
                            con.data.is_cgi = false;
                            con.data.update_timeout();
                        }
                    }
                    else if (pfds[i].revents & POLLIN && con.data.action == READ_ && pfds[i].fd == con.read_fd)
                    {
                        // cout << "POLLIN" << endl;
                        r = con.readbuff(pfds[i].fd);
                        if (r > 0 && !con.data.ready && parse_header(con))
                        {
                            fetch_config(con, conf_servs);
                            cout << "after fetching config " << con_state(con) << endl;
                        }
                        if (r < 0)
                        {
                        }
                    }
                    else if (pfds[i].revents & POLLOUT && con.data.action == WRITE_ && pfds[i].fd == con.write_fd)
                    {
                        // cout << "POLLOUT" << endl;
                        r = con.writebuff(pfds[i].fd);
                    }
                    else if (con.type != SERVER_ && con.data.timeout())
                    {
                        con.close_connection(LINE, con.fd);
                    }
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    vector<Server> conf_servs;
    try
    {
        if (argc != 2)
            throw Error(string("Invalid number of arguments"));
        parse_config(conf_servs, argv[1]);
        // cout << "\n\n";
        // for (size_t i = 0; i < conf_servs.size(); i++)
        //     pserver(conf_servs[i]);
        for (size_t i = 0; exts_struct[i].type.length(); ++i)
        {
            memetype[exts_struct[i].type] = exts_struct[i].ext;
            memetype[exts_struct[i].ext] = exts_struct[i].type;
        }
        Webserve(conf_servs);
    }
    catch (exception &err)
    {
        cerr << RED "Error: " << err.what() << RESET << endl;
    }
}