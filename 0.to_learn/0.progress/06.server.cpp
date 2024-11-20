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

// TODO: set a max events
typedef struct addrinfo addrinfo;
typedef struct sockaddr_in sockaddr_in;
typedef struct pollfd pollfd;
typedef struct sockaddr sockaddr;
typedef struct sockaddr_storage sockaddr_storage;
typedef struct Server Server;
typedef struct Socket Socket;
typedef struct timeval timeval;
using namespace std;

#if 0
#define CLIENT_TIMEOUT 10
#endif

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
    size_t port;
    /*
    TODO:
        + check negative port value
        + port should be required
    */
    map<long, string> errors;
    map<string, bool> methods;
    ssize_t limit;
    map<string, Server *> children;

    // constractor
    Server()
    {
        cout << "call constractor" << endl;
        port = 0;
        limit = 0;
        Data["NAME"] = Data["LISTEN"] = Data["LOCATION"] = "";
        Data["INDEX"] = Data["UPLOAD"] = Data["ROOT"] = "";
        methods["GET"] = methods["POST"] = methods["DELETE"] = false;
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
        methods.clear();
    }
};

map<int, Socket *> srcs;
map<int, Socket *> dests;

map<int, Socket *> socks;
vector<pollfd> pfds;
struct Socket
{
    Action action;
    Method method;
    Type type;

    string fname;
    ssize_t full_len;
    time_t timeout;
    // string rdbuff;
    // string rsbuff;
    string buff;

    Server *server;
    bool ready;
    Socket *pair;

    // string ctype;
    // string clen;

    bool is_error;
    size_t error_status;
    int fd;

    Socket(int fd_, Server *server_, Type type_)
    {
        action = READ_;
        timeout = 0;
        method = (Method)0;
        server = server_;
        is_error = false;
        error_status = 0;
        pair = NULL;
        type = type_;
        ready = false;
        fd = fd_;
        timeout = time(NULL);

        if (type == CLIENT_)
            cout << "client has server: " << server->Data["NAME"] << ":" << server->port << endl;
    }
    void set_non_blocking()
    {
        int flags = fcntl(fd, F_GETFL, 0);
        assert(flags != -1);
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
    bool buff_starts_with(string value)
    {
        return buff.compare(0, value.length(), value, 0, value.length()) == 0;
    }
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

        // socks.erase(fd);
        socks[fd] = NULL;
        cout << "pfds size " << pfds.size() << endl;
    }
};

// CONFIG FILE PARSING
/*
    TODO:
        + handle values inside quotes
        + check if hostname doesn't exists, throw error
        + test with key and no value !!!!!!


*/
vector<string> tokens;
void Tokenize(string &text)
{
    int i = 0;
    while (text[i])
    {
        if (text[i] == ' ')
        {
            i++;
            continue;
        }
        if (text[i] && strchr("#{}\n", text[i]))
        {
            tokens.push_back(text.substr(i, 1));
            i++;
            continue;
        }
        int j = i;
        while (text[i] && !strchr("#{} \n", text[i]))
            i++;
        if (i > j)
            tokens.push_back(text.substr(j, i - j));
    }
}

bool isAllDigits(const string &str)
{
    for (size_t i = 0; i < str.length(); i++)
        if (!isdigit(str[i]))
            return false;
    return true;
}

vector<string> splitString(string &str, char delimiter)
{
    vector<string> vec;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != string::npos)
    {
        vec.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }
    vec.push_back(str.substr(start));
    return vec;
}

int tk_pos = 0;
Server *parse_config_file()
{
    Server *serv = new Server();
    string keyword;
    if (tokens[tk_pos] != string("{"))
    {
        delete serv;
        throw Error(string("Expected '{'\n"));
    }
    tk_pos++;
    while (tk_pos < tokens.size() && tokens[tk_pos] != "}")
    {
        if (tokens[tk_pos] == string("\n"))
        {
            tk_pos++;
            continue;
        }
        if (tokens[tk_pos] == string("#"))
        {
            while (tokens[tk_pos] != string("\n") && tk_pos < tokens.size())
                tk_pos++;
            continue;
        }
        map<string, string>::iterator it;
        for (it = serv->Data.begin(); it != serv->Data.end(); ++it)
        {
            if (it->first == tokens[tk_pos])
            {
                tk_pos++;
                if (tokens[tk_pos] == "\n" || tokens[tk_pos] == "#" || tokens[tk_pos] == "{")
                {
                    delete serv;
                    throw Error(string("Invalid UPLOAD folder\n"));
                }
                serv->Data[it->first] = tokens[tk_pos++];
                if (it->first == string("LISTEN"))
                {
                    vector<string> vec = splitString(serv->Data[it->first], ':');
                    if (vec.size() > 2)
                    {
                        delete serv;
                        throw Error(string("Invalid hostname\n"));
                    }
                    if (vec.size() == 2)
                    {
                        if (!isAllDigits(vec[1]))
                        {
                            delete serv;
                            throw Error(string("Invalid port\n"));
                        }
                        serv->port = atol(vec[1].c_str());
                        serv->Data["LISTEN"] = vec[0];
                    }
                }
                break;
            }
        }
        if (it != serv->Data.end())
            continue;
        if (tokens[tk_pos] == string("METHODS"))
        {
            tk_pos++;
            while (tokens[tk_pos] != string("\n") && tk_pos < tokens.size())
            {
                if (tokens[tk_pos] == string("POST") || tokens[tk_pos] == string("GET") ||
                    tokens[tk_pos] == string("DELETE"))
                    serv->methods[tokens[tk_pos]] = true;
                else
                {
                    delete serv;
                    throw Error(("Invalid method " + tokens[tk_pos] + "\n").c_str());
                }
                tk_pos++;
            }
            continue;
        }
        if (tokens[tk_pos] == string("ERRORS"))
        {
            tk_pos++;
            if (tokens[tk_pos] != string("{"))
            {
                delete serv;
                throw Error(string("Expected '{'\n"));
            }
            tk_pos++;
            while (tk_pos < tokens.size() && tokens[tk_pos] != string("}"))
            {
                if (tokens[tk_pos] == string("\n"))
                {
                    tk_pos++;
                    continue;
                }
                else if (isAllDigits(tokens[tk_pos]))
                {
                    long status = atol(tokens[tk_pos].c_str());
                    if (status < 400 || status > 499)
                    {
                        delete serv;
                        throw Error(string("Invalid error status\n"));
                    }
                    tk_pos++;
                    serv->errors[status] = tokens[tk_pos++];
                    continue;
                }
                else
                {
                    delete serv;
                    throw Error(("Unexpected " + tokens[tk_pos] + "\n").c_str());
                }
            }
            if (tokens[tk_pos] != string("}"))
            {
                delete serv;
                throw Error(string("Expected '}'\n"));
            }
            tk_pos++;
            continue;
        }
        if (tokens[tk_pos] == string("{"))
        {
            try
            {
                Server *child = parse_config_file();
                serv->children[child->Data["LOCATION"]] = child;
            }
            catch (...)
            {
                delete serv;
                throw;
            }
            continue;
        }
        if (tk_pos < tokens.size())
        {
            delete serv;
            throw Error(("Unexpected " + tokens[tk_pos] + "\n").c_str());
        }
    }
    if (tokens[tk_pos] != string("}"))
    {
        delete serv;
        throw Error(string("Expected '}'\n"));
    }
    tk_pos++;
    return serv;
}

bool ends_with(string value, string ending)
{
    if (ending.size() > value.size())
        return false;
    return equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void check_server(Server *serv, string key)
{
    // TODO: if there is a root add it to upload and location path
    cout << "server has port " << serv->port << endl;
    if (serv->Data[key] == "")
        serv->Data[key] = ".";
    if (!ends_with(serv->Data[key], string("/")))
        serv->Data[key] += "/";
    if (serv->children.size())
    {
        map<string, Server *>::iterator it;
        for (it = serv->children.begin(); it != serv->children.end(); it++)
        {
            if ((key == "LOCATION" || key == "UPLOAD") && it->second->Data[key][0] == '.')
                it->second->Data[key] = serv->Data[key] + it->second->Data[key].substr(0, it->second->Data[key].length());
            else if (it->second->Data[key] == "")
                it->second->Data[key] = serv->Data[key];
            check_server(it->second, key);
            if (it->second->port == 0)
                it->second->port = serv->port;
        }
    }
}

// DEBUGING
void pserver(Server *serv, int space)
{
    if (serv == NULL)
        throw Error(string("serv in NULL\n"));
    map<string, string>::iterator it1;
    for (it1 = serv->Data.begin(); it1 != serv->Data.end(); it1++)
    {
        if (it1->second.length())
            cout << setw(space) << it1->first << " : <" << it1->second << ">" << endl;
    }
    if (serv->methods.size())
    {
        cout << setw(space + 3) << "METHODS : ";
        map<string, bool>::iterator it2;
        for (it2 = serv->methods.begin(); it2 != serv->methods.end(); it2++)
        {
            if (it2->second)
                cout << it2->first << ", ";
        }
        cout << endl;
    }
    if (serv->errors.size())
    {
        cout << setw(space + 3) << "ERRORS : " << endl;
        map<long, string>::iterator it3;
        for (it3 = serv->errors.begin(); it3 != serv->errors.end(); it3++)
        {
            if (it3->second.length())
                cout << setw(space + 6) << it3->first << " : <" << it3->second << ">" << endl;
        }
    }
    cout << setw(space + 3) << "PORT : " << serv->port << "\n\n";
    map<string, Server *>::iterator it3;
    for (it3 = serv->children.begin(); it3 != serv->children.end(); it3++)
        pserver(serv->children[it3->first], space + 8);
}

// SERVER LESTENNING
string fetch_config(string key, Socket *sock)
{
    // if (path == "")
    // {
    //     // TODO: get the index file
    // }
    string res = sock->server->Data[key] + sock->fname;
    return res;
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

size_t get_file_size(int fd)
{
    struct stat state;
    fstat(fd, &state);
    return state.st_size;
}

string size_to_string(size_t size)
{
    stringstream stream;
    stream << size;
    return stream.str();
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
    + test repeated key (Transfer-Encoding multiple times in header)
+ CMD:
    + ls /proc/$(ps -aux | grep "a.out" | awk 'NR==1{print $2}')/fd | wc -w
*/
string text;
map<string, string> memetype;

void add_socket(int fd, Server *serv, Type type, int pair_fd)
{
    if (socks[fd])
        delete socks[fd];
    pfds.push_back((pollfd){.fd = fd, .events = POLLIN | POLLOUT});
    socks[fd] = new Socket(fd, serv, type);
    if (type == CLIENT_)
        socks[fd]->set_non_blocking();
    else if (type == FILE_)
    {
        socks[fd]->pair = socks[pair_fd];
        socks[pair_fd]->pair = socks[fd];
    }
}

int main(int argc, char **argv)
{
    vector<Server *> servs;
    try
    {
        for (int i = 0; exts_struct[i].type.length(); ++i)
        {
            memetype[exts_struct[i].type] = exts_struct[i].ext;
            memetype[exts_struct[i].ext] = exts_struct[i].type;
        }
        if (argc != 2)
            throw Error(string("Invalid argument\n"));
        ifstream file(argv[1]);
        if (!file.is_open())
            throw Error(string("Failed to open file\n"));
        text = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();
        Tokenize(text);
        while (tk_pos < tokens.size())
        {
            if (tokens[tk_pos] == string("\n"))
            {
                tk_pos++;
                continue;
            }
            Server *res = parse_config_file();
            check_server(res, "LOCATION");
            check_server(res, "UPLOAD");
            check_server(res, "LISTEN");
            servs.push_back(res);
        }
        int i = 0;
        while (i < servs.size())
        {
            pserver(servs[i], 10);
            Socket *sock = new Socket(servs[i]->get_fd(), servs[i], SERVER_);
            socks[sock->fd] = sock;
            pfds.push_back((pollfd){.fd = sock->fd, .events = POLLRDNORM});
            i++;
        }
        struct sigaction sa = {};
        sa.sa_handler = SIG_IGN;
        sigaction(SIGPIPE, &sa, 0);

        ssize_t timing = 0;
        cout << "pfds size " << pfds.size() << endl;
        while (1)
        {
            timing++;
            if (timing % 10000000 == 0)
            {
                timing = 0;
                cout << "pfds size " << pfds.size() << endl;
            }
            int ready = poll(pfds.data(), pfds.size(), -1);
            if (ready < 0)
                throw Error(string("random error 0\n"));
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
                        delete sock->pair;
                        delete sock;
                        // continue;
                    }
                    // ACCEPT CONNECTION
                    else if (sock->type == SERVER_ && pfds[i].revents)
                    {
                        sockaddr_storage client_addr;
                        socklen_t client_socket_len = sizeof(client_addr);
                        int client = accept(pfds[i].fd, (sockaddr *)&client_addr, &client_socket_len);
                        assert(client > 0);
                        if (client < 0)
                        {
                            // throw Error(string("accept\n"));
                            continue;
                        }
                        else if (client > 0)
                        {
                            add_socket(client, sock->server, CLIENT_, 0);
                            cout << "New connection from " << client << " to: " << pfds[i].fd << endl;
                        }
                    }
                    else if (pfds[i].revents & POLLIN && sock->action == READ_
                             /*&& sock->buff.length() < BUFFSIZE*/
                    )
                    {
                        char buff[BUFFSIZE + 1] = {0};
                        ssize_t r = read(pfds[i].fd, buff, BUFFSIZE);
                        cout << "read " << r << " bytes" << endl;
                        if (r < 0)
                            throw Error(string("reading\n"));
                        else if (r > 0)
                        {
                            sock->buff.append(buff, r);
                            sock->update_timeout();
                        }
                        if (r > 0 && sock->type == CLIENT_ && !sock->ready)
                        {
                            // cout << "read" << endl;
                            ssize_t pos = sock->buff.find(END, 0, strlen(END));
                            while (pos != string::npos)
                            {
                                if (!sock->method)
                                {
                                    ssize_t rpos;
                                    if ((rpos = sock->buff.rfind(string("GET "), 0)) != string::npos)
                                    {
                                        sock->method = GET_;
                                        rpos += std::string("GET ").length();
                                        ssize_t s = rpos;
                                        while (rpos < sock->buff.length() && sock->buff[rpos] != ' ')
                                            rpos++;
                                        if (s == rpos)
                                            throw Error(string("Invalid request 1\n"));
                                        sock->fname = sock->buff.substr(s, rpos - s);
                                        if (sock->buff.substr(rpos, pos - rpos) != " HTTP/1.1")
                                            throw Error(string("Invalid request 2\n"));
                                        // prepare the file for response
                                        ssize_t dot_pos = sock->fname.find_last_of(".");
                                        string ext = sock->fname.substr(dot_pos, sock->fname.length());
                                        // sock->ctype = memetype[ext];
                                        if (memetype[ext].length() == 0)
                                            throw Error(string("Invalid extention 3\n"));
                                        sock->fname = fetch_config("LOCATION", sock);
                                        cout << "filename <" << sock->fname << ">" << endl;
                                        int fd = open(sock->fname.c_str(), O_RDONLY, 0777);
                                        if (fd < 0)
                                        {
                                            /*
                                            TODO:
                                                + in GET : file not found
                                                + in POST: internal error
                                            */
                                            cout << "file not found " + sock->fname << endl;
                                            delete sock;
                                            throw Error("openning file ");
                                            continue;
                                        }
                                        else
                                            add_socket(fd, NULL, FILE_, sock->fd);
                                        sock->pair->full_len = get_file_size(sock->pair->fd);
                                        sock->pair->buff = "HTTP/1.0 200 OK\r\nContent-Type: " +
                                                           memetype[ext] + "\r\nContent-Length: " +
                                                           size_to_string(sock->pair->full_len) + "\r\n\r\n";
                                        sock->pair->full_len += sock->pair->buff.length();
                                        sock->pair->action = READ_;
                                        sock->action = WRITE_;
                                    }
                                    else if ((rpos = sock->buff.rfind(string("POST "), 0)) != string::npos)
                                    {
                                        sock->method = POST_;
                                        rpos += string("POST ").length();
                                        // TODO:; get location
                                        ssize_t s = rpos;
                                        while (rpos < sock->buff.length() && sock->buff[rpos] != ' ')
                                            rpos++;
                                        if (s == rpos)
                                            throw Error(string("Invalid request 4\n"));
                                    }
                                    else
                                        throw Error("bad request");
                                }
                                if (sock->buff_starts_with("Content-Length: "))
                                {
                                    // TODO: throw bad request if no content length or no content type
                                    cout << "found content len" << endl;
                                    string clen = sock->buff.substr(strlen("Content-Length: "), pos - strlen("Content-Length: "));
                                    cout << "clen <" << clen << ">" << endl;
                                    sock->full_len = strtol(clen.c_str(), NULL, 10);
                                    // TODO: check if len is valid
                                    cout << "full len " << sock->full_len << endl;
                                    // exit(1);
                                    // continue;
                                }
                                else if (sock->buff_starts_with("Content-Type: "))
                                {
                                    // TODO: throw bad reuqest if no contetn length or no content type
                                    cout << "found content type" << endl;
                                    string ctype = sock->buff.substr(strlen("Content-Type: "), pos - strlen("Content-Type: "));
                                    cout << "ctype <" << ctype << ">" << endl;
                                    ctype = memetype[ctype];
                                    if (ctype.length() == 0 || ctype[0] != '.') // protectiob becaude memtype contains
                                        throw Error("Invalid type");            // both memtype and extention
                                    sock->fname = rand_name() + ctype;
                                    sock->fname = fetch_config("UPLOAD", sock);
                                    cout << "filename " << sock->fname << endl;
                                    int fd = open(sock->fname.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
                                    if (fd < 0)
                                    {
                                        /*
                                        TODO:
                                            + in GET : file not found
                                            + in POST: internal error
                                        */
                                        cout << "file not found " + sock->fname << endl;
                                        delete sock;
                                        throw Error("openning file ");
                                        continue;
                                    }
                                    else
                                        add_socket(fd, NULL, FILE_, sock->fd);
                                    sock->pair->action = WRITE_;
                                    sock->pair->method = POST_;
                                    sock->action = READ_;
                                }
                                else if (sock->buff.compare(0, strlen(END), END, 0, strlen(END)) == 0)
                                {
                                    sock->buff = sock->buff.substr(pos + strlen(END), sock->buff.length());
                                    sock->ready = true;
                                    // protect in POST, if no content length or no content type
                                    break;
                                }
                                sock->buff = sock->buff.substr(pos + strlen(END), sock->buff.length());
                                pos = sock->buff.find(END, 0, strlen(END));
                            }
                            // exit(1);
                        }
                    }
                    else if (pfds[i].revents & POLLOUT && sock->action == WRITE_)
                    {
                        Socket *src = sock->pair;
                        if (sock->method == GET_ && src->buff.length())
                        {
                            ssize_t bytes = src->buff.length() > BUFFSIZE ? BUFFSIZE : src->buff.length();
                            bytes = write(sock->fd, src->buff.c_str(), bytes);
                            if (bytes <= 0)
                            {
                                delete sock;
                                delete src;
                                throw Error("write 0");
                                continue;
                            }
                            else if (bytes > 0)
                            {
                                src->buff = src->buff.substr(bytes, src->buff.length());
                                src->full_len -= bytes;
                                sock->update_timeout();
                                if (src->full_len <= 0)
                                {
                                    cout << "reached the end of file" << endl;
                                    delete sock;
                                    delete src;
                                    continue;
                                }
                            }
                        }
                        else if (sock->method == POST_ && sock->type == FILE_ && src->buff.length())
                        {
                            ssize_t bytes = src->buff.length() > BUFFSIZE ? BUFFSIZE : src->buff.length();
                            bytes = write(sock->fd, src->buff.c_str(), bytes);
                            cout << "write " << bytes << ", remaining " << src->full_len - bytes
                                 << ", src " << src->buff.length() << endl;
                            if (bytes < 0)
                            {
                                delete sock;
                                delete src;
                                throw Error("write 0");
                                continue;
                            }
                            else if (bytes > 0)
                            {
                                src->buff = src->buff.substr(bytes, src->buff.length());
                                src->full_len -= bytes;
                                sock->update_timeout();
                                if (src->full_len <= 0)
                                {
                                    cout << "reached the end of file" << endl;
                                    // exit(1);
                                    delete sock;
                                    delete src;
                                    continue;
                                }
                            }
                        }
                    }
                    i++;
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << RED "Error: " << e.what() << RESET << endl;
    }
    ssize_t i = 0;
    while (i < servs.size())
        delete servs[i++];
    servs.clear();
    i = 0;
    while (i < socks.size())
        delete socks[i++];
    cout << "Exiting the server" << endl;
}