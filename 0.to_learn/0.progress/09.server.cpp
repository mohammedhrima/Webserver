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

// TODO: set a max events
typedef struct addrinfo addrinfo;
typedef struct sockaddr_in sockaddr_in;
typedef struct pollfd pollfd;
typedef struct sockaddr sockaddr;
typedef struct sockaddr_storage sockaddr_storage;
typedef struct Server Server;
typedef struct Socket Socket;
typedef struct timeval timeval;
typedef struct stat state;
typedef struct dirent dirent;
using namespace std;

#if 0
#define CLIENT_TIMEOUT 10
#endif

#define BAD_REQUEST 400
#define FORBIDEN 403
#define NOT_FOUND 404
#define NOT_ALLOWED 405
#define INTERNAL 500

#define POLLTIMEOUT 10
#define LISTEN_LEN SOMAXCONN
#define BUFFSIZE 8192
// #define MAXLEN 4096
#define END "\r\n"
#define HEADEREND "\r\n\r\n"
#define SOCK_TIMEOUT 2

#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define RESET "\033[0m"

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

class Error : public exception
{
private:
    string message;

public:
    Error(string msg) throw() : message(msg) {}
    virtual ~Error() throw() {}
    const char *what() const throw() { return message.c_str(); }
};

struct Server
{
    map<string, string> Data;
    ssize_t port;
    /*
    TODOS:
        + check negative port value
        + port should be required
    */
    map<long, string> errors;
    ssize_t limit;
    map<string, Server *> children;

    // constractor
    Server()
    {
        cout << "call constractor" << endl;
        port = -1;
        limit = -1;
        Data["name"] = Data["listen"] = Data["location"] = "";
        Data["index"] = Data["upload"] = Data["root"] = Data["autoindex"] = "";
        Data["GET"] = Data["POST"] = Data["DELETE"] = "off";
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
        map<string, Server *>::iterator it;

        for (it = children.begin(); it != children.end(); it++)
            delete it->second;
        Data.clear();
        errors.clear();
        children.clear();
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
        map<long, string>::iterator it3;
        for (it3 = srv->errors.begin(); it3 != srv->errors.end(); it3++)
        {
            if (it3->second.length())
                cout << setw(space + 6) << it3->first << " : <" << it3->second << ">" << endl;
        }
    }
    cout << setw(space + 3) << "PORT : " << srv->port << "\n\n";
    map<string, Server *>::iterator it3;
    for (it3 = srv->children.begin(); it3 != srv->children.end(); it3++)
        pserver(srv->children[it3->first], space + 4);
}

// PARSING
bool _isspace(char c)
{
    return (c == ' ');
}

bool _isdigits(string &str)
{
    size_t i = 0;
    while (i < str.length())
    {
        if (!isdigit(str[i]))
            return false;
        i++;
    }
    return str.length() && str[i] == '\0';
}

string size_to_string(size_t size)
{
    stringstream stream;
    stream << size;
    return stream.str();
}

bool starts_with(string str, string sub)
{
    return str.length() >= sub.length() && str.compare(0, sub.length(), sub) == 0;
}

bool ends_with(string str, string sub)
{
    return str.length() >= sub.length() &&
           str.compare(str.length() - sub.length(), sub.length(), sub) == 0;
}

string rand_name()
{
    timeval current_time;
    gettimeofday(&current_time, NULL);
    localtime(&current_time.tv_sec);
    stringstream filename;
    filename << current_time.tv_sec;
    return filename.str();
}

// state get_file_state(string path)
// {
//     stat state;
//     return state;
// }

string text;
size_t pos;
vector<string> arr;

vector<string> split_string()
{
    text.erase(remove_if(text.begin(), text.end(), _isspace), text.end());
    vector<string> result;
    stringstream ss(text);
    string token;
    while (getline(ss, token, '\n'))
    {
        size_t pos = 0;
        while ((pos = token.find_first_of(";{}:,#\n", pos)) != string::npos)
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

string clear_path(string &path)
{
    string res;
    bool prevIsSlash = false;
    for (size_t i = 0; i < path.length(); ++i)
    {
        if (path[i] == '/')
        {
            if (!prevIsSlash)
                res += '/';
            prevIsSlash = true;
        }
        else
        {
            res += path[i];
            prevIsSlash = false;
        }
    }
    if (res.length() && res[res.length() - 1] == '/')
        res.erase(res.length() - 1);
    if (res.length() == 0)
        res = "/";
    return res;
}

Server *parse()
{
    Server *serv_ptr = new Server();
    Server &srv = *serv_ptr;

    if (arr[pos++] != "{")
        delete serv_ptr, throw Error("Parsing 0: Expected '{' got <" + arr[pos - 1] + ">");
    while (pos < arr.size() && arr[pos] != "}")
    {
        string Properties[] = {"name", "root", "upload", "index",
                               "limit", "autoindex", "listen", "location"};
        bool found = false;
        for (size_t i = 0; i < sizeof(Properties) / sizeof(Properties[0]); i++)
        {
            if (arr[pos] == Properties[i])
            {
                found = true;
                pos++;
                if (arr[pos++] != ":")
                    delete serv_ptr, throw Error("Parsing 1: Expected ':' got <" + arr[pos - 1] + ">");
                if (srv[Properties[i]].length())
                    delete serv_ptr, throw Error("Repeated key " + Properties[i]);
                else if (!strchr(";{}:,#", arr[pos][0]))
                    srv[Properties[i]] = arr[pos++];
                else
                    delete serv_ptr, throw Error("Invalid value");
                if (Properties[i] == "listen" && arr[pos] == ":")
                {
                    pos++;
                    if (_isdigits(arr[pos]))
                        srv.port = atol(arr[pos++].c_str()); // TODO: check valid value for port
                    else
                        delete serv_ptr, throw Error("Invalid port");
                }
                else if (Properties[i] == "root")
                    srv["root"] = clear_path(srv["root"]);
                else if (Properties[i] == "location")
                {
                    string value = srv["location"];
                    if (srv["location"][0] != '/')
                        delete serv_ptr, throw Error("location should starts with / <" + value + ">");
                    if (srv["location"].find('/', 1) != string::npos)
                        delete serv_ptr, throw Error("location should contains only one /, at the beginning <" +
                                                     value + ">");
                    srv["location"] = clear_path(srv["location"]);
                }
                if (arr[pos++] != ";")
                    delete serv_ptr, throw Error("Parsing 2: Expected ';' got <" + arr[pos - 1] + ">");
                break;
            }
        }
        if (found)
            continue;
        else if (arr[pos] == "methods")
        {
            pos++;
            if (arr[pos++] != ":")
                delete serv_ptr, throw Error("Parsing 3: Expected ':' got <" + arr[pos - 1] + ">");
            while (arr[pos] == "GET" || arr[pos] == "POST" || arr[pos] == "DELETE" || arr[pos] == ",")
            {
                if (arr[pos] != ",")
                    srv[arr[pos]] = "on";
                pos++;
            }
            if (arr[pos++] != ";")
                delete serv_ptr, throw Error("Parsing 4: Expected ';' got <" + arr[pos - 1] + ">");
            continue;
        }
        else if (arr[pos] == "errors")
        {
            pos++;
            while (arr[pos] == "\n")
                pos++;
            if (arr[pos++] != "{")
                delete serv_ptr, throw Error("Parsing 7: Expected '{' got <" + arr[pos - 1] + ">");
            while (arr[pos] != "}" && pos < arr.size())
            {
                if (_isdigits(arr[pos])) // TODO: check 400 < error status < 500
                {
                    long status = atol(arr[pos++].c_str());
                    if (arr[pos++] != ":")
                        delete serv_ptr, throw Error("Parsing 8: Expected ':' got <" + arr[pos - 1] + ">");
                    if (!ends_with(arr[pos], ".html"))
                        delete serv_ptr, throw Error("Parsing 9: invalid error page, got <" + arr[pos] + ">");
                    srv.errors[status] = arr[pos++];
                    if (arr[pos++] != ";")
                        delete serv_ptr, throw Error("Parsing 10: Expected ';' got <" + arr[pos - 1] + ">");
                }
                else if (arr[pos] == "\n")
                    pos++;
                else
                    delete serv_ptr, throw Error("Parsing 11: Unexpected <" + arr[pos] + ">");
            }
            if (arr[pos++] != "}")
                delete serv_ptr, throw Error("Parsing 12: Expected '}' got <" + arr[pos - 1] + ">");
            continue;
        }
        else if (arr[pos] == "{")
        {
            try
            {
                Server *child = parse();
                if (srv.children[(*child)["location"]])
                    delete child, throw Error("Repeated location");
                srv.children[(*child)["location"]] = child;
            }
            catch (...)
            {
                delete serv_ptr;
                throw;
            }
            continue;
        }
        else if (arr[pos] == "\n" || arr[pos] == ";")
        {
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
            delete serv_ptr, throw Error("Parsing 5: found: <" + arr[pos] + ">");
    }
    if (arr[pos++] != "}")
        delete serv_ptr, throw Error("Parsing 6: Expected }, found: <" + arr[pos - 1] + ">");
    return serv_ptr;
}

void check(Server &parent)
{
    // cout << "parent has port : " << parent.port << endl;
    // cout << "check" << endl;

    map<string, Server *>::iterator it;
    for (it = parent.children.begin(); it != parent.children.end(); it++)
    {
        Server &child = *it->second;
        if (child.port == -1) // TODO: to be removed
            child.port = parent.port;
        if (child.limit == -1)
            child.limit = parent.limit;
        // TODO: location should only starts with / and ends with no /
        child["location"] = parent["location"] + child["location"];
        if (child["listen"].length())
            throw Error("Parsing: child should not have a listen key");
        if (child["name"].length())
            throw Error("Parsing: child should not have a name key");

        if (child["root"] == "")
            child["root"] = parent["root"] + child["location"];
        child["name"] = parent["name"];
        map<long, string>::iterator it1;
        for (it1 = parent.errors.begin(); it1 != parent.errors.end(); it1++)
        {
            // cout << "<" << it1->first << ">" << endl;
            if (child.errors[it1->first] == "")
                child.errors[it1->first] = parent.errors[it1->first];
        }
        check(child);
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
                pos++;
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

/*
TODO:
    + protect multiple servers on same port
    + error in duplicated value in config file
    + each scoop must have a location, if not throw an error in parsing
    + throw error in duplicated locations in childs
    + add POLLERROR, POLLHANGUP
    + check EWOULDBLOCK
    + HOST must is required
    + test one client from two servers
    + test repeated location (Transfer-Encoding multiple times in header)
    + multiple keyword on htpp request -> bad request
    + always join location to the path
+ CMD:
    + ls /proc/$(ps -aux | grep "a.out" | awk 'NR==1{print $2}')/fd | wc -w
*/
typedef struct HTTP HTTP;
struct HTTP
{
    string buff;

    Method method;
    string uri;

    string resp;
    string hostname;

    // bool is_error; // to be removed
    int error;

    string content_type;
    ssize_t content_len;

    ssize_t full_len;
    bool ready;

    HTTP()
    {
        method = (Method)0;
        // is_error = false;
        // status = 0;
        error = 0;
        content_len = 0;
        full_len = 0;
        ready = false;
    }
    ~HTTP() {}
};

map<string, string> memetype;
map<int, Socket *> socks;
// map<int, Socket *> pairs;
vector<pollfd> pfds;

struct Socket
{
    Server *srv;
    Action action;
    Type type;
    HTTP http;
    bool keep_alive;

    time_t timeout;
    Socket *pair;

    int fd;
    // TODO: set all struct values
    Socket(int fd_, Server *srv, Type type_, Socket *pair_) : fd(fd_), type(type_), action(READ_),
                                                              pair(pair_), timeout(0), srv(srv),
                                                              http(), keep_alive(false)
    {
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
        }
        else if (type == FILE_ || type == CLIENT_)
        {
            if (fd > 0)
            {
                if (socks[fd])
                {
                    throw Error("weird error 1"); // to be removed
                    // delete socks[fd];
                }
                pfds.push_back((pollfd){.fd = fd, .events = POLLIN | POLLOUT});
                if (type == CLIENT_)
                    set_non_blocking();
            }
            if (pair)
                pair->pair = this;
        }
        socks[fd] = this;
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
    }

    // destractor
    ~Socket()
    {
        cout << "kill socket " << fd << " has type " << (type == FILE_ ? "FILE" : "SOCKET") << endl;
        close(fd);
        for (vector<pollfd>::iterator it = pfds.begin(); it != pfds.end(); ++it)
        {
            if (it->fd == fd)
            {
                pfds.erase(it);
                break;
            }
        }
        socks[fd] = NULL;
        cout << "pfds size " << pfds.size() << endl;
    }
};

/*
TODOS:
    + get the index.html from location if PATH is empty
    + remove / after location in config file
    + clear filename before sending it to fetch_config
    + remove multiple followed slashes
    + location on config file should contains only characters and digits no '.'
    + location sould be only alpha number and - and _
    + if root has value ./ turn it to ""
    + handle hostnames in config file, and use it for request
    + test autoindex on (maybe auto index is only for folders)
    + play with mutiple servers
    + mulitplw servers cause a weird behavious fix it
*/

// TODO: optimize this shity code
void generate_error(Socket *sock)
{
    Socket *fsock = new Socket(-1, NULL, CLIENT_, sock);

    string msg;
    switch (sock->http.error)
    {
    case BAD_REQUEST:
        msg = "Bad request";
        break;
    case FORBIDEN:
        msg = "Forbiden";
        break;
    case NOT_FOUND:
        msg = "Not found";
        break;
    case NOT_ALLOWED:
        msg = "Not Allowed";
        break;
    case INTERNAL:
        msg = "Internal error";
        break;
    default:
        throw Error("weird error 1");
        break;
    }
    msg = "<style>"
          "div {display: flex;justify-content: center;"
          "align-items: center;color : blue;font-size: large;}"
          "</style><div>" +
          msg + "</div>";
    fsock->http.full_len = msg.length();
    fsock->action = WRITE_;
    fsock->http.buff = "HTTP/1.0 " +
                       size_to_string(sock->http.error) +
                       " NOK\r\nContent-Type: text/html\r\nContent-Length: " +
                       size_to_string(fsock->http.full_len) +
                       "\r\n\r\n" + msg;
};

// optimize this function
string fetch_config(Socket *sock) // TODO: handle hostname
{
    if (sock->http.error)
    {
        // TODO: check if it does exists
        string path = sock->srv->Data["root"] + "/" + sock->srv->errors[sock->http.error];
        if (path.length())
        {
            int fd = open(path.c_str(), O_RDONLY, 0777);
            cout << "open error page " << path << endl;
            state state_;
            if (fd > 0 && stat(path.c_str(), &state_) == 0 && state_.st_mode & S_IFREG)
            {
                cout << RED "response with error page" RESET << endl;
                Socket *fsock = new Socket(fd, NULL, FILE_, sock);
                fsock->action = READ_;
                fsock->http.full_len = state_.st_size;
                fsock->http.buff = "HTTP/1.0 " +
                                   size_to_string(sock->http.error) +
                                   " NOK\r\nContent-Type: text/html\r\nContent-Length: " +
                                   size_to_string(fsock->http.full_len) +
                                   "\r\n\r\n";
                return path;
            }
        }
        cout << RED "response with generated error " RESET << endl;
        generate_error(sock);
        cout << sock->pair->http.buff << endl;
        return "";
    }
    else
    {
        Method method = sock->http.method;
        string uri = sock->http.uri;

        cout << "fetch: " << uri << " from " << sock->srv->Data["name"] << endl;
        vector<Server *> servs;
        servs.push_back(sock->srv);
        size_t i = 0;
        while (i < servs.size())
        {
            Server &srv = *servs[i];
            cout << "compare " << uri << " and " << srv["location"] << endl;
            // TODO: if uri / find index.html
            if (starts_with(uri, srv["location"]) && srv["name"] == sock->http.hostname)
            {
                cout << __LINE__ << " " << uri << " starts with " << srv["location"] << endl;
                // join it to root and check if it's a file or a folder
                string path = srv["root"] + uri;
                cout << "search in " << path << endl;
                state state_;
                if (stat(path.c_str(), &state_) == 0)
                {
                    if (state_.st_mode & S_IFDIR)
                    {
                        // it's a directory
                        // check read is access
                        // if auto index on send index.html
                        // else open it and create an html response
                        cout << path << " is directory" << endl;
                        if (method == GET_)
                        {
                            if (srv[method] == "on")
                            {
                                if (access(path.c_str(), R_OK) != -1)
                                {
                                    string res;
                                    dirent *entry;
                                    DIR *dir = opendir(path.c_str());
                                    res = "<ul>\n";
                                    while ((entry = readdir(dir)) != NULL)
                                    {
                                        string url = "<a href=\"" + uri + "/" +
                                                     string(entry->d_name) + "\">" +
                                                     string(entry->d_name) + "</a><br>\n";
                                        res += url;
                                    }
                                    res += "</ul>";
                                    closedir(dir); // TODO: check status code

                                    Socket *fsock = new Socket(-1, NULL, CLIENT_, sock);
                                    fsock->action = WRITE_;
                                    fsock->http.full_len = state_.st_size;
                                    fsock->http.buff = "HTTP/1.0 200 OK\r\nContent-Type: "
                                                       "text/html\r\nContent-Length: " +
                                                       size_to_string(res.length()) +
                                                       "\r\n\r\n";
                                    fsock->http.buff += res;
                                    return path;
                                }
                                else
                                {
                                    sock->http.error = FORBIDEN;
                                    return fetch_config(sock);
                                }
                            }
                            else
                            {
                                sock->http.error = NOT_ALLOWED;
                                return fetch_config(sock);
                            }
                            // throw Error("method not allowed");
                        }
                    }
                    else if (state_.st_mode & S_IFREG)
                    {
                        cout << path << " is file" << endl;
                        if (method == GET_)
                        {
                            if (srv[method] == "on")
                            {
                                if (access(path.c_str(), R_OK) != -1)
                                {
                                    ssize_t dot_pos = path.find_last_of(".");
                                    if (dot_pos == string::npos)
                                    {
                                        // TODO: maybe you need to change it
                                        sock->http.error = BAD_REQUEST;
                                        return fetch_config(sock);
                                    }
                                    else
                                    {
                                        // open file and get from it
                                        string ext = path.substr(dot_pos, path.length());
                                        int fd = open(path.c_str(), O_RDONLY, 0777);
                                        if (fd < 0)
                                        {
                                            // TODO: internal error
                                            sock->http.error = FORBIDEN;
                                            return fetch_config(sock);
                                        }
                                        Socket *fsock = new Socket(fd, NULL, FILE_, sock);
                                        fsock->http.full_len = state_.st_size;
                                        fsock->http.buff = "HTTP/1.0 200 OK\r\nContent-Type: " +
                                                           memetype[ext] + "\r\nContent-Length: " +
                                                           size_to_string(fsock->http.full_len) +
                                                           "\r\n\r\n";
                                        return path;
                                    }
                                }
                                else
                                {
                                    sock->http.error = FORBIDEN;
                                    return fetch_config(sock);
                                }
                            }
                            else
                            {
                                sock->http.error = NOT_ALLOWED;
                                return fetch_config(sock);
                            }
                            // throw Error("method not allowed");
                        }
                    }

                    else
                    {
                        // TODO: list all file and directories
                    }
                }
                else if (srv.children.size())
                {
                    string tmp_uri = uri.substr(srv["location"].length(), uri.length());
                    map<string, Server *>::iterator it;
                    for (it = srv.children.begin(); it != srv.children.end(); it++)
                    {
                        cout << __LINE__ << " check if " << it->first << " starts with " << tmp_uri << endl;
                        if (starts_with(it->first, tmp_uri))
                            servs.push_back(it->second);
                    }
                }
            }
            i++;
        }
    }
    cout << RED "Not found" RESET << endl;
    sock->http.error = NOT_FOUND;
    return fetch_config(sock);
}

/*
TODOS:
    + when reading from file, if read 0 close file
    + finish the first reuqest then handle the second one
    + when finishing response set ready to false
    + bad request if uri contains two followed / /
    + remove all throws from the the serving part
*/

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
            throw Error(string("poll failed\n"));
        else if (ready > 0)
        {
            ssize_t i = 0;
            while (i < pfds.size())
            {
                Socket *sock = socks[pfds[i].fd];
                // ERROR
                if (pfds[i].revents & POLLERR)
                {
                    cout << "POLLERR" << endl;
                    delete sock;
                    // continue;
                }
                // ACCEPT CONNECTION
                else if (sock->type == SERVER_ && pfds[i].revents)
                {
                    sockaddr_storage client_addr;
                    socklen_t client_socket_len = sizeof(client_addr);
                    int client = accept(pfds[i].fd, (sockaddr *)&client_addr, &client_socket_len);
                    if (client < 0)
                    {
                        throw Error(string("accept")); // TODO: do an internal error maybe
                        continue;
                    }
                    else if (client > 0)
                    {
                        new Socket(client, sock->srv, CLIENT_, NULL);
                        cout << "New connection from " << client << " to: " << pfds[i].fd << endl;
                    }
                }
                else if (pfds[i].revents & POLLIN && sock->action == READ_)
                {
                    char buff[BUFFSIZE];
                    ssize_t r = read(pfds[i].fd, buff, BUFFSIZE);
                    if (r < 0)
                        throw Error(string("reading"));
                    else if (r > 0)
                    {
                        // cout << "read " << r << " bytes" << endl;
                        cout.write(buff, r);
                        cout << endl;
                        sock->http.buff.append(buff, r);
                        sock->update_timeout();
                    }
                    HTTP &http = sock->http;
                    if (r > 0 && sock->type == CLIENT_)
                    {
                        // parse request
                        // cout << "method " << http.method << endl;
                        ssize_t pos = http.buff.find(END, 0, strlen(END));
                        while (pos != string::npos)
                        {
                            ssize_t rpos;
                            ssize_t s;
                            if (!http.method)
                            {
                                if ((rpos = http.buff.rfind(string("GET "), 0)) != string::npos)
                                {
                                    http.method = GET_;
                                    rpos += string("GET ").length();
                                    s = rpos;
                                    while (rpos < http.buff.length() && !isspace(http.buff[rpos]))
                                        rpos++;
                                    if (s == rpos)
                                    {
                                        sock->http.ready = true;
                                        sock->http.error = BAD_REQUEST;
                                        break;
                                    }
                                    // throw Error(string("Invalid request 1\n"));
                                    // cout << "s: " << s << ", rpos: " << rpos << endl;
                                    http.uri = http.buff.substr(s, rpos - s);
                                    // cout << "uri: " << http.uri << endl;
                                    if (http.buff.substr(rpos, pos - rpos) != " HTTP/1.1")
                                    {
                                        sock->http.ready = true;
                                        sock->http.error = BAD_REQUEST;
                                        break;
                                    }
                                    // throw Error(string("Invalid request 2\n"));
                                }
                                else
                                {
                                    sock->http.ready = true;
                                    sock->http.error = BAD_REQUEST;
                                    break;
                                }
                            }
                            else if ((rpos = http.buff.rfind(string("Host: "), 0)) != string::npos)
                            {
                                rpos += string("Host: ").length();
                                s = rpos;
                                while (rpos < http.buff.length() && !isspace(http.buff[rpos]))
                                    rpos++;
                                if (s == rpos)
                                {
                                    cout << RED "bad request" RESET << endl;
                                    sock->http.ready = true;
                                    sock->http.error = BAD_REQUEST;
                                    break;
                                }
                                // throw Error(string("Invalid request 3\n"));
                                http.hostname = http.buff.substr(s, rpos - s);
                                // cout << "s: " << s << ", rpos: " << rpos << endl;
                                // cout << "hostname <" << http.hostname << ">" << endl;
                                // throw(Error("debuging"));
                            }
                            else if ((rpos = http.buff.rfind(string("Connection: "), 0)) != string::npos)
                            {
                                rpos += string("Connection: ").length();
                                s = rpos;
                                while (rpos < http.buff.length() && !isspace(http.buff[rpos]))
                                    rpos++;
                                if (s == rpos)
                                {
                                    sock->http.ready = true;
                                    sock->http.error = BAD_REQUEST;
                                    break;
                                }
                                // throw Error(string("Invalid request 4\n"));
                                if (http.buff.substr(s, rpos - s) == "keep-alive")
                                    sock->keep_alive = true;
                                cout << (sock->keep_alive ? "keep_live" : "not_keep_alive") << endl;
                                // throw(Error("debuging"));
                            }
                            else if (http.buff.compare(0, strlen(END), END, 0, strlen(END)) == 0)
                            {
                                http.buff = http.buff.substr(pos + strlen(END), http.buff.length());
                                /*
                                TODO:
                                    + if Connection: keep-alive (don't close connection)
                                    + if POST: (requirement)
                                        + content-type
                                        + content-length (except for chunked an boundry)
                                    + elif GET: (requirement)
                                        + uri
                                        + hostname
                                    + else bad request
                                */
                                http.ready = true;
                                break;
                            }
                            http.buff = http.buff.substr(pos + strlen(END), http.buff.length());
                            pos = http.buff.find(END, 0, strlen(END));
                        }
                        if (http.ready)
                        {
                            if (http.hostname.empty())
                                http.error = BAD_REQUEST;
                            if (http.error == 0)
                            {
                                if (http.method == POST_ && (http.content_type.empty() || !http.content_len))
                                    http.error = BAD_REQUEST; // TODO: to be checked

                                // else  if (http.method == GET_ &&)
                            }
                            if (http.method == 0)
                                http.method = GET_;
                            if (http.method == GET_)
                                sock->action = WRITE_;
                            fetch_config(sock);
                            http.ready = false;
                        }
                    }
                    else if (r > 0 && sock->type == FILE_)
                    {
                        sock->action = WRITE_; // TODO:set it to read in POLLOUT
                                               // when writing it's buuf to the socket
                        // cout << "to file" << endl;
                    }
                }
                else if (pfds[i].revents & POLLOUT && sock->action == WRITE_)
                {
                    Socket *src = sock->pair;
                    if (sock->http.method == GET_ && src->action == WRITE_)
                    {
                        ssize_t bytes = src->http.buff.length() > BUFFSIZE ? BUFFSIZE : src->http.buff.length();
                        bytes = write(sock->fd, src->http.buff.c_str(), bytes);
                        cout << "send " << src->http.buff << endl;
                        if (bytes <= 0)
                        {
                            delete sock;
                            delete src;
                            // throw Error("write 0");
                            continue;
                        }
                        else if (bytes > 0)
                        {
                            src->http.buff = src->http.buff.substr(bytes, src->http.buff.length());
                            src->http.full_len -= bytes;
                            sock->update_timeout();
                            if (src->http.full_len <= 0)
                            {
                                cout << "reached the end of file" << endl;
                                delete sock;
                                delete src;
                                continue;
                            }
                        }
                    }
                    // }
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
                if (
                    (servs[i]->Data["listen"] == servs[j]->Data["listen"]) &&
                    (servs[i]->Data["name"] == servs[j]->Data["name"]) &&
                    (servs[i]->port == servs[j]->port))
                    throw Error("can not have same listen, name, port on same multiple servers");
            }
        }
        Webserv(servs);
    }
    catch (exception &err)
    {
        cerr << RED "Error: " << err.what() << RESET << endl;
    }
    for (size_t i = 0; i < servs.size(); i++)
        delete servs[i];
    for (size_t i = 0; i < socks.size(); i++)
        delete socks[i];
}