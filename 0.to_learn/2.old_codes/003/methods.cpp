#include "header.hpp"

// Location struct
Location::Location()
{
    limit_body = -1;
    autoindex = none;
    methods[GET_] = false;
    methods[POST_] = false;
    methods[DELETE_] = false;
}

void Location::set_method(size_t line, string &value)
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

bool &Location::operator[](const Method &key)
{
    return methods[key];
}

Location::~Location() {}

// Server struct
Server::Server()
{
    location[""] = Location();
}

void Server::update_host(size_t line)
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

Server::~Server() {}

// Data struct
bool Data::is_error()
{
    return (status >= 400 && status <= 511);
}
bool Data::check(int line, bool cond, Status status_, string cause_)
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
// string Data::cut(ssize_t s, ssize_t &e)
// {
//     e = s;
//     while (e < requbuff.length() && !isspace(requbuff[e]))
//         e++;
//     if (s == e)
//         return "";
//     return substr(LINE, requbuff, s, e);
// }

void Data::init()
{
    *this = (Data){};

    action = READ_;
    ctype = "application/octet-stream";
}
void Data::refresh()
{
    *this = (Data){};
    action = READ_;
    ctype = "application/octet-stream";
}

// Connection struct
void Connection::init()
{
    *this = (Connection){};
    data.init();
    fd = -1;
    read_fd = -1;
    write_fd = -1;
}

int Connection::open_error_page(string &path)
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

int Connection::open_file(string &path)
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
        cout << "New GET file connection has fd " << new_fd << " from client " << fd << endl;
        pfds.push_back((pollfd){.fd = new_fd, .events = POLLIN | POLLOUT});

        string header = generate_header(data, data.ctype, data.st.st_size);
        data.clen = data.st.st_size;

        data.respbuff = header;
        data.flen = data.clen + data.respbuff.length();

        data.action = READ_;
        read_fd = new_fd;
        write_fd = fd;
        types[new_fd] = FILE_;
        pairs[new_fd] = fd;
    }
    else if (data.method == POST_)
    {
        new_fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (new_fd < 0)
        {
            data.status = HTTP_INTERNAL_SERVER;
            return new_fd;
        }
        cout << "New POST file connection has fd " << new_fd << " from client " << fd << endl;
        data.status = HTTP_CREATED;
        pfds.push_back((pollfd){.fd = new_fd, .events = POLLIN | POLLOUT});
        if (data.requbuff.length())
            data.action = WRITE_;
        else
            data.action = READ_;
        // data.clen is provided by the client
        read_fd = fd;
        write_fd = new_fd;
        types[new_fd] = FILE_;
        pairs[new_fd] = fd;
    }
    return new_fd;
}

int Connection::readbuff(int src_fd)
{
    if (src_fd != read_fd)
        throw Error(__LINE__, "readbuff");
    char buffer[READ_BUFFSIZE];
    ssize_t r = 0;

    r = read(src_fd, buffer, READ_BUFFSIZE);
    // TODO: check this -> don't use if its error because you can read from external error pages
    if (r > 0 && data.method == (Method)0)
    {
        cout << "buffer is " << buffer << endl;
        update_timeout();
        data.requbuff.append(buffer, r);
    }
    else if (data.method == GET_)
    {
        if (r > 0)
        {
            update_timeout();
            if (!data.header_parsed) // reading from client
                data.requbuff.append(buffer, r);
            else // reading from file
            {
                data.respbuff.append(buffer, r);
                data.action = WRITE_;
            }
        }
        else if (r == 0)
        {
            cout << "line " << LINE << ": clen: " << data.clen << " flen: " << data.flen << endl;
            cout << "line " << LINE << ": read 0 from " << src_fd << " " << to_string(type) << endl;
            close_connection(LINE, src_fd);
            // if (data.is_cgi) // TODO: remove when parsing the result
            // {
            //     close_connection(LINE, write_fd);
            // }
        }
        else // error, close it
        {
            cerr << RED << "read failed in " + con_state(*this) << RESET << endl;
            close_connection(LINE, src_fd);
        }
    }
    return r;
}

int Connection::writebuff(int dest_fd)
{
    if (dest_fd != write_fd)
        throw Error(__LINE__, "writebuff");
    ssize_t w = 0;
    if (data.method == GET_)
    {
        string &buff = data.respbuff;
        w = write(dest_fd, buff.c_str(), buff.length());
        if (w > 0)
        {
            update_timeout();
            data.write_size += w;
            cout << "line: " << LINE << " write res: " << w << " clen: " << data.clen
                 << " flen: " << data.flen << " wlen: " << data.write_size << " to fd: " << dest_fd << endl;
            buff = substr(LINE, buff, w, buff.length());
            if (data.write_size == data.flen)
            {
                cout << "GET: write reach end of file" << endl;
            }
            else if (buff.empty())
                data.action = READ_;
        }
        else if (w == 0)
        {
            cerr << RED << "write 0 in " + con_state(*this) << RESET << endl;
            close_connection(LINE, dest_fd);
        }
        else // error close it
        {
            cerr << RED << "write failed in " + con_state(*this) << RESET << endl;
            close_connection(LINE, dest_fd);
        }
    }
    return w;
}

int Connection::open_dir(string &path, string &uri)
{
    cout << "GET dir " << path << endl;
    dirent *entry;
    DIR *dir = opendir(path.c_str());
    if (dir == NULL)
        throw Error("open dir failed");
    data.respbuff = "<ul>\n";
    entry = readdir(dir);
    entry = readdir(dir);
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
    return 1;
}

void Connection::close_connection(int line, int close_fd)
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

bool Connection::timeout()
{
    bool res = (!data.keep_alive && (time(NULL) - data.last_activity > SOCK_TIMEOUT)) ||
               (data.keep_alive && (time(NULL) - data.last_activity > KEEP_ALIVE_TIMEOUT));
    if (res)
        cout << "timeout " << endl;
    return (res);
}

void Connection::update_timeout()
{
    data.last_activity = time(NULL);
}