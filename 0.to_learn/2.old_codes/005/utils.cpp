#include "header.hpp"

string to_string(size_t size)
{
    stringstream stream;
    stream << size;
    return stream.str();
}

string to_string(HTTP_version version)
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
        return "UNKNOWN (Method)" + to_string((int)method);
}

string to_string(Type type)
{
    if (type == CLIENT_)
        return "CLIENT";
    else if (type == SERVER_)
        return "SERVER";
    else if (type == FILE_)
        return "FILE";
    else
        return "UNKNOWN (Type)" + to_string((int)type);
}

string to_string(Status &status)
{
    switch (status)
    {
    case HTTP_OK:
        return "OK";
    case HTTP_CREATED:
        return "Created";
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
    case HTTP_METHOD_NOT_IMPLEMENTED:
        return "Not Implemented";
    case HTTP_BAD_GATEWAY:
        return "Bad Gateway";
    case HTTP_GATEWAY_TIMEOUT:
        return "Gateway Timeout";
    case HTTP_INSUPPORTED_HTTP:
        return "HTTP Version Not Supported";
    }
    return "";
}

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
ssize_t find(string &str, char *to_find)
{
    return str.find(to_find, 0, strlen(to_find));
}

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
    size_t pos = res.find("//");
    while (pos != std::string::npos)
    {
        res = res.substr(0, pos) + res.substr(pos + 1);
        pos = res.find("//");
    }
    return res;
}

string rand_name()
{
    // return "file000";
    timeval current_time;
    gettimeofday(&current_time, NULL);
    localtime(&current_time.tv_sec);
    stringstream filename;
    filename << current_time.tv_sec;
    return filename.str();
}

string to_lower(string str)
{
    string res;
    for (size_t i = 0; i < str.length(); i++)
        res += tolower(str[i]);
    return res;
}
string to_upper(string &str)
{
    string res;
    for (size_t i = 0; i < str.length(); i++)
        res += toupper(str[i]);
    return res;
}

state get_state(string &path)
{
    state state_;
    stat(path.c_str(), &state_);
    return state_;
}

void init_memetypes()
{
    Type_ext exts_struct[] = {
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

bool is_error(Data &data)
{
    return (data.status >= 400 && data.status <= 511);
}

bool is_move_dir(Data &data)
{
    return (
        data.status == HTTP_MOVE_PERMANENTLY ||
        data.status == HTTP_TEMPORARY_REDIRECT ||
        data.status == HTTP_PERMANENT_REDIRECT);
}

// DEBUG
ostream &operator<<(ostream &out, Location &loc)
{
    int space = 14;
    out << setw(space - 4) << "root <" << loc.root << "> src <" << loc.src << "> ";
    out << "dest <" << loc.dest << "> index <" << loc.index << ">";
    out << (loc.has_limit ? "limit <" + to_string(loc.limit) + ">" : "") << endl;
    out << setw(space - 2) << "methods:"
        << " GET " << (loc.methods[GET_] == on ? "<on>" : "<off>")
        << " POST " << (loc.methods[POST_] == on ? "<on>" : "<off>")
        << " DELETE " << (loc.methods[DELETE_] == on ? "<on>" : "<off>") << endl;
    out << setw(space) << "autoindex "
        << (loc.autoindex == on ? "<on>" : loc.autoindex == off ? "<off>"
                                                                : "<none>")
        << " return " << loc.is_ret << (loc.is_ret ? (" -> " + to_string(loc.ret_status) + " " + loc.ret_location) : "") << endl;
    out << setw(space) << "cgi:      " << endl;
    map<string, string>::iterator it;
    for (it = loc.cgi.begin(); it != loc.cgi.end(); it++)
        out << setw(space) << "ext:      " << it->first << ", path: " << it->second << endl;
    out << setw(space) << "errors:   " << endl;
    map<int, string>::iterator it_err;
    for (it_err = loc.errors.begin(); it_err != loc.errors.end(); it_err++)
        out << setw(space + 5) << "status: " << it_err->first << ", path: " << it_err->second << endl;
    return out;
}

ostream &operator<<(ostream &out, Server &serv)
{
    out << "listen:" << serv.listen << endl;
    out << "port  :" << serv.port << endl;
    out << "name  :" << serv.name << endl;
    map<string, Location>::iterator it;
    for (it = serv.locations.begin(); it != serv.locations.end(); it++)
    {
        out << "location: " << it->first << endl;
        out << it->second << endl;
    }
    return out;
}

ostream &operator<<(ostream &out, Connection &con)
{
    out << " fd " + to_string(con.fd);
    out << " rfd " << con.read_fd;
    out << " wfd " << con.write_fd;
    out << " act " << to_string(con.action);
    out << " met " << to_string(con.data.method);
#if 0
    out << " buff <" + con.buff + ">";
    // out << " writbuff <" + con.writbuff + ">";
#endif
    out << " <" + to_string(con.data.status) + "> " + (is_error(con.data) ? con.data.cause : "");
    return out;
}
