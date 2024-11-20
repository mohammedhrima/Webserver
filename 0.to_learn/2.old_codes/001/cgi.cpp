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
// TODO: set a max events
typedef struct addrinfo addrinfo;
typedef struct sockaddr_in sockaddr_in;
typedef struct pollfd pollfd;
typedef struct sockaddr sockaddr;
typedef struct sockaddr_storage sockaddr_storage;
typedef struct Server Server;
typedef struct Socket Socket;
typedef struct HTTP HTTP;
typedef struct timeval timeval;
typedef struct stat state;
typedef struct dirent dirent;
using namespace std;
#define map_macro map<string, Server *>
typedef map<string, map_macro> ServerChild;

#define CLIENT_TIMEOUT 10

#define BAD_REQUEST 400
#define FORBIDEN 403
#define NOT_FOUND 404
#define NOT_ALLOWED 405
#define INTERNAL 500
#define INSUPPORTED_MEMETYPE 415
#define TIMEOUT 408
#define SUCCESS 200

#define POLLTIMEOUT 10
#define LISTEN_LEN SOMAXCONN
#define BUFFSIZE 7000
// TODO: set a max addres len
#define ADDRLEN 4096

// #define CHUNK_SIZE 8192

#define END (char *)"\r\n"
#define HEADEREND (char *)"\r\n\r\n"

#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define RESET "\033[0m"

// stupid c++ compiler

// string to_string(size_t size);
string substr(size_t line, string &str, ssize_t s, ssize_t e);
bool compare(ssize_t start, string left, string right);

#define substr_(str, s, e) substr(__LINE__, str, s, e)
#define debug cout << GREEN "line " << __LINE__ << " " 
#define end_ RESET << endl

// GLOBALS
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

string text;
size_t pos;
vector<string> arr;
map<string, string> memetype;
map<int, Socket *> socks;
vector<pollfd> pfds;
extern char **environ;

// STRUCTS, ENUMS
enum Action
{
    READ_ = 11,
    WRITE_,
};

enum Method
{
    GET_ = 22,
    POST_,
    DELETE_
};

enum Type
{
    SERVER_ = 33,
    CLIENT_,
    FILE_
};
string to_string(Type type);
string to_string(Action action);
string to_string(Method method);

enum Transfer
{
    CHUNKED_ = 44,
    // BOUNDARY_,
};

class Error : public exception
{
private:
    string message;

public:
    Error(string msg) throw() : message(msg) {}
    Error(size_t line, string msg) throw()
    {
        message = "line " + to_string(line) + " " + msg;
    }
    virtual ~Error() throw() {}
    const char *what() const throw() { return message.c_str(); }
};

struct Server
{
    map<string, string> Data;
    ssize_t port;
    map<long, string> errors;
    ssize_t limit;
    // name -> location
    ServerChild children;

    // constractor
    Server()
    {
        cout << "call constractor" << endl;
        port = -1;
        limit = -1;
        Data["name"] = Data["listen"] = Data["location"] = "";
        Data["index"] = Data["upload"] = Data["root"] = "";
        Data["autoindex"] = "";
        Data["exec"] = Data["ext"] = Data["cgi"] = "";
        Data["GET"] = Data["POST"] = Data["DELETE"] = "off";
    }
    void update_host()
    {
        struct hostent *hostInfo = gethostbyname(Data["listen"].c_str());
        if (hostInfo == NULL)
            throw Error("Unable to resolve hostname");
        struct in_addr **addressList = (struct in_addr **)(hostInfo->h_addr_list);
        if (addressList[0] != NULL)
        {
            std::cout << "IP Address of " << Data["listen"] << ": " << inet_ntoa(*addressList[0]) << std::endl;
            Data["listen"] = inet_ntoa(*addressList[0]);
        }
        else
            throw Error("Unable to retrieve IP address.");
    }
    // operator
    string &operator[](const string &key)
    {
        try
        {
            return Data.at(key);
        }
        catch (...)
        {
            cerr << RED << "error accessing " << key << RESET << endl;
            throw;
        }
    }
    string &operator[](const Method &key)
    {
        switch (key)
        {
        case GET_:
            return Data["GET"];
        case POST_:
            return Data["POST"];
        case DELETE_:
            return Data["DELETE"];
        default:
            throw Error("finding method");
        }
    }
    // methods
    int get_fd()
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0)
            throw Error(string("socket: \n"));
        int option = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)))
            throw Error(string("setsockopt: \n"));

        sockaddr_in addr = (sockaddr_in){0};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port);

        if (bind(fd, (sockaddr *)&addr, sizeof(addr)))
            throw Error(string("bind: \n"));
        if (listen(fd, LISTEN_LEN))
            throw Error(string("listen: \n"));
        return fd;
    }
    // destractor
    ~Server()
    {
        cout << "call destractor" << endl;
        ServerChild::iterator it0;
        for (it0 = children.begin(); it0 != children.end(); ++it0)
        {
            map<string, Server *>::iterator it1;
            for (it1 = it0->second.begin(); it1 != it0->second.end(); ++it1)
                delete it1->second;
        }
        Data.clear();
        errors.clear();
        children.clear();
    }
};

struct HTTP
{
    Transfer trans;
    string buff;
    Method method;
    string uri;

    // string resp;
    string host;
    int status;

    string ctype;
    ssize_t clen; // could be content-Length or chunk-Length
    // ssize_t flen;
    // map<string, string> params;
    string queries;
    bool ready;
    bool keep_alive;

    bool is_cgi;
    string cgi_fname;
    pid_t pid;

    // TODO: check all constractor, they should set all values with 0
    HTTP()
    {
        trans = (Transfer)0;
        method = (Method)0;
        status = 0;
        clen = 0;
        ready = false;
        keep_alive = false;
        pid = 0;
        is_cgi = false;
    }
    bool is_error()
    {
        return status >= 400 && status <= 511;
    }
    bool check(int line, bool cond, int status_) // TODO: check every place where is called if is valid
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
        while (e < buff.length() && !isspace(buff[e]))
            e++;
        if (s == e)
            return "";
        return substr_(buff, s, e);
    }
    void refresh() // TOOD: use it to clean the request after finishing response
    {
        trans = (Transfer)0;
        method = (Method)0;
        status = 0;
        clen = 0;
        // params.clear();
        ready = false;
        // keep_alive = false;
        pid = 0;
        is_cgi = false;
        cgi_fname = "";
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

/*
TODOS:
    + remove all throw except from parsing
    + use limit body-size
*/
struct Socket
{
    // TODO: use an enum to define wheter is parsing header, or something else...
    Server *srv;
    Action action;
    Type type;
    HTTP http;
    // TODO: set limit for accepted addresses
    char address[ADDRLEN];

    time_t timeout;
    Socket *pair;

    int fd;
    Socket(int fd_, Server *srv_, Type type_, Socket *pair_) : http()
    {
        // is_cgi = false;
        fd = fd_, srv = srv_, type = type_, pair = pair_, action = READ_;
        timeout = 0;
        if (type == SERVER_)
        {
            fd = socket(AF_INET, SOCK_STREAM, 0);
            if (fd < 0)
                throw Error(string("socket: "));
            int option = 1;
            if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)))
                throw Error(string("setsockopt: "));

            sockaddr_in addr = (sockaddr_in){0};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
            addr.sin_port = htons(srv->port);

            if (bind(fd, (sockaddr *)&addr, sizeof(addr)))
                throw Error(string("bind: "));
            if (listen(fd, LISTEN_LEN))
                throw Error(string("listen: "));
#if 0
            addrinfo hints = {0};
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE;

            // TODO: it may segvault in srv if is NULL
            addrinfo *addr_info;
            if (getaddrinfo(address, to_string(srv->port).c_str(), &hints, &addr_info) != 0)
                throw Error("getaddrinfo");
            for (; addr_info; addr_info = addr_info->ai_next)
            {
                inet_ntop(AF_INET, addr_info->ai_addr, address, addr_info->ai_addrlen);
            }
#endif
        }
        else if (type == FILE_ || type == CLIENT_)
        {
            if (fd > 0)
            {
                if (socks[fd])
                    throw Error("weird error 1"); // TODO: to be removed
                pfds.push_back((pollfd){.fd = fd, .events = POLLIN | POLLOUT});
                if (type == CLIENT_)
                    set_non_blocking();
            }
            if (pair)
                pair->pair = this;
        }
        socks[fd] = this;
        update_timeout();
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

    void update_timeout()
    {
        timeout = time(NULL);
        if (pair)
            pair->timeout = timeout;
    }
    // destractor
    ~Socket()
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
        if (pair)
        {
            pair->pair = NULL;
            pair->http.refresh();
        }
        socks[fd] = NULL;
        // cout << "pfds size " << pfds.size() << endl;
    }
};

// DEBUGING
void pserver(Server *srv, int space)
{
    if (srv == NULL)
        throw Error(string("srv in NULL\n"));

    map<string, string>::iterator it1;
    for (it1 = srv->Data.begin(); it1 != srv->Data.end(); it1++)
    {
        if (it1->second.length())
            cout << setw(space) << it1->first << " : <" << it1->second << ">" << endl;
    }
    if (srv->errors.size())
    {
        cout << setw(space + 3) << "ERRORS : " << endl;
        map<long, string>::iterator it2;
        for (it2 = srv->errors.begin(); it2 != srv->errors.end(); it2++)
        {
            if (it2->second.length())
                cout << setw(space + 6) << it2->first << " : <" << it2->second << ">" << endl;
        }
    }
    cout << setw(space + 3) << "PORT : " << srv->port << "\n\n";
    // name -> location
    ServerChild::iterator it3;
    for (it3 = srv->children.begin(); it3 != srv->children.end(); it3++)
    {
        map<string, Server *>::iterator it4;
        for (it4 = it3->second.begin(); it4 != it3->second.end(); it4++)
            pserver(it4->second, space + 4);
    }
}

// string to_string(size_t size)
// {
//     stringstream stream;
//     stream << size;
//     return stream.str();
// }

string to_string(Action action)
{
    switch (action)
    {
    case READ_:
        return "READ";
        break;
    case WRITE_:
        return "WRITE";
        break;
    default:
        return "UNKNOWN (Action)" + to_string((int)action);
    }
    // return "UNKNOWN";
}

string to_string(Method method)
{
    switch (method)
    {
    case GET_:
        return "GET";
        break;
    case POST_:
        return "POST";
        break;
    case DELETE_:
        return "DELETE";
        break;
    default:
        return "UNKNOWN (Method)" + to_string((int)method);
        break;
    }
    // return "UNKNOWN";
}

string to_string(Type type)
{
    switch (type)
    {
    case CLIENT_:
        return "CLIENT";
        break;
    case SERVER_:
        return "SERVER";
        break;
    default:
        return "UNKNOWN (Type)" + to_string((int)type);
        break;
    }
    // return "UNKNOWN";
}


string sock_state(Socket *sock)
{
    return "type: " + to_string(sock->type) +
           " action: " + to_string(sock->action) +
           " method: " + to_string(sock->http.method) +
           " buff: <" + sock->http.buff + ">";
}

// PARSING
bool _isspace(char c)
{
    return (c == ' ');
}

bool _isdigits(string &str)
{
    size_t i = 0;
    while (isdigit(str[i]) && i < str.length())
        i++;
    return str.length() && str[i] == '\0';
}

bool starts_with(string str, string sub)
{
    return str.length() >= sub.length() && str.compare(0, sub.length(), sub) == 0;
}

bool ends_with(string str, string sub)
{
    return str.length() >= sub.length() && str.compare(str.length() - sub.length(), sub.length(), sub) == 0;
}

bool compare(string left, string right)
{
    return left.compare(0, right.length(), right.c_str(), 0, right.length()) == 0;
}

bool compare(ssize_t start, string left, string right)
{
    return left.compare(start, right.length(), right.c_str(), 0, right.length()) == 0;
}

string substr(size_t line, string &str, ssize_t s, ssize_t e)
{
    try
    {
        return str.substr(s, e - s);
    }
    catch (...)
    {
        throw Error("Failed to substract <" + str + "> from " +
                    to_string(s) + " to " + to_string(e) + " line " + to_string(line));
    }
}

ssize_t find(string &str, char *to_find)
{
    return str.find(to_find, 0, strlen(to_find));
}

string rand_name()
{
    return "file";
    timeval current_time;
    gettimeofday(&current_time, NULL);
    localtime(&current_time.tv_sec);
    stringstream filename;
    filename << current_time.tv_sec;
    return filename.str();
}

vector<string> split_string()
{
    text.erase(remove_if(text.begin(), text.end(), _isspace), text.end());
    size_t i = 0;
    while ((i = text.find('#', i)) != string::npos)
    {
        size_t j = text.find("\n", i);
        if (j == string::npos)
            j = text.find("\0", i);
        text = text.substr(0, i) + text.substr(j + 1, text.length());
    }
    cout << "text: " << text << endl;
    vector<string> result;
    stringstream ss(text);
    string token;
    while (getline(ss, token, '\n'))
    {
        size_t pos = 0;
        while ((pos = token.find_first_of(";{}:,\n", pos)) != string::npos)
        {
            if (pos)
                result.push_back(token.substr(0, pos));
            result.push_back(string(1, token[pos]));
            token.erase(0, pos + 1);
            pos = 0;
        }
        if (!token.empty())
            result.push_back(token);
        result.push_back("\n");
    }
    return result;
}

string clear_path(string path)
{
    string res;
    size_t i = 0;
    while (i < path.length())
    {
        res += path[i];
        if (path[i] == '/')
        {
            while (path[i] == '/')
                i++;
        }
        else
            i++;
    }
    return res;
}

size_t line = 1;
Server *parse()
{
    Server *serv_ptr = new Server();
    Server &srv = *serv_ptr;
    try
    {
        if (arr[pos++] != "{")
            throw Error("Expected '{' got <" + arr[pos - 1] + "> line " + to_string(line));
        while (pos < arr.size() && arr[pos] != "}")
        {
            string Props[] = {"name", "root", "upload", "index", "limit", "autoindex", "listen", "location", "exec", "ext", "cgi"};
            bool found = false;
            for (size_t i = 0; i < sizeof(Props) / sizeof(Props[0]); i++)
            {
                if (arr[pos] == Props[i])
                {
                    found = true;
                    pos++;
                    if (arr[pos++] != ":")
                        throw Error("Expected ':' got <" + arr[pos - 1] + "> line " + to_string(line));
                    if (srv[Props[i]].length())
                        throw Error("Repeated key " + Props[i] + " line " + to_string(line));
                    else if (!strchr(";{}:,", arr[pos][0]))
                        srv[Props[i]] = arr[pos++];
                    else
                        throw Error("Invalid value line " + to_string(line));
                    if (Props[i] == "listen" && arr[pos] == ":")
                    {
                        pos++;
                        if (_isdigits(arr[pos]))
                            srv.port = atol(arr[pos++].c_str()); // TODO: check valid value for port
                        else
                            throw Error("Invalid port line " + to_string(line));
                        srv.update_host();
                    }
                    else if ((Props[i] == "autoindex" || Props[i] == "cgi") && srv[Props[i]] != "on" && srv[Props[i]] != "off")
                        throw Error("Invalid " + Props[i] + " value line " + to_string(line));
                    else if (Props[i] == "root")
                        srv["root"] = clear_path(srv["root"]);
                    else if (Props[i] == "location")
                    {
                        string value = srv["location"];
                        if (srv["location"][0] != '/')
                            throw Error("location should starts with / <" + value + "> line " + to_string(line));
                        if (srv["location"].find('/', 1) != string::npos)
                        {
                            delete serv_ptr;
                            throw Error("location should contains only one /, at the beginning <" + value + "> line " + to_string(line));
                        }
                        srv["location"] = clear_path(srv["location"]);
                    }
                    if (arr[pos++] != ";")
                        throw Error("Expected ';' got <" + arr[pos - 1] + "> line " + to_string(line));
                    break;
                }
            }
            if (found)
                continue;
            else if (arr[pos] == "methods")
            {
                pos++;
                if (arr[pos++] != ":")
                    throw Error("Expected ':' got <" + arr[pos - 1] + "> line " + to_string(line));
                while (arr[pos] == "GET" || arr[pos] == "POST" || arr[pos] == "DELETE" || arr[pos] == ",")
                {
                    if (arr[pos] != ",")
                        srv[arr[pos]] = "on";
                    pos++;
                }
                if (arr[pos++] != ";")
                    throw Error("Expected ';' got <" + arr[pos - 1] + "> line " + to_string(line));
                continue;
            }
            else if (arr[pos] == "errors")
            {
                pos++;
                while (arr[pos] == "\n")
                {
                    line++;
                    pos++;
                }
                if (arr[pos++] != "{")
                    throw Error("Expected '{' got <" + arr[pos - 1] + "> line " + to_string(line));
                while (arr[pos] != "}" && pos < arr.size())
                {
                    if (_isdigits(arr[pos])) // TODO: check 400 < error status < 500
                    {
                        long status = atol(arr[pos++].c_str());
                        if (arr[pos++] != ":")
                            throw Error("Expected ':' got <" + arr[pos - 1] + "> line " + to_string(line));
                        if (!ends_with(arr[pos], ".html"))
                            throw Error("Invalid error page, got <" + arr[pos] + "> line " + to_string(line));
                        srv.errors[status] = arr[pos++];
                        if (arr[pos++] != ";")
                            throw Error("Expected ';' got <" + arr[pos - 1] + "> line " + to_string(line));
                    }
                    else if (arr[pos] == "\n")
                    {
                        line++;
                        pos++;
                    }
                    else
                        throw Error("Unexpected <" + arr[pos] + "> line " + to_string(line));
                }
                if (arr[pos++] != "}")
                    throw Error("Expected '}' got <" + arr[pos - 1] + "> line " + to_string(line));
                continue;
            }
            else if (arr[pos] == "{")
            {
                Server *child = parse();
                string location = (*child)["location"];
                string name = (*child)["name"];
                // cout << "location: " << location << " name: " << name << endl;
                if (srv.children[location][name])
                {
                    delete child;
                    throw Error("Repeated location with same name");
                }
                srv.children[location][name] = child;
                continue;
            }
            else if (arr[pos] == "\n" || arr[pos] == ";")
            {
                if (arr[pos] == "\n")
                    line++;
                pos++;
                continue;
            }
            else if (arr[pos] == "#")
            {
                pos++;
                while (arr[pos] != "\n" && pos < arr.size())
                    pos++;
                continue;
            }
            else
                throw Error("Unexpected: <" + arr[pos] + ">");
        }
        if (arr[pos++] != "}")
            throw Error("Expected }, found: <" + arr[pos - 1] + ">");
    }
    catch (...)
    {
        delete serv_ptr;
        throw;
    }
    return serv_ptr;
}

/*
    TODO:
        + clear parsing errors
        + remove all new , use the stack instead, be carefull of stack overflow
        + handle location without name
*/

void check(Server &parent)
{
    if (parent["autoindex"] == "")
        parent["autoindex"] = "off";
    if (parent["cgi"] == "")
        parent["cgi"] = "off";
    if (parent["name"].empty() && parent["location"].length())
        throw Error("name is required");
    if (parent["cgi"] == "on")
    {
        if (parent["exec"].empty())
            throw Error("CGI: Executable path is required");
        if (parent["ext"].empty())
            throw Error("CGI: Extention is required");
    }
    ServerChild::iterator it0;
    for (it0 = parent.children.begin(); it0 != parent.children.end(); it0++)
    {
        map<string, Server *>::iterator it1;
        for (it1 = it0->second.begin(); it1 != it0->second.end(); it1++)
        {
            Server &child = *it1->second;
            if (child.port == -1) // TODO: to be removed
                child.port = parent.port;
            if (child.limit == -1) // TODO: handle limit body size
                child.limit = parent.limit;
            // TODO: location should only starts with / and ends with no /
            child["location"] = clear_path(parent["location"] + child["location"]);
            if (child["listen"].length())
                throw Error("Parsing: child should not have a listen key");

            if (child["root"] == "")
                child["root"] = parent["root"];
            if (child["cgi"] == "")
                child["cgi"] = parent["cgi"];
            child["root"] = clear_path(child["root"]);
            if (child["name"].empty())
                child["name"] = parent["name"];
            child["listen"] = parent["listen"]; // keep it used in <ul></ul>
            map<long, string>::iterator it1;
            for (it1 = parent.errors.begin(); it1 != parent.errors.end(); it1++)
                if (child.errors[it1->first] == "")
                    child.errors[it1->first] = parent.errors[it1->first];
            check(child);
        }
    }
}

vector<Server *> parse_config_file(char *filename)
{
    vector<Server *> servs;
    try
    {
        ifstream file(filename);
        if (!file.is_open())
            throw Error(string("Failed to open file"));
        text = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();
        arr = split_string();
        // for (size_t i = 0; i < arr.size(); i++)
        //     cout << "<" << arr[i] << "> ";
        while (pos < arr.size())
        {
            if (arr[pos] == "\n")
            {
                line++;
                pos++;
            }
            else
                servs.push_back(parse());
        }
        cout << GREEN << "srv has size: " << servs.size() << RESET << endl;
        for (size_t i = 0; i < servs.size(); i++)
            check(*servs[i]);
    }
    catch (...)
    {
        for (size_t i = 0; i < servs.size(); i++)
            delete servs[i];
        throw;
    }
    return servs;
}

// Webserving
/*
TODO:
    + test one client from two servers
    + test repeated location (Transfer-Encoding multiple times in header)
    + multiple keyword on http request -> bad request
    + remove / after location in config file
    + remove multiple followed slashes
    + finish the first request then handle the second one
    + when finishing response set ready to false
    + bad request if uri contains two followed / /
    + remove all throws from the the serving part
    + verify the GET for directory
    + if no host, see it with zakaria
    + response with success page when finishing post
    + redirection for directory error status ?
    + max header size
    + max body size
    + timeout thing
    + handle multiple request on same request
    + if no hostnmae send it to first one
    + parsing config file, if cgi: on requires execute pah and extention
    + parse uri with hexadecimal value
    + use realpath function, specially in DELETE
    + if no port listen on 80
+ CMD:
    + ls /proc/$(ps -aux | grep "a.out" | awk 'NR==1{print $2}')/fd | wc -w
*/

// TODO: handle also HTTP/1.0
void generate_response(Socket *sock)
{
    Socket *res_sock = new Socket(-1, NULL, (Type)0, sock);
    HTTP &http = sock->http;
    HTTP &res_http = res_sock->http;

    string response;
    // TODO: check all status and set here the values
    switch (http.status)
    {
    case BAD_REQUEST:
        response = "Bad request";
        break;
    case FORBIDEN:
        response = "Forbiden";
        break;
    case NOT_FOUND:
        response = "Not found";
        break;
    case NOT_ALLOWED:
        response = "Not Allowed";
        break;
    case INTERNAL:
        response = "Internal error";
        break;
    case SUCCESS:
        response = "Success";
        break;
    default:
        throw Error(__LINE__, "weird error");
        break;
    }
    response = "<style>div {display: flex;justify-content: center;"
               "align-items: center;color : blue;font-size: large;}"
               "</style>"
               "<div id=\"generated\">" +
               response + "</div>";
    res_http.clen = response.length();
    res_http.buff = "HTTP/1.1 " + to_string(http.status) +
                    (http.is_error() ? " NOK\r\n" : " OK\r\n") +
                    "Content-Type: text/html\r\nContent-Length: " +
                    to_string(res_sock->http.clen) + "\r\n\r\n" +
                    response;
    sock->pair = res_sock;
    sock->action = WRITE_;
}

void fetch_config(Socket *sock)
{
    if (sock->http.is_error())
    {
        if (sock->srv->errors[sock->http.status].length())
        {
            string path = clear_path(sock->srv->Data["root"] + "/" + sock->srv->errors[sock->http.status]);
            int fd = open(path.c_str(), O_RDONLY, 0777);
            cout << "open error page " << path << endl;
            state state_;
            if (fd > 0 && stat(path.c_str(), &state_) == 0 && state_.st_mode & S_IFREG)
            {
                cout << RED "response with error page" RESET << endl;
                Socket *res_sock = new Socket(fd, NULL, FILE_, sock);
                res_sock->action = READ_;
                res_sock->http.clen = state_.st_size;
                res_sock->http.buff = "HTTP/1.1 " +
                                      to_string(sock->http.status) +
                                      " NOK\r\nContent-Type: text/html\r\nContent-Length: " +
                                      to_string(res_sock->http.clen) +
                                      "\r\n\r\n";
                res_sock->http.clen += res_sock->http.buff.length();
                return;
            }
        }
        cout << RED "response with generated error " RESET << endl;
        generate_response(sock);
        cout << sock->pair->http.buff << endl;
        return;
    }
    Method method = sock->http.method;
    string uri = sock->http.uri;

    string match;
    string remain_uri;
    Server *match_srv = NULL;

    HTTP &http = sock->http;
    vector<Server *> srvs;
    srvs.push_back(sock->srv);

    size_t i = 0;
    while (i < srvs.size())
    {
        Server &srv = *srvs[i];
        cout << "fetch: " << uri << " from " << srv["name"] << endl;
        if (starts_with(uri, srv["location"]) && srv["name"] == http.host)
        {
            if (srv["location"].length() > match.length())
            {
                match_srv = &srv;
                match = srv["location"];
                remain_uri = substr_(uri, match.length(), uri.length());
            }
        }
        if (starts_with(uri, srv["location"]) && srv.children.size() && (srv["name"] == http.host || srv["name"].empty()))
        {
            string tmp_uri = uri.substr(srv["location"].length(), uri.length());
            // cout << tmp_uri << endl;
            ServerChild::iterator it;
            for (it = srv.children.begin(); it != srv.children.end(); it++)
            {
                // cout << __LINE__ << " check if " << it->first << " starts with " << tmp_uri << endl;
                map<string, Server *>::iterator it0;
                for (it0 = it->second.begin(); it0 != it->second.end(); it0++)
                {
                    // cout << "> " << it0->first << endl;
                    Server &srv0 = *it0->second;
                    if (http.host == srv0["name"] || srv0["name"].empty())
                        srvs.push_back(it0->second);
                }
            }
        }
        i++;
    }
    if (match_srv)
    {
        Server &srv = *match_srv;
        uri = remain_uri;
        cout << uri << " starts with " << srv["location"] << endl;
        string path = clear_path(srv["root"] + "/" + srv["location"] + "/" + uri);
        cout << "search in " << path << endl;
        state state_;
        if (method == POST_ && srv["location"] == uri) // to be checked
        {
            // TODO: to check / between them
            string fname = clear_path(srv["upload"] + "/" + rand_name() + memetype[http.ctype]);
            cout << "post to filename " << fname << endl;
            // throw Error("debuging");
            // TODO: use acces instead of open
            int fd = open(fname.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0777);
            if (fd < 0)
            {
                // TODO: internal error, or permission denied
                sock->http.status = FORBIDEN;
                return fetch_config(sock);
            }
            Socket *res_sock = new Socket(fd, NULL, FILE_, sock);
            res_sock->http.method = POST_;
            res_sock->action = WRITE_;
            sock->http.status = SUCCESS;
            return;
        }
        // TODO: check nested location with on allowed mthod and other not allowed
        else if (method == GET_ && stat(path.c_str(), &state_) == 0 && srv["name"] == http.host)
        {
            if (srv[method] == "off")
            {
                cout << "============================================" << endl;
                cout << "GET"
                     << " " << srv["GET"] << endl;
                cout << method << " " << srv[method] << endl;
                pserver(&srv, 4);
                cout << "============================================" << endl;
                http.status = NOT_ALLOWED;
                return fetch_config(sock);
            }
            else if ((state_.st_mode & S_IFDIR || state_.st_mode & S_IFREG) && access(path.c_str(), R_OK) == -1)
            {
                http.status = FORBIDEN;
                return fetch_config(sock);
            }
            else if (state_.st_mode & S_IFDIR)
            {
                // TODO: fix this one, directory should not be responded this way
                string index_path = clear_path(path + "/index.html");
                cout << "index_path: " << index_path << endl;
                if (stat(index_path.c_str(), &state_) == 0 && state_.st_mode & S_IFREG)
                {
                    if (access(index_path.c_str(), R_OK) != -1)
                    {
                        // open file and get from it
                        int fd = open(index_path.c_str(), O_RDONLY, 0777);
                        Socket *res_sock = new Socket(fd, NULL, FILE_, sock);
                        res_sock->http.clen = state_.st_size;
                        res_sock->http.buff = "HTTP/1.1 200 OK\r\nContent-Type: "
                                              "text/html\r\nContent-Length: " +
                                              to_string(res_sock->http.clen) +
                                              "\r\n\r\n";
                        sock->http.status = SUCCESS;
                        return;
                    }
                    else
                    {
                        sock->http.status = FORBIDEN;
                        return fetch_config(sock);
                    }
                    return;
                }
                if (srv["autoindex"] == "off")
                {
                    http.status = FORBIDEN;
                    return fetch_config(sock);
                }
                cout << path << " is directory" << endl;
                cout << "method allowed" << endl;
                string res;
                dirent *entry;
                DIR *dir = opendir(path.c_str());
                res = "<ul>\n";
                /*
                TODO:
                    + listing directory is not working properly on postman or browser, fix it
                    + use clear path
                */
                while ((entry = readdir(dir)) != NULL)
                {
                    string url = "<a href=\"" + srv["listen"] + ":" +
                                 to_string(srv.port) + "/" + uri + "/" +
                                 string(entry->d_name) + "\">" +
                                 string(entry->d_name) + "</a><br>\n";
                    url = clear_path(url);
                    res += url;
                }
                res += "</ul>";
                closedir(dir); // TODO: check status code
                Socket *res_sock = new Socket(-1, NULL, CLIENT_, sock);
                res_sock->action = WRITE_;
                res_sock->http.clen = state_.st_size;
                res_sock->http.buff = "HTTP/1.1 200 OK\r\nContent-Type: "
                                      "text/html\r\nContent-Length: " +
                                      to_string(res.length()) +
                                      "\r\n\r\n";
                // TODO: add header len to clen
                res_sock->http.buff += res;
                sock->http.status = SUCCESS;
                return;
            }
            else if (state_.st_mode & S_IFREG)
            {
                cout << "srv[cgi]: " << srv["cgi"] << " srv[ext]: " << srv["ext"] << endl;
                if (srv["cgi"] == "on" && ends_with(path, srv["ext"]))
                {
                    cout << path << " is file cgi" << endl;

                    sock->http.cgi_fname = rand_name() + ".txt";
                    sock->http.is_cgi = true;
#if 1
                    http.pid = fork();
                    // pid_t pid = -1;
                    int status = 0;
                    if (http.pid == 0)
                    {
#if 0
                        std::vector<std::string> env;
                        map<string, string>::iterator param_it;
                        for (param_it = http.params.begin(); param_it != http.params.end(); param_it++)
                        {
                            cout << param_it->first << " => " << param_it->second << endl;
                            // TODO: check if set env failed or something
                            setenv((char *)param_it->first.c_str(), (char *)param_it->second.c_str(), 1);
                            env.push_back(param_it->first + "=" + param_it->second);
                        }
#elif 1
                        int fd = open(sock->http.cgi_fname.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
                        if (fd < 0)
                            exit(-1);
                        std::vector<std::string> env;
                        env.push_back("QUERY_STRING=" + sock->http.queries);
                        // env.push_back("QUERY_STRING=name=mohammed&age=26");

                        env.push_back("PATH_INFO=" + sock->http.uri);
                        if (sock->http.method == POST_)
                        {
                            // env.push_back("CONTENT_LENGTH=" + to_string(request.body_bytes_read));
                            // if (!request.content_type.empty())
                            //     env.push_back("CONTENT_TYPE=" + request.content_type);
                        }
                        env.push_back("REQUEST_METHOD=GET");
                        env.push_back("GATEWAY_INTERFACE=CGI/1.1");
                        // env.push_back("REMOTE_ADDR=" + client.address);
                        // env.push_back("REMOTE_HOST=");
                        // env.push_back("SCRIPT_NAME=" + request.parsed_uri.path_encoded);

                        // env.push_back("SERVER_NAME=" + client.interface + ":" + to_string(client.server_port));
                        env.push_back("SERVER_PORT=" + to_string(sock->srv->port));

                        env.push_back("SERVER_PROTOCOL=HTTP/1.1");
                        env.push_back("SERVER_SOFTWARE=");
                        env.push_back("SCRIPT_FILENAME=" + sock->http.uri);
                        env.push_back("REDIRECT_STATUS=CGI");

                        // TODO: bonus
                        // if (!request.cookie.empty())
                        // env.push_back("HTTP_COOKIE=" + request.cookie);
#endif
                        std::vector<char *> envp(env.size() + 1);
                        for (size_t i = 0; i < env.size(); i++)
                            envp[i] = (char *)env[i].c_str();
                        envp[env.size()] = 0;

                        cout << "has exec path " << srv["exec"] << endl;
                        cout << "execute       " << path << endl;
                        cout << "cgi filename " << sock->http.cgi_fname << endl;
                        cout << "env: " << endl;
                        for (size_t i = 0; i < envp.size(); i++)
                            cout << envp[i] << endl;

                        // child process
                        map<int, Socket *>::iterator it;
                        if (dup2(fd, 1) > 0)
                        {
                            char *arg[3];
                            arg[0] = (char *)srv["exec"].c_str();
                            arg[1] = (char *)path.c_str();
                            arg[2] = NULL;
                            // execve(srv["exec"].c_str(), arg, environ);
                            execve(srv["exec"].c_str(), arg, envp.data());
                        }
                        close(fd);
                        exit(-1);
                    }
                    else if (sock->http.pid < 0)
                    {
                        http.status = INTERNAL;
                        return fetch_config(sock);
                    }
                    sock->action = READ_;
#endif

                    // wait for child
                    // int val = waitpid(sock->http.pid, &status, WNOHANG);
                    // if (val < 0 || status < 0)
                    // {
                    //     http.status = INTERNAL;
                    //     return fetch_config(sock);
                    // }
                    // else
                    // {
                    //     int fd = open(cgi_fname.c_str(), O_RDONLY, 0777);
                    //     if (fd < 0)
                    //     {
                    //         throw Error("opening " + cgi_fname);
                    //     }
                    //     /*
                    //         TODOS:
                    //             + check RFC for all value that I have to set
                    //     */
                    //     stat(cgi_fname.c_str(), &state_);
                    //     cout << "pfd size : " << pfds.size() << endl;
                    //     Socket *res_sock = new Socket(fd, NULL, FILE_, sock);
                    //     cout << "pfd size : " << pfds.size() << endl;

                    //     throw Error(__LINE__, "debugging");
                    //     // TODO: check if it contains header if not add it
                    //     res_sock->http.method = sock->http.method; // to be removed maybe
                    //     res_sock->action = READ_;
                    //     res_sock->http.clen = state_.st_size;
                    //     res_sock->update_timeout();
                    //     cout << GREEN "CGI: Success" RESET << endl;
                    //     sock->http.status = SUCCESS;
                    //     sock->action = READ_;
                    // }

                    return;
                }
                else
                {
                    cout << path << " is file" << endl;
                    // throw Error(__LINE__, "debugging 1");
                    ssize_t dot_pos = path.find_last_of(".");
                    string ext;
                    // TODO: check inssupported memtyp everywhere
                    if (dot_pos == string::npos || (ext = path.substr(dot_pos, path.length())).length() == 0)
                    {
                        sock->http.status = INSUPPORTED_MEMETYPE;
                        return fetch_config(sock);
                    }
                    // open file and get from it
                    int fd = open(path.c_str(), O_RDONLY, 0777);
                    if (fd < 0)
                    {
                        // TODO: internal error
                        sock->http.status = FORBIDEN;
                        return fetch_config(sock);
                    }
                    Socket *res_sock = new Socket(fd, NULL, FILE_, sock);
                    res_sock->action = READ_;
                    res_sock->http.clen = state_.st_size;
                    // TODO: this line is repeated standarize it
                    res_sock->http.buff = "HTTP/1.1 200 OK\r\nContent-Type: " +
                                          memetype[ext] + "\r\nContent-Length: " +
                                          to_string(res_sock->http.clen) +
                                          "\r\n\r\n";
                    res_sock->http.clen += res_sock->http.buff.length();
                    sock->http.status = SUCCESS;
                    sock->action = (Action)0;
                    return;
                }
            }
        }
    }
    cout << RED "Not found" RESET << endl;
    if (sock->http.status == 0)
        sock->http.status = NOT_FOUND;
    return fetch_config(sock);
}

// TODO: optimize it
string parse_hexadecimal(string &value)
{
    string res;
    ssize_t i = 0;

    while (i < value.length())
    {
        if (value[i] == '%')
        {
            if (i + 2 >= value.length())
            {
                res += value[i++];
                continue;
            }
            std::string hex = value.substr(i + 1, 2);
            char *endptr;
            long dec = strtol(hex.c_str(), &endptr, 16);
            if (endptr != hex.c_str() + 2)
            {
                res += value[i++];
                continue;
            }
            res += (char)(dec);
            i += 3;
        }
        else
            res += value[i++];
    }
    return res;
}


void Webserv(vector<Server *> servs)
{
    // SIGNALS
    struct sigaction sa = {};
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);

    // CREATE SERVERS SOCKETS
    size_t i = 0;
    while (i < servs.size())
    {
        Socket *sock = new Socket(0, servs[i], SERVER_, NULL);
        pfds.push_back((pollfd){.fd = sock->fd, .events = POLLRDNORM});
        i++;
    }
    ssize_t timing = 0;
    cout << "pfds size " << pfds.size() << endl;

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
            throw Error("poll failed");
        else if (ready > 0)
        {
            ssize_t i = 0;
            while (i < pfds.size())
            {
                // cout << "loop" << endl;
                Socket *sock = socks[pfds[i].fd];
                cout << "pfds size " << pfds.size() << " " << sock_state(sock) << endl;
                if (sock && sock->http.is_cgi)
                {
                    // throw Error("is cgi");
                    int status = 1;
                    int val = waitpid(sock->http.pid, &status, WNOHANG);
                    if (val < 0 || status < 0)
                    {
                        cout << "did exit with " << status << endl;
                        throw Error("debuging");
                        sock->http.status = INTERNAL;
                        fetch_config(sock);
                    }
                    else if (val > 0)
                    {
                        state state_;
                        int fd = open(sock->http.cgi_fname.c_str(), O_RDONLY, 0777);
                        if (fd < 0)
                            continue;
                        /*
                            TODOS:
                                + check RFC for all value that I have to set
                        */
                        stat(sock->http.cgi_fname.c_str(), &state_);
                        cout << "pfd size : " << pfds.size() << endl;
                        Socket *res_sock = new Socket(fd, NULL, FILE_, sock);
                        cout << "pfd size : " << pfds.size() << endl;

                        // throw Error(__LINE__, "debugging");
                        // TODO: check if it contains header if not add it
                        res_sock->http.method = sock->http.method; // to be removed maybe
                        res_sock->action = READ_;
                        res_sock->http.clen = state_.st_size;
                        // res_sock->http.buff = "HTTP/1.1 200 OK\r\n"
                        //    + "Content-Type: " // TODO: modify it
                        //                       "text/html\r\nContent-Length: " +
                        //                       to_string(state_.st_size) +
                        //                       "\r\n\"
                        //   + string("\r\n");
                        // res_sock->http.clen += res_sock->http.buff.length();
                        cout << "state.size: " << state_.st_size << endl;
                        // throw Error("debugging");
#if 1
                        string header;
                        header = string("HTTP/1.1 200 OK\r\n") +
                                 "Content-Type: text/html\r\n" +
                                 "Content-Length: " + to_string(state_.st_size) + "\r\n\r\n";
                        res_sock->http.buff = header + res_sock->http.buff;
#endif
                        res_sock->update_timeout();
                        cout << GREEN "CGI: Success" RESET << endl;
                        sock->http.status = SUCCESS;
                        // sock->action = WRITE_;
                        sock->http.is_cgi = false;
                    }
                }
                if (pfds[i].revents & POLLERR)
                {
                    cout << "POLLERR" << endl;
                    delete sock;
                }
                // ACCEPT CONNECTION
                else if (sock->type == SERVER_ && pfds[i].revents)
                {
                    sockaddr_storage client_addr;
                    socklen_t client_socket_len = sizeof(client_addr);
                    int client = accept(pfds[i].fd, (sockaddr *)&client_addr, &client_socket_len);
                    if (client < 0)
                    {
                        throw Error("accept"); // TODO: do an internal error maybe
                        continue;
                    }
                    else if (client > 0)
                    {
                        Socket *client_sock = new Socket(client, sock->srv, CLIENT_, NULL);
                        inet_ntop(AF_INET, &(((struct sockaddr_in *)&client_addr)->sin_addr), client_sock->address, INET_ADDRSTRLEN);
                        cout << "New connection from " << client << " to: " << pfds[i].fd << " has addr " << client_sock->address << endl;
                    }
                }
                else if (pfds[i].revents & POLLIN && sock->action == READ_ /*&& sock->http.buff.clen == 0*/)
                {
                    cout << "POLLIN " << sock_state(sock) << endl;
                    char buff[BUFFSIZE];
                    ssize_t s, e;
                    ssize_t r = 0;
                    HTTP &http = sock->http;
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
                        // throw Error("did read 0 has buff " + sock->http.buff);
                        // sock->action = WRITE_;
                    }
                    else if (r > 0)
                    {
                        http.buff.append(buff, r);
                        
                        sock->update_timeout();
                    }
                    // TODO: "ready" will be used for long header size to determine the end of header parsing
                    if (r > 0 && sock->type == CLIENT_ && http.ready == false)
                    {
                        ssize_t pos = find(http.buff, END);
                        while (pos != string::npos)
                        {
                            // remember: I'm cutting the buff inside if statement
                            if (!http.method)
                            {
                                if (compare(http.buff, "POST "))
                                {
                                    http.method = POST_;
                                    http.uri = http.cut(strlen((char *)"POST "), e);
                                    // TODO: to be checked
                                    if (http.check(__LINE__, http.uri.empty() || http.uri[0] != '/', BAD_REQUEST))
                                        break;

                                    cout << "POST to uri <" << http.uri << ">" << endl;
                                    if (http.check(__LINE__, !compare(e, http.buff, " HTTP/1.1\r\n"), BAD_REQUEST))
                                        break;
                                }
                                else if (compare(http.buff, "GET "))
                                {
                                    http.method = GET_;
                                    http.uri = http.cut(strlen((char *)"GET "), e);
                                    // while(!isspace(http.uri[e]) && e < http.uri.length())
                                    //     e++;
                                    if (http.check(__LINE__, http.uri.empty(), BAD_REQUEST))
                                        break;
                                    http.uri = parse_hexadecimal(http.uri);
                                    // TODO: check multiple "?" -> something like "???"
                                    ssize_t q_pos = find(http.uri, (char *)"?");
                                    // ssize_t tmp_q_pos = q_pos;
                                    if (q_pos != string::npos) // TODO: add query to POST and DELETE also
                                    {
                                        // found queries
                                        cout << "found ?" << endl;
#if 0
                                        string &uri = http.uri;
                                        q_pos++;
                                        size_t s = q_pos;
                                        while (q_pos < uri.length())
                                        {
                                            if (uri[q_pos] == '=')
                                            {
                                                string key = substr_(uri, s, q_pos);
                                                q_pos++;
                                                s = q_pos;
                                                while (q_pos < uri.length() && uri[q_pos] != '&')
                                                    q_pos++;
                                                string value = substr_(uri, s, q_pos);
                                                http.params[key] = value;
                                                q_pos++;
                                                s = q_pos;
                                            }
                                            q_pos++;
                                        }
                                        map<string, string>::iterator it;
                                        for (it = http.params.begin(); it != http.params.end(); it++)
                                            cout << it->first << " => " << it->second << endl;
#else
                                        s = q_pos;
                                        while (!isspace(http.uri[s]) && s < http.uri.length())
                                            s++;
                                        http.queries = substr_(http.uri, q_pos + 1, s);
                                        http.uri = substr_(http.uri, 0, q_pos);
                                        // cout << "queries <" << http.queries << ">" << endl;
#endif
                                    }
                                    bool cond = !compare(e, http.buff, " HTTP/1.1\r\n") && !compare(e, http.buff, " HTTP/1.0\r\n");
                                    // cout << "cond " << cond << endl;
                                    // throw Error(__LINE__, "debugging");
                                    if (http.check(__LINE__, cond, BAD_REQUEST))
                                        break;
                                    // if (tmp_q_pos != string::npos)
                                    //     http.uri = substr_(http.uri, 0, tmp_q_pos);
                                    cout << "GET to uri <" << http.uri << ">" << endl;
                                }
                                else if (http.check(__LINE__, true, BAD_REQUEST)) // if no method
                                    break;
                            }
                            else if (compare(http.buff, "Connection: "))
                            {
                                string con = http.cut(strlen((char *)"Connection: "), e);
                                cout << "found con <" << con << ">" << endl;
                                if (con == "keep-alive")
                                    http.keep_alive = true;
                            }
                            else if (compare(http.buff, "Host: "))
                            {
                                http.host = http.cut(strlen((char *)"Host: "), e);
                                cout << "found Host <" << http.host << ">" << endl;
                                if (http.check(__LINE__, http.host.empty(), BAD_REQUEST))
                                    break;
                            }
                            else if (compare(http.buff, "Content-Type: "))
                            {
                                // TODO: add some insuported memtype or something ...
                                http.ctype = http.cut(strlen((char *)"Content-Type: "), e);
                                cout << "found Content-type <" << http.ctype << ">" << endl;
                                if (http.check(__LINE__, http.ctype.empty(), BAD_REQUEST))
                                    break;
                                debug << " <" << http.ctype << ">" << end_;
                            }
                            else if (compare(http.buff, "Content-Length: "))
                            {
                                string val = http.cut(strlen((char *)"Content-Length: "), e);
                                cout << "found Content-Length, val <" << val << "> check " << (val.empty() || !_isdigits(val)) << endl;
                                if (http.check(__LINE__, val.empty() || !_isdigits(val), BAD_REQUEST))
                                    break;
                                // TODO: check if content=len is valid, should be all digits and postive
                                http.clen = atol(val.c_str());
                            }
                            else if (compare(http.buff, "Transfer-Encoding: "))
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
                            else if (compare(http.buff, END))
                            {
                                if (http.trans == CHUNKED_)
                                    http.clen = 0;
                                else
                                    http.buff = substr_(http.buff, strlen(END), http.buff.length());
                                http.ready = true;
                                break;
                            }
                            http.buff = substr_(http.buff, pos + strlen(END), http.buff.length());
                            pos = find(http.buff, END);
                        }
                        if (http.ready)
                        {
                            cout << GREEN "ready" RESET << endl;
                            // TODO: becarefull with boundry case
                            http.check(__LINE__, http.host.empty() || (http.method == POST_ && http.ctype.empty()), BAD_REQUEST);
                            if (http.is_error() || http.method == GET_) // TODO: set it in fetch_config
                                sock->action = WRITE_;
                            fetch_config(sock);
                        }
                    }
                    else if (r > 0 && sock->type == FILE_)
                    {
                        sock->action = WRITE_;                       // TODO: set it to read in POLLOUT, when writing it's buff to the socket
                        sock->http.status = sock->pair->http.status; // to be checked
                        // cout << "has pair " << (sock->pair->type == CLIENT_ ? "client" : sock->pair->type == FILE_ ? "file"
                        //                                                              : sock->pair->type == SERVER_ ? "server"
                        //                                                                                            : "unkown")
                        //      << endl;
                        cout << "has buff <" << sock->http.buff << ">" << endl;
                        cout << "status " << sock->http.status << endl;
                        // throw Error(__LINE__, "read from file " + to_string(r) + "\n");
                        // close(sock->fd);
                    }
                    // TODO: reset everyting after finishing reponse
                    if (sock->type == CLIENT_ && http.method == POST_ && http.trans == CHUNKED_ && http.ready && http.clen == 0)
                    {
                        cout << GREEN "is chunked" RESET << endl;
                        http.buff = substr_(http.buff, strlen(END), http.buff.length());
                        pos = find(http.buff, END);
                        if (pos != string::npos)
                        {
                            cout << "<" << http.buff << ">" << endl;
                            cout << GREEN "parse number" RESET << endl;
                            // chunk_size\r\n
                            string val = substr_(http.buff, 0, pos);
                            http.buff = substr_(http.buff, pos + strlen((char *)END), http.buff.length());
                            cout << "buff <" << http.buff << ">" << endl;
                            cout << "val  <" << val << ">" << endl;

                            // throw Error("debuging");
                            char *ptr = NULL;
                            http.clen = strtol(val.c_str(), &ptr, 16);
                            // http.flen = http.clen;
                            if (ptr == NULL)
                                throw Error("parsing number");
                            sock->pair->action = WRITE_;
                            cout << "clen " << http.clen << endl;
                            if (http.clen == 0)
                            {
                                // throw Error("reached end of chunk 0");
                                delete sock->pair;
                                http.status = 200;
                                generate_response(sock);
                            }
                        }
                    }
                }
                else if (pfds[i].revents & POLLOUT && sock->action == WRITE_)
                {
                    cout << "POLLOUT" << endl;
                    Socket *pair = sock->pair;
                    HTTP &http = sock->http;
                    ssize_t bytes = 0;
                    if (http.method == GET_)
                    {
                        if (sock->type == CLIENT_ && http.status)
                        {
                            debug << " send <" << pair->http.buff.c_str() << ">" << endl;
                            bytes = pair->http.buff.length() > BUFFSIZE ? BUFFSIZE : pair->http.buff.length();
                            bytes = write(sock->fd, pair->http.buff.c_str(), bytes);
                            debug << sock_state(sock) << end_;
                            debug << sock_state(sock->pair) << end_;
                            // throw Error(__LINE__, "debuging");
                            if (bytes < 0) // TODO: to be removed
                            {
                                // throw Error(__LINE__, "write failed");
                                cout << "write failed" << endl;
                                // delete sock;
                                // delete pair;
                                continue;
                            }
                            else if (bytes > 0)
                            {
                                pair->http.buff = pair->http.buff.substr(bytes, pair->http.buff.length());
                                pair->http.clen -= bytes;
                                sock->update_timeout();
                                debug << " clen " << pair->http.clen << " buff <" << pair->http.buff << ">" << endl;
                                if (pair->http.clen <= 0)
                                {
                                    cout << RED "reached the end of file" RESET << endl;
                                    // delete sock;
                                    delete pair;
                                    sock->action = READ_;
                                    continue;
                                }
                            }
                        }
                        else if (sock->type == FILE_ && http.status)
                        {
                            debug << " send <" << sock->http.buff.c_str() << ">" << endl;
                            string header;
#if 0
                            header = string("HTTP/1.1 200 OK\r\n") +
                                     "Content-Type: text/html\r\n" +
                                     "Content-Length: " + to_string(sock->http.buff.length()) + "\r\n\r\n";
                            sock->http.buff = header + sock->http.buff;
#endif                       
                            bytes = sock->http.buff.length() > BUFFSIZE ? BUFFSIZE : sock->http.buff.length();
                            bytes = write(pair->fd, sock->http.buff.c_str(), bytes);
                            // throw Error(__LINE__, " debugging");
                            if (bytes < 0) // TODO: to be removed
                            {
                                cout << "write failed" << endl;
                                throw Error(__LINE__, "write failed");
                                // delete pair;
                                // delete file;
                                continue;
                            }
                            else if (bytes > 0)
                            {
                                sock->http.buff = sock->http.buff.substr(bytes, sock->http.buff.length());
                                sock->http.clen -= bytes;
                                sock->update_timeout();
                                debug << " clen " << sock->http.clen << " buff <" << sock->http.buff << ">" << endl;
                                if (sock->http.clen <= 0)
                                {
                                    cout << RED "reached the end of file" RESET << endl;
                                    // delete pair;
                                    delete sock;
                                    pair->action = READ_;
                                    continue;
                                }
                            }
                        }
                    }
                    else if (http.method == POST_)
                    {
                        if (sock->type == CLIENT_ && http.status)
                        {
                            cout << "response to client" << endl;
                            cout << "<" << pair->http.buff << ">" << endl;
                            // TODO: use BUFFSIZE
                            bytes = write(sock->fd, pair->http.buff.c_str(), pair->http.buff.length());
                            if (bytes < 0) // TODO: to be removed
                            {
                                throw Error(__LINE__, "write failed");
                                delete sock;
                                delete pair;
                                continue;
                            }
                            else if (bytes > 0)
                            {
                                pair->http.buff = substr_(pair->http.buff, bytes, pair->http.buff.length());
                                sock->update_timeout();
                            }
                        }
                        else if (sock->type == FILE_)
                        {
                            bytes = pair->http.clen > BUFFSIZE ? BUFFSIZE : pair->http.clen;
                            bytes = bytes > pair->http.buff.length() ? pair->http.buff.length() : bytes;
                            bytes = write(sock->fd, pair->http.buff.c_str(), bytes);
                            if (bytes > 0)
                            {
                                pair->http.buff = substr_(pair->http.buff, bytes, pair->http.buff.length());
                                if (http.trans == CHUNKED_ && starts_with(pair->http.buff, "\r\n0\r\n"))
                                {
                                    cout << GREEN "generate response" RESET << endl;
                                    delete sock;
                                    pair->http.status = 200;
                                    generate_response(pair);
                                    cout << "<" << pair->pair->http.buff << ">" << endl;
                                }
                                else
                                {
                                    pair->http.clen -= bytes;
                                    if (pair->http.clen < 0)
                                    {
                                        throw Error("something went wrong"); // to be removed
                                    }
                                    if (pair->http.clen == 0)
                                    {
                                        pair->action = READ_;
                                        pair->http.ready = true;
                                        delete sock;
                                        pair->http.status = 200;
                                        generate_response(pair);
                                    }
                                }
                                sock->update_timeout(); // TODO: update it if not deleted
                            }
                            else if (bytes < 0)
                                throw Error("writing");
                        }
                    }
                }
                if (sock->type != SERVER_ && !sock->http.keep_alive && time(NULL) - sock->timeout > CLIENT_TIMEOUT)
                {
                    if (sock->pair)
                        sock->pair->pair = NULL;
                    cout << "timeout on " << sock_state(sock) << endl;
                    throw Error("debugging");
                    delete sock;
                }
                i++;
            }
        }
    }
}

int main(int argc, char **argv)
{
    vector<Server *> servs;
    try
    {
        if (argc != 2)
            throw Error(string("Invalid argument"));
        servs = parse_config_file(argv[1]);
        for (size_t i = 0; exts_struct[i].type.length(); ++i)
        {
            memetype[exts_struct[i].type] = exts_struct[i].ext;
            memetype[exts_struct[i].ext] = exts_struct[i].type;
        }
        for (size_t i = 0; i < servs.size(); i++)
            pserver(servs[i], 8);
        for (size_t i = 0; i < servs.size(); i++)
        {
            for (size_t j = i + 1; j < servs.size(); j++)
            {
                Server &srv1 = *servs[i];
                Server &srv2 = *servs[j];
                if (srv1["listen"] == srv2["listen"] && srv1.port == srv2.port)
                    throw Error("can not have same host and port on multiple servers");
            }
        }
        Webserv(servs);
    }
    catch (exception &err)
    {
        cerr << RED "Error: " << err.what() << RESET << endl;
    }
    map<int, Socket *>::iterator it;
    for (it = socks.begin(); it != socks.end(); it++)
        delete it->second;
    for (size_t i = 0; i < servs.size(); i++)
        delete servs[i];
}