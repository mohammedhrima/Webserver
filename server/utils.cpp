/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhrima <mhrima@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/07 16:53:16 by mhrima            #+#    #+#             */
/*   Updated: 2024/06/07 16:53:16 by mhrima           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.hpp"

string to_string(size_t size)
{
    stringstream stream;
    stream << size;
    return stream.str();
}

string to_string(HTTP version)
{
    if (version == HTTP1_0)
        return "HTTP/1.0";
    else if (version == HTTP1_1)
        return "HTTP/1.1";
    return "";
}

string to_string(Action action)
{
    if (action == READ_)
        return "READ";
    if (action == WRITE_)
        return "WRITE";
    return "";
}

string to_string(Method method)
{
    if (method == GET_)
        return "GET";
    else if (method == POST_)
        return "POST";
    else if (method == DELETE_)
        return "DELETE";
    else
        return "UNKNOWN (Method)";
}

string to_string(Status status)
{
    switch (status)
    {
    case HTTP_OK:
        return "OK";
    case HTTP_CREATED:
        return "Created";
    case HTTP_NO_CONTENT:
        return "No Content";
    case HTTP_MOVE_PERMANENTLY:
        return "Moved Permanently";
    case HTTP_TEMPORARY_REDIRECT:
        return "Temporary Redirect";
    case HTTP_PERMANENT_REDIRECT:
        return "Permanent Redirect";
    case HTTP_BAD_REQUEST:
        return "Bad Request";
    case HTTP_FORBIDEN:
        return "Forbidden";
    case HTTP_NOT_FOUND:
        return "Not Found";
    case HTTP_METHOD_NOT_ALLOWED:
        return "Method Not Allowed";
    case HTTP_TIMEOUT:
        return "Request Timeout";
    case HTTP_LENGTH_REQUIRED:
        return "Length Required";
    case HTTP_CONTENT_TO_LARGE:
        return "Content Too Large";
    case HTTP_URI_TO_LARGE:
        return "URI Too Long";
    case HTTP_HEADER_TO_LARGE:
        return "Request Header Fields Too Large";
    case HTTP_INTERNAL_SERVER:
        return "Internal Server Error";
    case HTTP_NOT_IMPLEMENTED:
        return "Not Implemented";
    case HTTP_BAD_GATEWAY:
        return "Bad Gateway";
    case HTTP_GATEWAY_TIMEOUT:
        return "Gateway Timeout";
    case HTTP_INSUPPORTED_HTTP:
        return "HTTP Version Not Supported";
    case HTTP_NONE:
        return "";
    }
    return "";
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

ssize_t find(string &str, char *to_find)
{
    return str.find(to_find, 0, strlen(to_find));
}

string substr(size_t line, string &str, ssize_t s, ssize_t e)
{
    try
    {
        return str.substr(s, e - s);
    }
    catch (...)
    {
        cerr << RED << "substract " + str << " line: " << to_string(line) << endl;
        cerr << "from " << to_string(s) << " to " << to_string(e) << RESET << endl;
        return "";
    }
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
            while (path[i] == '/' && i < path.length())
                i++;
        }
        else
            i++;
    }
    return res;
}

string _tolower(string str)
{
    string res;
    for (size_t i = 0; i < str.length(); i++)
        res += tolower(str[i]);
    return res;
}

string _toupper(string &str)
{
    string res;
    for (size_t i = 0; i < str.length(); i++)
        res += toupper(str[i]);
    return res;
}

bool _isdigits(string &str)
{
    for (size_t i = 0; i < str.length(); i++)
        if (!isdigit(str[i]))
            return false;
    return true;
}

bool _isaldigits(string &str)
{
    for (size_t i = 0; i < str.length(); i++)
        if (!isalpha(str[i]) && !isdigit(str[i]) && str[i] != '_')
            return false;
    return true;
}

State get_state(string &path)
{
    State state = {};
    stat(path.c_str(), &state);
    return state;
}

string parse_hexadecimal(string value)
{
    string res;
    size_t i = 0;

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

bool is_error(Status &status)
{
    return (status >= 400 && status <= 511);
}

bool is_move_dir(Status &status)
{
    return (status == HTTP_MOVE_PERMANENTLY || status == HTTP_TEMPORARY_REDIRECT || status == HTTP_PERMANENT_REDIRECT);
}

bool check_error(int line, Connection &con, bool cond, Status status, string cause)
{
    if (cond && !is_error(con.res.status))
    {
        RLOG("check error") << "found in line " << line << " cause: " << cause << END;
        con.res.cause = cause;
        con.res.status = status;
        con.action = WRITE_;
        con.req.header.conds["parsed"] = true;
        con.req.header.conds["body_checked"] = true; // to remove it test DELETE with negative content-len

        // if you want to uncomment it check POST chunked
        con.req.header.conds["response"] = false;

        if (con.res.cgi.pid > 0)
        {
            kill(con.res.cgi.pid, SIGKILL);
            con.res.cgi.pid = -1;
            if (con.res.cgi.input.name.length())
                remove(con.res.cgi.input.name.c_str());
            if (con.res.cgi.output.name.length())
                remove(con.res.cgi.output.name.c_str());
        }
    }
    return cond;
}

string trim(string str)
{
    size_t s = 0;
    size_t e = str.length();

    while (s < e && isspace(str[s]))
        s++;
    while (e > s && isspace(str[e - 1]))
        e--;
    return str.substr(s, e - s);
}

string generate_html(string msg)
{
    return (
        "<div style=\"display:flex; justify-content:center;"
        "align-items:center; color:blue; font-size: large;\">" +
        msg +
        "</div>\n");
}

string rand_name()
{
    // return "file";
    timeval current_time;
    gettimeofday(&current_time, NULL);
    localtime(&current_time.tv_sec);
    stringstream filename;
    filename << current_time.tv_sec;
    return filename.str();
}

// INIT WEBSERVER
void init_memetypes()
{
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

    for (size_t i = 0; exts_struct[i].type.length(); ++i)
    {
        memetype[exts_struct[i].type] = exts_struct[i].ext;
        memetype[exts_struct[i].ext] = exts_struct[i].type;
    }
}

void set_machine_ip_address()
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        return;
    }
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;
        family = ifa->ifa_addr->sa_family;
        if (family == AF_INET)
        {
            s = getnameinfo(ifa->ifa_addr, sizeof(sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                continue;
            }
            if (string(host) == "127.0.0.1")
                continue;
            machine_ip_address = string(host);
            break;
        }
    }
    freeifaddrs(ifaddr);
}

// CONNECTION
void add_pfd(int fd)
{
    pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN | POLLOUT;
    pfds.push_back(pfd);
}

int create_server_connection(Server srv)
{
    int fd;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        RLOG("create server") << "failed to create server socket" << END;
        return 1;
    }
    int option = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)))
    {
        close(fd);
        RLOG("create server") << "setsockopt failed" << END;
        return 1;
    }
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(srv.port);

    if (inet_pton(AF_INET, srv.listen.c_str(), &addr.sin_addr) <= 0)
    {
        close(fd);
        RLOG("create server") << "Invalid address: " << srv.listen << END;
        return 1;
    }
    if (bind(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
    {
        close(fd);
        RLOG("create server") << "bind failed in " << srv.listen << END;
        return 1;
    }
    if (listen(fd, LISTEN_LEN))
    {
        close(fd);
        RLOG("create server") << "listen failed" << END;
        return 1;
    }
    char ipstr[INET_ADDRSTRLEN];
    if (!inet_ntop(AF_INET, &addr.sin_addr, ipstr, INET_ADDRSTRLEN))
    {
        close(fd);
        RLOG("create server") << "inet_ntop failed" << END;
        return 1;
    }
    add_pfd(fd);
    servers_addresses[fd] = srv.listen;
    servers_ports[fd] = srv.port;
    CLOG("new server") << "has fd " << fd << " listen on " << servers_addresses[fd] << ":" << servers_ports[fd] << endl;
    return 0;
}

void connection_accept(int serv_fd)
{
    Connection con = (Connection){};

    con.action = READ_;
    con.server_address = servers_addresses[serv_fd];
    con.server_port = servers_ports[serv_fd];

    sockaddr_storage addr;
    socklen_t len = sizeof(addr);

    con.fd = accept(serv_fd, (sockaddr *)&addr, &len);
    if (con.fd < 0)
    {
        RLOG("accept client") << "Failed to accept connection from " << con.server_address << ":" << con.server_port << END;
        return;
    }
    char ipstr[INET_ADDRSTRLEN];   // Buffer to hold the IP address string
    if (addr.ss_family == AF_INET) // Check if the address family is IPv4
    {
        sockaddr_in *ipv4 = reinterpret_cast<sockaddr_in *>(&addr);
        inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, INET_ADDRSTRLEN);
    }
    else
    {
        // Unsupported address family
        close(con.fd);
        RLOG("accept client") << "Unsupported address family" << END;
        return;
    }
    con.address = ipstr;
    con.read_fd = con.fd;
    con.write_fd = -1;
    // add_pfd(con.fd);
    pollfd pfd;
    pfd.fd = con.fd;
    pfd.events = POLLIN | POLLOUT | POLLRDHUP | POLLHUP;
    pfds.push_back(pfd);

    CLOG("accept client") << con.address << " has fd " << con.fd << " from " << con.server_address << ":" << con.server_port << endl;
    con.timing = time(NULL);
    clients[con.fd] = con;
}
