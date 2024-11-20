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

// TODO: set a max events
typedef struct addrinfo addrinfo;
typedef struct sockaddr_in sockaddr_in;
typedef struct pollfd pollfd;
typedef struct sockaddr sockaddr;
typedef struct sockaddr_storage sockaddr_storage;
typedef struct Server Server;
typedef struct timeval timeval;

#define LISTEN_LEN SOMAXCONN
#define BUFFSIZE 1024
#define MAXLEN 4096
#define END "\r\n"
#define HEADEREND "\r\n\r\n"
#define SOCK_TIMEOUT 2

#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define RESET "\033[0m"

struct
{
    std::string type;
    std::string ext;
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
    {"", ""}};

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

struct Server
{
    std::map<std::string, std::string> Data;
    size_t port;
    std::map<long, std::string> errors;
    std::map<std::string, bool> methods;
    size_t limit;
    std::map<std::string, Server *> children;

    Server()
    {
        std::cout << "call constractor" << std::endl;
        port = 0;
        limit = 0;
        Data["NAME"] = Data["LISTEN"] = Data["ROOT"] =
            Data["INDEX"] = Data["UPLOAD"] = Data["LOCATION"] = "";
        methods["GET"] = methods["POST"] = methods["DELETE"] = false;
    }

    ~Server()
    {
        std::cout << "call destractor" << std::endl;
        std::map<std::string, Server *>::iterator it;
        for (it = children.begin(); it != children.end(); it++)
            delete it->second;

        Data.clear();
        errors.clear();
        children.clear();
        methods.clear();
    }
};

struct Socket
{
    Action action;
    Method method;
    Type type;

    size_t full_len;
    time_t timeout;
    std::string buff;
    Server *server;

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
        type = type_;
        fd = fd_;
        if (type == CLIENT_)
            std::cout << "client has server: " << server->Data["NAME"] << ":" << server->port << std::endl;
    }
};

int init_socket(size_t port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        throw std::string("socket: ");
    int option = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)))
        throw std::string("setsockopt: ");

    sockaddr_in addr = (sockaddr_in){0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(fd, (sockaddr *)&addr, sizeof(addr)))
        throw std::string("bind: ");
    if (listen(fd, LISTEN_LEN))
        throw std::string("listen: ");
    return fd;
}

// CONFIG FILE PARSING
std::vector<std::string> tokens;
void Tokenize(std::string &text)
{
    int i = 0;
    while (text[i])
    {
        if (text[i] == ' ')
        {
            i++;
            continue;
        }
        if (text[i] && std::strchr("#{}\n", text[i]))
        {
            tokens.push_back(text.substr(i, 1));
            i++;
            continue;
        }
        int j = i;
        while (text[i] && !std::strchr("#{} \n", text[i]))
            i++;
        if (i > j)
            tokens.push_back(text.substr(j, i - j));
    }
}

bool isAllDigits(const std::string &str)
{
    for (size_t i = 0; i < str.length(); i++)
        if (!isdigit(str[i]))
            return false;
    return true;
}

std::vector<std::string> splitString(std::string &str, char delimiter)
{
    std::vector<std::string> vec;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos)
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
    std::string keyword;
    if (tokens[tk_pos] != std::string("{"))
    {
        delete serv;
        throw std::string("Expected '{");
    }
    tk_pos++;
    while (tk_pos < tokens.size() && tokens[tk_pos] != "}")
    {
        if (tokens[tk_pos] == std::string("\n"))
        {
            tk_pos++;
            continue;
        }
        if (tokens[tk_pos] == std::string("#"))
        {
            while (tokens[tk_pos] != std::string("\n") && tk_pos < tokens.size())
                tk_pos++;
            continue;
        }
        std::map<std::string, std::string>::iterator it;
        // TODO: check if hostname doesn't exists, throw error
        for (it = serv->Data.begin(); it != serv->Data.end(); ++it)
        {
            if (it->first == tokens[tk_pos])
            {
                tk_pos++;
                // TODO: protect it
                // TODO: test with key and no value !!!!!!
                if (tokens[tk_pos] == "\n" || tokens[tk_pos] == "#" || tokens[tk_pos] == "{")
                {
                    delete serv;
                    throw std::string("Invalid UPLOAD folder");
                }
                serv->Data[it->first] = tokens[tk_pos++];
                if (it->first == std::string("LISTEN"))
                {
                    std::vector<std::string> vec = splitString(serv->Data[it->first], ':');
                    if (vec.size() > 2)
                    {
                        delete serv;
                        throw std::string("Invalid hostname");
                    }
                    if (vec.size() == 2)
                    {
                        if (!isAllDigits(vec[1]))
                        {
                            delete serv;
                            throw std::string("Invalid port");
                        }
                        serv->port = std::atol(vec[1].c_str());
                        serv->Data["LISTEN"] = vec[0];
                    }
                }
                break;
            }
        }
        if (it != serv->Data.end())
            continue;
        if (tokens[tk_pos] == std::string("METHODS"))
        {
            tk_pos++;
            while (tokens[tk_pos] != std::string("\n") && tk_pos < tokens.size())
            {
                if (tokens[tk_pos] == std::string("POST") || tokens[tk_pos] == std::string("GET") ||
                    tokens[tk_pos] == std::string("DELETE"))
                    serv->methods[tokens[tk_pos]] = true;
                else
                {
                    delete serv;
                    throw std::string("Invalid method ") + tokens[tk_pos];
                }
                tk_pos++;
            }
            continue;
        }
        if (tokens[tk_pos] == std::string("ERRORS"))
        {
            tk_pos++;
            if (tokens[tk_pos] != std::string("{"))
            {
                delete serv;
                throw std::string("Expected '{'");
            }
            tk_pos++;
            while (tk_pos < tokens.size() && tokens[tk_pos] != std::string("}"))
            {
                if (tokens[tk_pos] == std::string("\n"))
                {
                    tk_pos++;
                    continue;
                }
                else if (isAllDigits(tokens[tk_pos]))
                {
                    long status = std::atol(tokens[tk_pos].c_str());
                    if (status < 400 || status > 499)
                    {
                        delete serv;
                        throw std::string("Invalid error status");
                    }
                    tk_pos++;
                    serv->errors[status] = tokens[tk_pos++];
                    continue;
                }
                else
                {
                    delete serv;
                    throw std::string("Unexpected ") + tokens[tk_pos];
                }
            }
            if (tokens[tk_pos] != std::string("}"))
            {
                delete serv;
                throw std::string("Expected '}'");
            }
            tk_pos++;
            continue;
        }
        if (tokens[tk_pos] == std::string("{"))
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
            throw std::string("Unexpected ") + tokens[tk_pos];
        }
    }
    if (tokens[tk_pos] != std::string("}"))
    {
        delete serv;
        throw std::string("Expected '}");
    }
    tk_pos++;
    return serv;
}

bool ends_with(std::string value, std::string ending)
{
    if (ending.size() > value.size())
        return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void check_server(Server *serv, std::string key)
{
    if (serv->Data[key] == "")
        serv->Data[key] = ".";
    if (!ends_with(serv->Data[key], std::string("/")))
        serv->Data[key] += "/";
    if (serv->children.size())
    {
        std::map<std::string, Server *>::iterator it;
        for (it = serv->children.begin(); it != serv->children.end(); it++)
        {
            it->second->Data[key] = serv->Data[key] + it->second->Data[key];
            check_server(it->second, key);
        }
    }
}

void pserver(Server *serv, int space)
{
    if (serv == NULL)
        throw std::string("serv in NULL");
    std::map<std::string, std::string>::iterator it1;
    for (it1 = serv->Data.begin(); it1 != serv->Data.end(); it1++)
    {
        if (it1->second.length())
            std::cout << std::setw(space) << it1->first << " : <" << it1->second << ">" << std::endl;
    }
    if (serv->methods.size())
    {
        std::cout << std::setw(space + 3) << "METHODS : ";
        std::map<std::string, bool>::iterator it2;
        for (it2 = serv->methods.begin(); it2 != serv->methods.end(); it2++)
        {
            if (it2->second)
                std::cout << it2->first << ", ";
        }
        std::cout << std::endl;
    }
    if (serv->errors.size())
    {
        std::cout << std::setw(space + 3) << "ERRORS : " << std::endl;
        std::map<long, std::string>::iterator it3;
        for (it3 = serv->errors.begin(); it3 != serv->errors.end(); it3++)
        {
            if (it3->second.length())
                std::cout << std::setw(space + 6) << it3->first << " : <" << it3->second << ">" << std::endl;
        }
    }
    std::cout << std::setw(space + 3) << "PORT : " << serv->port << "\n\n";
    std::map<std::string, Server *>::iterator it3;
    for (it3 = serv->children.begin(); it3 != serv->children.end(); it3++)
        pserver(serv->children[it3->first], space + 8);
}

// SERVER LISTENNING
void set_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        throw std::string("fcntl");
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0)
        throw std::string("fcntl");
}

std::string fetch_config(std::string key, std::string path, Socket *sock)
{
    // std::cout << "fetch_config: <" << path << "> from " << sock->server->Data["NAME"]
    //           << " has location: " << sock->server->Data["LOCATION"] << std::endl;
    if (path == "")
    {
        // TODO: get the index
    }
    std::string res = sock->server->Data[key] + std::string("/") + std::string(path);
    return res;
}

std::vector<pollfd> pfds;
std::string text;
std::map<std::string, std::string> ext_to_mem;
std::map<std::string, std::string> mem_to_ext;
std::map<int, Socket *> socks;
std::map<int, Socket *> pairs;

/*
TODO:
    + exits only if error in config file
    + parse servers
    + start the servers on PORT
    + read in current buff
    + write from pair buff
TODO:
    + protect multiple servers on same port
    + bind fail in invalid port fix it
    + close everything on exit
    + error in duplicated value in config file
    + each scoop must have a location, if not throw an error in parsing
    + throw error in  duplicated locations in childs
*/

int main(int argc, char **argv)
{
    std::vector<Server *> servs;

    for (int i = 0; exts_struct[i].type.length(); ++i)
    {
        mem_to_ext[exts_struct[i].type] = exts_struct[i].ext;
        ext_to_mem[exts_struct[i].ext] = exts_struct[i].type;
    }

    // std::map<std::string, std::string>::iterator it;
    // for (it = ext_to_mem.begin(); it != ext_to_mem.end(); ++it)
    //     std::cout << it->first << " => " << it->second << std::endl;
    // exit(1);
    try
    {
        if (argc != 2)
            throw std::string("Invalid argument");
        std::ifstream file(argv[1]);
        if (!file.is_open())
            throw std::string("Failed to open file");
        text = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        Tokenize(text);
        while (tk_pos < tokens.size())
        {
            if (tokens[tk_pos] == std::string("\n"))
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
            Socket *sock = new Socket(init_socket(servs[i]->port), servs[i], SERVER_);
            socks[sock->fd] = sock;
            pfds.push_back((pollfd){.fd = sock->fd, .events = POLLRDNORM});
            i++;
        }
        // TODO: handle HOST is required
        while (1)
        {
            int ready = poll(pfds.data(), pfds.size(), -1);
            if (ready > 0)
            {
                for (size_t i = 0; i < pfds.size(); i++)
                {
                    int curr_fd = pfds[i].fd;
                    Socket *sock = socks[curr_fd];
                    if (sock)
                    {
                        // std::cout << "loop" << std::endl;
                        if (sock->type == SERVER_ && pfds[i].revents)
                        {
                            sockaddr_storage client_addr;
                            socklen_t client_socket_len = sizeof(client_addr);
                            int client = accept(curr_fd, (sockaddr *)&client_addr, &client_socket_len);
                            if (client < 0)
                            {
                                // throw std::string("accept");
                            }
                            else if (client > 0)
                            {
                                set_non_blocking(client);
                                pfds.push_back((pollfd){.fd = client, .events = POLLIN | POLLOUT});
                                socks[client] = new Socket(client, sock->server, CLIENT_);
                                std::cout << "New connection from " << client << " to: " << curr_fd << std::endl;
                            }
                        }
                        else if (pfds[i].revents & POLLIN && sock->action == READ_)
                        {
                            // std::cout << "EPOLLIN" << std::endl;
                            char buff[BUFFSIZE + 1];
                            memset(buff, 0, sizeof(buff));
                            ssize_t r = read(pfds[i].fd, buff, BUFFSIZE);
                            if (r > 0)
                            {
                                sock->buff.append(buff, r);
                                if (sock->type == CLIENT_)
                                {
                                    if (sock->method == POST_)
                                    {
                                        // sock->action = WRITE_;
                                        // std::cout << "change action to write" << std::endl;
                                        // exit(1);
                                    }
                                    size_t pos = sock->buff.find(END, 0, strlen(END));
                                    while (pos != std::string::npos && sock->buff.length() && sock->action == READ_)
                                    {
                                        if (!sock->method)
                                        {
                                            // std::string to_find = "GET ";
                                            size_t rpos;
                                            if ((rpos = sock->buff.rfind(std::string("GET "), 0)) != std::string::npos)
                                            {
                                                sock->method = GET_;
                                                rpos += std::string("GET ").length();
                                                while (rpos < sock->buff.length() && sock->buff[rpos] == ' ')
                                                    rpos++;
                                                size_t s = rpos;
                                                while (rpos < sock->buff.length() && sock->buff[rpos] != ' ')
                                                    rpos++;
                                                std::string filename = sock->buff.substr(s, rpos - s);
                                                if (filename == "")
                                                    throw std::string("Invalid request 1");
                                                while (rpos < sock->buff.length() && sock->buff[rpos] == ' ')
                                                    rpos++;
                                                if (rpos == sock->buff.length())
                                                    throw std::string("Invalid request 2");
                                                s = rpos;

                                                if (sock->buff.substr(s, pos - s) != "HTTP/1.1")
                                                    throw std::string("Invalid request 4");
                                                sock->buff = "";
                                                // TODO: check it
                                                filename = fetch_config("LOCATION", filename, sock);
                                                std::cout << "filename: " << filename << std::endl;
                                                int fd = open(filename.c_str(), O_RDONLY, 0777);
                                                if (fd < 0)
                                                {
                                                    sock->is_error = true;
                                                    std::stringstream content_len;
                                                    content_len << std::string("404 File Not Found").length();
                                                    sock->buff = "HTTP/1.0 404 Not Found\r\n"
                                                                 "Content-Type: text/plain\r\n"
                                                                 "Content-Length: " +
                                                                 content_len.str() + "\r\n\r\n" +
                                                                 "404 File Not Found";
                                                    sock->full_len = sock->buff.length();
                                                    sock->action = WRITE_;
                                                }
                                                else
                                                {
                                                    std::string file_ext_str = filename.substr(filename.find_last_of("."));
                                                    std::cout << "has extension: " << file_ext_str << ", Content-type: " << ext_to_mem[file_ext_str] << std::endl;
                                                    if (!ext_to_mem[file_ext_str].length())
                                                        throw std::string("Invalid extention");
                                                    std::string content_type = ext_to_mem[file_ext_str];
                                                    // get file size
                                                    struct stat state;
                                                    fstat(fd, &state);
                                                    std::stringstream content_len;
                                                    content_len << state.st_size;

                                                    pfds.push_back((pollfd){.fd = fd, .events = POLLIN | POLLOUT});
                                                    socks[fd] = new Socket(fd, NULL, FILE_);
                                                    socks[fd]->buff = "HTTP/1.0 200 OK\r\n"
                                                                      "Content-Type: " +
                                                                      content_type +
                                                                      "\r\nContent-Length: " +
                                                                      content_len.str() + "\r\n\r\n";
                                                    socks[fd]->full_len = state.st_size + socks[fd]->buff.length();
                                                    std::cout << "has full len: <" << socks[fd]->full_len << ">" << std::endl;
                                                    // set pairs
                                                    pairs[fd] = sock;
                                                    pairs[sock->fd] = socks[fd];

                                                    sock->action = WRITE_;
                                                    socks[fd]->action = READ_;
                                                }
                                            }
                                            else if ((rpos = sock->buff.rfind(std::string("POST "), 0)) != std::string::npos)
                                            {
                                                sock->method = POST_;
                                                rpos += strlen("POST ");
                                                while (sock->buff[rpos] == ' ')
                                                    rpos++;
                                                while (sock->buff[rpos] != ' ')
                                                    rpos++;
                                                while (sock->buff[rpos] == ' ')
                                                    rpos++;
                                                std::string http_version = sock->buff.substr(rpos, pos - rpos);
                                                if (http_version != "HTTP/1.1")
                                                {
                                                    sock->is_error = true;
                                                    std::stringstream content_len;
                                                    content_len << std::string("400 bad request").length();
                                                    sock->buff = "HTTP/1.0 400 Not Found\r\n"
                                                                 "Content-Type: text/plain\r\n"
                                                                 "Content-Length: " +
                                                                 content_len.str() + "\r\n\r\n" +
                                                                 "400 bad request";
                                                    sock->full_len = sock->buff.length();
                                                    // sock->action = WRITE_;
                                                    // throw std::string("testing");
                                                }
                                                else
                                                {
                                                    rpos = sock->buff.rfind("Content-Type: ");
                                                    if (rpos == std::string::npos)
                                                    {
                                                        sock->is_error = true;
                                                        std::stringstream content_len;
                                                        content_len << std::string("400 bad request").length();
                                                        sock->buff = "HTTP/1.0 400 Not Found\r\n"
                                                                     "Content-Type: text/plain\r\n"
                                                                     "Content-Length: " +
                                                                     content_len.str() + "\r\n\r\n" +
                                                                     "400 bad request";
                                                        sock->full_len = sock->buff.length();
                                                        // sock->action = WRITE_;
                                                        // throw std::string("testing");
                                                    }
                                                    else
                                                    {
                                                        // TODO: protect it from spaces, repeated key,
                                                        std::string ctype = sock->buff.substr(rpos + strlen("Content-Type: "), std::string::npos);
                                                        rpos = ctype.rfind(END);
                                                        ctype = ctype.substr(0, rpos - strlen(END));

                                                        if (ext_to_mem[ctype].length() == 0)
                                                        {
                                                            sock->is_error = true;
                                                            std::stringstream content_len;
                                                            content_len << std::string("400 bad request").length();
                                                            sock->buff = "HTTP/1.0 400 Not Found\r\n"
                                                                         "Content-Type: text/plain\r\n"
                                                                         "Content-Length: " +
                                                                         content_len.str() + "\r\n\r\n" +
                                                                         "400 bad request";
                                                            sock->full_len = sock->buff.length();
                                                            // throw std::string("testing");
                                                            sock->action = WRITE_;
                                                        }
                                                        else
                                                        {
                                                            timeval current_time;
                                                            gettimeofday(&current_time, NULL);
                                                            localtime(&current_time.tv_sec);
                                                            std::stringstream filename_stream;
                                                            filename_stream << current_time.tv_sec;
                                                            std::string filename = fetch_config("UPLOAD", filename_stream.str() + ext_to_mem[ctype], sock);
                                                            int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
                                                            if (fd < 0)
                                                                ; // internal error
                                                            pfds.push_back((pollfd){.fd = fd, .events = POLLIN | POLLOUT});
                                                            socks[fd] = new Socket(fd, NULL, FILE_);
                                                            pairs[fd] = sock;
                                                            pairs[sock->fd] = socks[fd];
                                                            // the new file will be the pair

                                                            socks[fd]->action = READ_;
                                                            // socks[fd]->method = POST_;
                                                            sock->action = WRITE_;

                                                            rpos = sock->buff.rfind("Content-Length: ");
                                                            std::string clen_str = sock->buff.substr(rpos + strlen("Content-Length: "), std::string::npos);
                                                            rpos = clen_str.rfind(END);
                                                            clen_str = clen_str.substr(0, rpos - strlen(END));
                                                            size_t clen = std::atol(clen_str.c_str());

                                                            rpos = sock->buff.rfind(HEADEREND);
                                                            sock->buff = sock->buff.substr(rpos + strlen(HEADEREND), sock->buff.length());
                                                            sock->full_len = clen;
                                                        }
                                                    }
                                                }
                                            }
                                            else
                                                throw std::string("Invalid request 0");
                                        }
                                        pos = sock->buff.find(END, pos, strlen(END));
                                    }
                                }
                                else if (sock->type == FILE_)
                                {
                                    // std::cout << "file has content <" << sock->buff << ">" << std::endl;
                                }
                            }
                        }
                        else if (pfds[i].revents & POLLOUT && sock->action == WRITE_)
                        {
                            // std::cout << "POLLOUT " << std::endl;
                            if (sock->method == GET_)
                            {
                                if (sock->is_error)
                                {
                                    if (send(sock->fd, sock->buff.c_str(), sock->buff.length(), 0) < 0)
                                        throw std::string("sending 0");
                                    sock->buff = "";
                                    sock->full_len = 0;
                                    sock->action = READ_;
                                    exit(1);
                                }
                                else
                                {
                                    Socket *src = pairs[sock->fd];
                                    if (src->buff.length())
                                    {
                                        ssize_t bytes = src->buff.length() > BUFFSIZE ? BUFFSIZE : src->buff.length();
                                        std::cout << "send " << bytes << " from " << src->buff.length() << std::endl;
                                        bytes = write(sock->fd, src->buff.c_str(), bytes);
                                        if (bytes < 0)
                                            throw std::string("sending 1");
                                        src->full_len -= bytes; // TODO: protect if writting less than bytes
                                        src->buff = src->buff.substr(bytes, src->buff.length());
                                    }
                                    if (src->full_len == 0)
                                    {
                                        close(src->fd);
                                        sock->action = READ_;
                                        std::cout << "close file" << std::endl;
                                        // exit(1);
                                    }
                                }
                            }
                            else if (sock->method == POST_)
                            {
                                Socket *src = pairs[sock->fd];
                                if (sock->type == FILE_ && src->buff.length())
                                {
                                    if (src->buff.length())
                                    {
                                        ssize_t bytes = src->buff.length() > BUFFSIZE ? BUFFSIZE : src->buff.length();

                                        src->full_len -= bytes; // TODO: protect if writting less than bytes
                                        bytes = write(sock->fd, src->buff.c_str(), bytes);
                                        if (bytes < 0)
                                            throw std::string("write");
                                        src->buff = src->buff.substr(bytes, src->buff.length());
                                        src->action = READ_;
                                        // if (src->buff.length() == 0)
                                        // {
                                        //     std::cout << "change action in POST to READ" << std::endl;
                                        //     // exit(1);
                                        //     src->action = READ_;
                                        // }
                                    }
                                    if (src->full_len == 0)
                                    {
                                        close(src->fd); // TODO: to be removed
                                        socks[src->fd] = NULL;

                                        close(sock->fd);
                                        src->action = READ_;
                                        // delete socks[sock->fd];
                                        socks[sock->fd] = NULL;
                                        std::cout << "close external file" << std::endl;
                                        // exit(1);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    catch (std::string &err)
    {
        std::cerr << RED << "Error: " << err << RESET << std::endl;
    }
    // exitting
    size_t i = 0;
    while (i < servs.size())
        delete servs[i++];
    servs.clear();
    i = 0;
    while (i < socks.size())
        delete socks[i++];
    std::cout << "Exiting the server" << std::endl;
}