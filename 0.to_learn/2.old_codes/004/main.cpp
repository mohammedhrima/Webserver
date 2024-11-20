#include "header.hpp"

map<int, int> pairs;
map<int, Connection> cons;
vector<pollfd> pfds;
map<size_t, vServer> servers;
map<string, string> memetype;
string machine_ip_address;
map<int, Type> types;

// config location alwaays starts with / and ends with /
// TODO: http://127.0.0.1:17000/retjy
bool match_location(string &conf_location, string uri)
{
    size_t i = 0;
    while (i < conf_location.length() && i < uri.length() && conf_location[i] == uri[i])
        i++;
    // cout << GREEN << "match: " << uri << " with: " << conf_location << endl;
    // cout << "i:                     " << i << endl;
    // cout << "conf_location.length() " << (conf_location.length()) << endl;
    // cout << "uri.length()           " << (uri.length()) << endl;
    // cout << END;

    return (
        (i == conf_location.length() && (i == uri.length() || uri[i - 1] == '/')) ||
        (i == conf_location.length() - 1 && i == uri.length()));
}

void set_status(int line, string &path, Connection &con, Location &location)
{
    DEBUG << "set status from path " << path << END;
    DEBUG << "con " << con << END;
    // check allowed mathod here
    con.data.status = HTTP_NOT_FOUND;
    con.data.st = (state){};
    if (!location.methods[con.data.method])
    {
        con.data.status = HTTP_METHOD_NOT_ALLOWED;
        return;
    }
    // int
    if (stat(path.c_str(), &con.data.st) == 0)
    {
        if (!(con.data.st.st_mode & IS_ACCESS))
        {
            con.data.cause = path + " is forbiden";
            con.data.status = HTTP_FORBIDEN;
        }
        else
        {
            if (con.data.st.st_mode & IS_FILE)
            {
                if (access(path.c_str(), con.data.method == POST_ ? W_OK : (R_OK | F_OK)) == 0)
                {
                    con.data.status = HTTP_OK;
                }
                else
                {
                    con.data.cause = path + " is forbiden";
                    con.data.status = HTTP_FORBIDEN;
                }
            }
            else if (con.data.st.st_mode & IS_DIR)
            {
                if (con.data.uri[con.data.uri.length() - 1] == '/' || con.data.method == DELETE_)
                    con.data.status = HTTP_OK;
                else
                    con.data.status = HTTP_MOVE_PERMANENTLY;
            }
        }
    }
}

string status_to_string(Status &status)
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

string generate_header(Connection &con, string ctype, size_t size)
{
    Data &data = con.data;
    string version = to_string(data.http_version);
    if (version.empty())
        version = "HTTP/1.0";
    string header = version + " " + to_string((int)data.status) + " " + status_to_string(data.status) + "\r\n";
    header += (ctype.length() ? "Content-Type: " + ctype + "\r\n" : "");
    header += (size ? "Content-Length: " + to_string(size) + "\r\n" : "");
    if (con.data.hosts.size())
        header += "Host: " + con.data.hosts[0] + "\r\n"; // to be checked

    if (is_move_dir(con.data))
        header += "Location: http://" + con.srv_listen + ":" + to_string(con.srv_port) + data.uri + "/\r\n";
    header += "\r\n";
    return header;
}

int open_error_page(Connection &con, string &path)
{
    int fd = -1;
    Data &data = con.data;

    fd = open(path.c_str(), O_RDONLY, 0777);
    if (fd < 0)
    {
        cerr << RED << "opening error page" RESET << endl;
        return fd;
    }
    ssize_t dot = path.find_last_of(".");
    string ext;
    if (dot != string::npos && memetype.count((ext = substr(LINE, path, dot, path.length()))))
        data.ctype = memetype[ext];
    else
        data.ctype = "application/octet-stream";

    cerr << RED "error page has fd: " RESET << fd << endl;
    pfds.push_back((pollfd){.fd = fd, .events = POLLIN | POLLOUT});
    types[fd] = FILE_;

    string header = generate_header(con, data.ctype, data.st.st_size);
    data.clen = data.st.st_size;
#if 0
    if (con.data.method)
    {
        con.writbuff = header;
        data.flen = data.clen + con.writbuff.length();
    }
    else
    {
        con.readbuff = header;
        data.flen = data.clen + con.readbuff.length();
    }
#else
    con.writbuff = header;
    data.flen = data.clen + con.writbuff.length();
    con.write_size = 0;
#endif
    con.action = READ_;
    pairs[fd] = con.fd;
    con.read_fd = fd;
    con.write_fd = con.fd;
    // data.read_size = 0;
    // data.write_size = 0;

    return fd;
}

int open_file(Connection &con, string &path)
{
    int fd = -1;
    Data &data = con.data;
    if (data.method == GET_)
    {
        ssize_t dot = path.find_last_of(".");
        string ext;
        if (dot != string::npos && memetype.count((ext = substr(LINE, path, dot, path.length()))))
            data.ctype = memetype[ext];
        else
            data.ctype = "application/octet-stream";
        fd = open(path.c_str(), O_RDONLY);
        if (fd < 0)
        {
            // cerr << RED << "openning " << path << RESET << endl;
            data.status = HTTP_INTERNAL_SERVER;
            data.cause = "openning GET file " + path;
            // throw Error(LINE, "");
            return fd;
        }
        cout << "New GET file connection " << path << " has fd " << fd << " from client " << con.fd << endl;
        pfds.push_back((pollfd){.fd = fd, .events = POLLIN | POLLOUT});
        types[fd] = FILE_;

        string header = generate_header(con, data.ctype, data.st.st_size);
        data.clen = 0;

        con.writbuff = header;
        data.flen = data.st.st_size + con.writbuff.length();

        con.action = READ_;
        con.read_fd = fd;
        con.write_fd = con.fd;
        pairs[fd] = con.fd;
        if (data.st.st_size == 0) // file is empty
        {
            DEBUG << "file is empty" << END;
            con.action = WRITE_;
        }
    }
    else if (data.method == POST_)
    {
        fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (fd < 0)
        {
            // cerr << RED << "openning " << path << RESET << endl;
            data.status = HTTP_INTERNAL_SERVER;
            data.cause = "openning POST file " + path;
            // throw Error(LINE, "");
            return fd;
        }
        cout << "New POST file connection " << path << " has fd " << fd << " from client " << con.fd << endl;
        pfds.push_back((pollfd){.fd = fd, .events = POLLIN | POLLOUT});
        types[fd] = FILE_;
        if (con.readbuff.length())
            con.action = WRITE_;
        else
            con.action = READ_;
        con.read_fd = con.fd;
        con.write_fd = fd;
        pairs[fd] = con.fd;
        con.data.flen = con.data.clen; // TODO: to be checked
        con.write_size = 0;
    }
    return fd;
}

int open_dir(Connection &con, string &path)
{
    cout << "GET dir " << path << endl;
    dirent *entry;
    DIR *dir = opendir(path.c_str());
    if (dir == NULL)
    {
        // cerr << RED << "openning " << path << RESET << endl;
        con.data.status = HTTP_INTERNAL_SERVER;
        con.data.cause = "openning " + path;
        return -1;
    }

    con.writbuff = "<ul>\n";
    entry = readdir(dir);
    entry = readdir(dir);
    while ((entry = readdir(dir)) != NULL)
    {
        // http://" + con.srv_listen + ":" + to_string(con.srv_port) + "/" + data.uri +
        string url = "<a href=\"http://" +
                     clear_path(con.srv_listen + ":" +
                                to_string(con.srv_port) + "/" + con.data.uri + "/" + string(entry->d_name)) +
                     "\">" +
                     string(entry->d_name) + "</a><br>\n";
        con.writbuff += url;
    }
    closedir(dir);
    con.writbuff += "</ul>";
    con.data.clen = con.writbuff.length();
    con.action = WRITE_;
    con.writbuff = generate_header(con, "text/html", con.data.clen) + con.writbuff;
    con.data.flen = con.writbuff.length();
    con.read_fd = con.fd;
    con.write_fd = con.fd;
    con.write_size = 0;
    return 1;
}

void generate_response(int line, Connection &con, Location &location)
{
    con.data.matched_location = &location;
    Data &data = con.data;
    cout << "call generate response from line " << line << " " << (int)data.status << " " << to_string(data.method) << " " << endl;
    Status status = data.status;

    if (is_error(data))
    {
        con.action = WRITE_;
        con.write_fd = con.fd;
        con.write_size = 0;
        con.readbuff = "";
        con.writbuff = "";

        cout << LINE << " is error status: " << (int)data.status << " cause: " << data.cause << endl;
        cout << RED << "======================================================" RESET << endl;
        cout << location << endl;
        if (location.errors.count((int)data.status))
        {
            string path = clear_path(location.errors[data.status]);
            cout << LINE << "check error page: " << path << endl;
            data.st = get_state(path);
            bool isaccess = (data.st.st_mode & IS_ACCESS) && (data.st.st_mode & IS_FILE);
            bool opened_succefully = isaccess && open_error_page(con, path) > 0;
            if (isaccess && opened_succefully)
            {
                cout << LINE << RED " response with openned error page: " RESET << endl;
                return;
            }
        }
        // throw Error("Error page not found");
        cout << LINE << RED " response with generated error page: " RESET << endl;
        string response;

        if (status == HTTP_BAD_REQUEST)
            response = "BAD REQUEST";
        else if (status == HTTP_FORBIDEN)
            response = "FORBIDEN";
        else if (status == HTTP_NOT_FOUND)
            response = "NOT FOUND";
        else if (status == HTTP_METHOD_NOT_ALLOWED)
            response = "METHOD NOT ALLOWED";
        else if (status == HTTP_TIMEOUT)
            response = "TIMEOUT";
        else if (status == HTTP_LENGTH_REQUIRED)
            response = "CONTENT LENGTH REQUIRED";
        else if (status == HTTP_CONTENT_TO_LARGE)
            response = "CONTENT TO LARGE";
        else if (status == HTTP_URI_TO_LARGE)
            response = "URI TO LARGE";
        else if (status == HTTP_HEADER_TO_LARGE)
            response = "HEADER TO LARGE";
        else if (status == HTTP_INTERNAL_SERVER)
            response = "INTERNAL SERVER ERROR";
        else if (status == HTTP_METHOD_NOT_IMPLEMENTED)
            response = "METHOD NOT IMPLEMENTED";
        else if (status == HTTP_BAD_GATEWAY)
            response = "BAD GATEWAY";
        else if (status == HTTP_GATEWAY_TIMEOUT)
            response = "GATEWAY TIMEOUT";
        else if (status == HTTP_INSUPPORTED_HTTP)
            response = "INSUPPORTED HTTP";
        else
            response = "";

        response = "<div style=\"display:flex; justify-content:center;"
                   "align-items:center; color:blue; font-size: large;\"> Generated " +
                   response + (data.cause.length() ? " because " + data.cause : "") + "</div>";

        string header = generate_header(con, "text/html", response.length());
        data.clen = response.length();

        // if (con.data.method)
        // {
        con.writbuff = header + response;
        con.data.flen = con.writbuff.length();
        cout << con.writbuff << endl;
        con.write_size = 0;
        // }
        // else
        // {
        //     con.readbuff = header + response;
        //     con.data.flen = con.readbuff.length();
        //     cout << con.writbuff << endl;
        // }
        // con.writbuff = header + response;
        // data.flen = con.writbuff.length();
        // throw Error("");

        con.action = WRITE_;
        con.read_fd = -1;
        con.write_fd = con.fd;
        // data.finish_connection = true;
    }
    else if (data.method == POST_ && status == HTTP_CREATED)
    {
        con.action = WRITE_;
        string response = "<div style=\"display:flex; justify-content:center;"
                          "align-items:center; color:blue; font-size: large;\">"
                          "FILE created succefully"
                          "</div>";
        // to be checked, I change clen here
        data.clen = response.length();
        string header = generate_header(con, "text/html", data.clen);
        con.writbuff = header + response;

        // data.write_size = 0;
        // data.flen = con.writbuff.length();
        // data.finish_connection = true;
    }
    else if (is_move_dir(data))
    {
        con.action = WRITE_;
        data.clen = 0;
        string header = generate_header(con, "", data.clen);
        con.writbuff = header;

        data.flen = con.writbuff.length();
        con.write_size = 0;
        con.read_fd = con.fd;
        con.write_fd = con.fd;
        // data.finish_connection = true;
    }
    else if (data.method == DELETE_ && status == HTTP_OK)
    {
        con.action = WRITE_;
        string header = generate_header(con, "text/html", 0);
        con.writbuff = header;
        data.clen = 0;
        data.flen = con.writbuff.length();
        con.write_size = 0;
        con.read_fd = con.fd;
        con.write_fd = con.fd;
    }
}

bool Delete(Data &data, string path)
{
    bool success_once = false;
    state st = get_state(path);
    if (!(st.st_mode & IS_FILE) && !(st.st_mode & IS_DIR))
    {
        data.status = HTTP_NOT_FOUND;
        data.cause = "NOT found " + path;
        return false;
    }
    else if (!(st.st_mode & IS_ACCESS))
    {
        data.status = HTTP_INTERNAL_SERVER;
        data.cause = "failed to delete file " + path;
        return false;
    }
    else if (st.st_mode & IS_FILE)
    {
        if (remove(path.c_str()) < 0)
        {
            // cout << "failed remove file " << path << endl;
            // res = -1;
            data.status = HTTP_INTERNAL_SERVER;
            data.cause = "failed to delete file " + path;
        }
        else
        {
            success_once = true;
            // data.status = HTTP_OK;
            cout << "remove file " << path << endl;
        }
    }
    else if (st.st_mode & IS_DIR)
    {
        dirent *entry;
        DIR *dir = opendir(path.c_str());
        if (dir == NULL)
            return -1;
        entry = readdir(dir); // skip .
        entry = readdir(dir); // skip ..
        while ((entry = readdir(dir)) != NULL)
        {
            Delete(data, path + "/" + string(entry->d_name));
            if (is_error(data))
            {
                cout << "failed remove " << path + "/" + string(entry->d_name) << endl;
                // res = -1;
            }
            else
                success_once = true;
        }
        cout << "remove dir " << path << endl;
        if (rmdir(path.c_str()) != 0 && !success_once)
        {
            data.status = HTTP_FORBIDEN;
            data.cause = "failed to delete dir " + path;
        }
    }
    if (success_once)
    {
        data.status = HTTP_OK;
        data.cause = "";
    }
    return success_once;
}

void connection_set_response(int line, Connection &con)
{
    cout << "called from line " << line << endl;
    // INFO << con << endl
    //      << RESET;
    string match;
    Location *matched_location = NULL;
    Data &data = con.data;
    string serv_name;

    int round = 0;
    if (!servers.count(con.srv_port)) // to be removed
    {
        ERROR << con << END;
        throw Error(LINE, "server with port " + to_string(con.srv_port) + "not found");
    }

    vServer vservs = servers[con.srv_port];
    ssize_t i = 0;
    // while(i < data.hosts.size())
    // {
    //     cout << "expect response from " << data.hosts[i] << endl;
    //     i++;
    // }
    // throw Error("debuging");
    i = 0;
    while (i < vservs.size())
    {
        Server &serv = vservs[i];
        // cout << "check server <" << serv.listen << "> : <" << serv.port << "> name: <" << serv.name << ">" << endl;
        if (con.srv_listen == serv.listen && con.srv_port == vservs[i].port)
        {
            if (data.hosts.size() == 0)
            {
                map<string, Location>::iterator it;
                for (it = serv.locations.begin(); it != serv.locations.end(); it++)
                {
                    string key = it->first;
                    // cout << "match: " << data.uri << " with " << key << endl;
                    // keep it > to skip empty location
                    if (match_location(key, data.uri) && key > match)
                    {
                        // cout << GREEN "location matched with " RESET << key << endl;
                        match = key;
                        matched_location = &it->second;
                        serv_name = vservs[i].name;
                    }
                    else if (round == 2 && key.empty())
                    {
                        matched_location = &it->second;
                        serv_name = vservs[i].name;
                        break;
                    }
                }
            }
            else
            {
                size_t j = 0;
                while (j < data.hosts.size())
                {
                    // cout << "check server name if it's <" << data.hosts[j] << ">" << endl;
                    if ((round == 0 && serv.name == data.hosts[j]) || round)
                    {
                        map<string, Location>::iterator it;
                        for (it = serv.locations.begin(); it != serv.locations.end(); it++)
                        {
                            string key = it->first;
                            // cout << "match: " << data.uri << " with " << key << endl;
                            // keep it > to skip empty location
                            if (match_location(key, data.uri) && key > match)
                            {
                                // cout << GREEN "location matched with " RESET << key << endl;
                                match = key;
                                matched_location = &it->second;
                                serv_name = vservs[i].name;
                            }
                            else if (round == 2 && key.empty())
                            {
                                matched_location = &it->second;
                                serv_name = vservs[i].name;
                                break;
                            }
                        }
                    }
                    j++;
                }
            }
        }
        i++;
        if (round < 2 && i == vservs.size() && matched_location == NULL)
        {
            round++;
            i = 0;
        }
    }
    if (matched_location->location.empty() || is_error(con.data))
    {
        // LOCATION NOT FOUND
        if (!is_error(con.data))
            check(LINE, con, true, HTTP_NOT_FOUND, "location not found");
        generate_response(LINE, con, *matched_location);
        return;
    }
    if (serv_name.length())
    {
        if (data.hosts.size())
            data.hosts[0] = serv_name; // to be checked
        else
            data.hosts.push_back(serv_name);
    }
    if (data.method)
    {
        if (matched_location->methods[data.method] != on)
        {
            data.status = HTTP_METHOD_NOT_ALLOWED;
            return generate_response(LINE, con, *matched_location);
        }
        else if (data.method == GET_)
        {
            if (matched_location->is_ret)
            {
                data.status = matched_location->ret_status;
                data.uri = matched_location->ret_location;
                return generate_response(LINE, con, *matched_location);
            }
            string match_uri = data.uri.length() > match.length()
                                   ? substr(LINE, data.uri, match.length(), data.uri.length())
                                   : "";
            string path = clear_path(matched_location->src + "/" + match_uri);
            DEBUG << "path is " << path << END;
            set_status(LINE, path, con, *matched_location);
            if (data.st.st_mode & IS_DIR)
            {
                cout << "found dir " << path << endl;
                if (is_move_dir(data))
                {
                    cout << con << endl;
                    return generate_response(LINE, con, *matched_location);
                }
                else if (data.status == HTTP_OK)
                {
                    if (matched_location->autoindex != on)
                    {
                        data.status = HTTP_FORBIDEN;
                        return generate_response(LINE, con, *matched_location);
                    }
                    /*
                    if has index:
                        if open_file send it
                    */
                    bool index_file_found = false;
                    if (matched_location->index.length())
                    {
                        string index_path = clear_path(path + "/" + matched_location->index);
                        // cout << "has index: " << index_path << endl;
                        // throw Error("");
                        state tmp_state = get_state(index_path);
                        if ((tmp_state.st_mode & IS_ACCESS) && (tmp_state.st_mode & IS_FILE) &&
                            access(index_path.c_str(), R_OK | F_OK) == 0)
                        {
                            index_file_found = true;
                            data.st = tmp_state;
                            path = index_path;
                        }
                    }
                    // working on listing directory
                    if (!index_file_found && open_dir(con, path) > 0)
                    {

                        // throw Error("open file");
                        return;
                    }
                }
            }
            if (data.st.st_mode & IS_FILE && data.status == HTTP_OK)
            {
                cout << "found file " << path << endl;
                data.clen = data.st.st_size;
                // TODO: file might be cgi and st_size == 0
                // if (con.file_is_cgi(path, loc))
                // {
                //     handle_cgi(con, path, loc);
                //     return;
                // }
                // else
                if (open_file(con, path) > 0)
                {
                    // throw Error("open file");
                    return;
                }
            }
        }
        else if (data.method == POST_)
        {
            if (matched_location->has_limit && con.data.clen > matched_location->limit)
            {
                con.data.status = HTTP_CONTENT_TO_LARGE;
                con.data.cause = "limit is " + to_string(matched_location->limit) + " but file has " + to_string(con.data.clen);
                return generate_response(LINE, con, *matched_location);
            }
            string match_uri = data.uri.length() > match.length()
                                   ? substr(LINE, data.uri, match.length(), data.uri.length())
                                   : "";
            string ext;
            if (data.ctype.length() && memetype.count(data.ctype))
                ext = memetype[data.ctype];
            string path = clear_path(matched_location->dest + "/" + match_uri + "/" + rand_name() + ext);
            cout << "POST to file " << path << endl;
            if (open_file(con, path) > 0)
            {
                if (con.data.clen == 0)
                {
                    con.data.status = HTTP_CREATED;
                    string response = "<div style=\"display:flex; justify-content:center;"
                                      "align-items:center; color:blue; font-size: large;\">"
                                      "empty file created succefully"
                                      "</div>";
                    // to be checked, I change clen here
                    con.data.clen = response.length();
                    string header = generate_header(con, "text/html", con.data.clen);
                    con.writbuff = header + response;
                    con.data.flen = con.writbuff.length();

                    con.write_size = 0;
                    con.action = WRITE_;
                    connection_close(LINE, con.write_fd);
                    con.write_fd = con.fd;
                }
                return;
            }
        }
        else if (data.method == DELETE_)
        {
            string match_uri = data.uri.length() > match.length()
                                   ? substr(LINE, data.uri, match.length(), data.uri.length())
                                   : "";
            string path = clear_path(matched_location->src + "/" + match_uri);
            set_status(LINE, path, con, *matched_location);
            // TODO: check permession with access
            if (data.status == HTTP_OK && Delete(data, path))
                return generate_response(LINE, con, *matched_location);
        }
    }
    if (is_error(data))
    {
        generate_response(LINE, con, *matched_location);
        return;
    }
    // throw Error("call connection response");
}

void connection_close(int line, int fd)
{
    cout << "line " << line << ": connection close " << fd << endl;
    if (cons.count(fd))
    {
        if (cons[fd].fd == fd)
        {
            if (cons[fd].read_fd != fd && cons[fd].read_fd > 0)
                connection_close(LINE, cons[fd].read_fd);
            if (cons[fd].write_fd != fd && cons[fd].write_fd > 0)
                connection_close(LINE, cons[fd].write_fd);
        }
        else if (cons[fd].read_fd == fd)
        {
            cons[fd].read_fd = cons[fd].fd;
            // data.write_size = 0; // used when listing directory
        }
        else if (cons[fd].write_fd == fd)
        {
            cons[fd].write_fd = -1;
            // data.read_size = 0; // used when listing directory
        }
        cons.erase(fd);
        // cout << "close " << fd << endl;
    }
    if (pairs.count(fd))
        pairs.erase(fd);
    // else
        // cout << fd << " Not found (connection_destory)" << endl;
    for (vector<pollfd>::iterator it = pfds.begin(); it != pfds.end(); ++it)
    {
        if (it->fd == fd)
        {
            pfds.erase(it);
            break;
        }
    }
    close(fd);
    see_pfds_size();
}

Connection &connection_find(int fd)
{
    if (cons.count(fd))
    {
        // cout << fd << " found in pairs (connection_find)" << endl;
        return cons[fd];
    }
    else if (pairs.count(fd))
    {
        // cout << fd << " found in cons (connection_find)" << endl;
        return cons[pairs[fd]];
    }
    throw Error("searching for " + to_string(fd));
}

void connection_read(Connection &con, int fd)
{
    if (fd != con.read_fd)
    {
        cout << "connection_read from: " << fd << endl;
        cout << con << endl;
        throw Error(LINE, "weird error");
    }
    char rbuff[BUFFSIZE];

    if (con.data.method == 0)
    {
        ssize_t r = read(fd, rbuff, BUFFSIZE);
        if (r < 0)
        {
            DEBUG << con << END;
            // throw Error(LINE, "read failed");
            connection_close(LINE, con.fd);
            return;
        }
        if (is_error(con.data))
        {
            con.timeout = time(NULL);
            con.writbuff.append(rbuff, r);
            con.action = WRITE_;
        }
        else
        {
            if (r > 0)
            {
                con.timeout = time(NULL);
                con.readbuff.append(rbuff, r);
                con.action = WRITE_;
                // DEBUG << "did read " << r << " bytes" /*<<"<" << buff << ">"*/ << END;
            }
            else if (r == 0)
            {
                // throw Error(LINE, "reached end of reading");
                // if(fd != con.fd)
                //     connection_close(LINE,  fd);
            }
            if (con.readbuff.length() && !con.data.header_parsed && parse_header(con))
            {
                con.timeout = time(NULL);
                connection_set_response(LINE, con);
            }
            if (con.data.header_parsed)
                con.action = WRITE_;
        }
    }
    else if (con.data.method == GET_)
    {
        ssize_t r = read(fd, rbuff, BUFFSIZE);
        if (r < 0)
        {
            DEBUG << con << END;
            // throw Error(LINE, "read failed");
            connection_close(LINE, con.fd);
            return;
        }
        if (is_error(con.data))
        {
            con.timeout = time(NULL);
            con.writbuff.append(rbuff, r);
            con.action = WRITE_;
        }
        else
        {
            if (r > 0)
            {
                con.timeout = time(NULL);
#if 1
                if (!con.data.header_parsed) // reading from client
                {
                    con.readbuff.append(rbuff, r);
                    if (parse_header(con))
                        connection_set_response(LINE, con);
                }
                else
                {
                    con.writbuff.append(rbuff, r);
                    con.action = WRITE_;
                }
#else
                DEBUG << con << END;
                if (!con.data.header_parsed) // reading from client
                {
                    con.readbuff.append(rbuff, r);
                    if (parse_header(con) && !con.data.clen_found)
                        connection_set_response(LINE, con);
                }
                DEBUG << con << END;
                if (con.data.header_parsed) // reading from file
                {
                    if (con.data.clen_found)
                    {
                        con.readbuff.append(rbuff, r);
                        if (con.data.clen <= con.readbuff.length())
                        {
                            con.readbuff = substr(LINE, con.readbuff, 0, con.data.clen);
                            // con.data.clen =
                            con.data.clen_found = false;
                            connection_set_response(LINE, con);
                        }
                        else
                        {
                            con.data.clen -= con.readbuff.length();
                            con.readbuff = "";
                        }
                    }
                    else
                    {
                        con.writbuff.append(rbuff, r);
                        con.action = WRITE_;
                    }
                }
#endif
                DEBUG << con << END;
                // DEBUG << "did read " << r << " bytes" /*<<"<" << buff << ">"*/ << END;
            }
            else if (r == 0 && con.readbuff.empty())
            {
                DEBUG << con << endl;
                connection_close(LINE, con.fd);
                // throw Error(LINE, "reached end of reading");
                // if(fd != con.fd)
                //     connection_close(LINE,  fd);
            }
            else if (r == 0 && con.readbuff.length())
            {
                // TODO: check this also
                if (!con.data.header_parsed && parse_header(con))
                {
                    con.timeout = time(NULL);
                    connection_set_response(LINE, con);
                }
            }
        }
    }
    else if (con.data.method == POST_)
    {
        ssize_t r = read(fd, rbuff, BUFFSIZE);
        if (r < 0)
        {
            DEBUG << con << END;
            connection_close(LINE, con.fd);
            // throw Error(LINE, "read failed");
            return;
        }
        else
        {
            if (r > 0)
            {
                con.timeout = time(NULL);
                if (is_error(con.data))
                {
                    con.writbuff.append(rbuff, r);
                    con.action = WRITE_;
                }
                else if (!con.data.header_parsed)
                {
                    con.readbuff.append(rbuff, r);
                    if (parse_header(con))
                        connection_set_response(LINE, con);
                }
                else
                {
                    // will only read from client
                    con.readbuff.append(rbuff, r);
                    con.action = WRITE_;
                }
                // if (r == con.data.flen)
                // {
                //     DEBUG << "read the full size for " << END;
                //     throw Error(LINE, "");
                // }
            }
            if (r == 0)
                cout << GREEN << "POST read 0 byte" << endl;
        }
    }
    // else if (con.data.method == DELETE_)
    // {
    //     ssize_t r = read(fd, rbuff, BUFFSIZE);
    //     if (r < 0)
    //     {
    //         DEBUG << con << END;
    //         connection_close(LINE, con.fd);
    //         // throw Error(LINE, "read failed");
    //         return;
    //     }
    //     if (r > 0)
    //     {
    //         con.timeout = time(NULL);
    //         if (is_error(con.data))
    //         {
    //             con.writbuff.append(rbuff, r);
    //             con.action = WRITE_;
    //         }
    //         else if (!con.data.header_parsed)
    //         {
    //             con.readbuff.append(rbuff, r);
    //             if (parse_header(con))
    //                 connection_set_response(LINE,con);
    //         }
    //         else
    //         {
    //             con.writbuff.append(rbuff, r);
    //             con.action = WRITE_;
    //         }
    //         // if (r == con.data.flen)
    //         // {
    //         //     DEBUG << "read the full size for " << END;
    //         //     throw Error(LINE, "");
    //         // }
    //     }
    // }
    else
        throw Error("read to unknow method");
}

void connection_write(Connection &con, int fd)
{
    if (fd != con.write_fd)
    {
        cout << "connection_write to: " << fd << endl;
        cout << con << endl;
        throw Error(LINE, "weird error");
    }
    ssize_t w;
    if (con.data.method == 0)
    {
        string &buff = is_error(con.data) ? con.writbuff : con.readbuff;
        w = write(fd, buff.c_str(), buff.length());
        if (w < 0)
        {
            // throw Error(LINE, "write failed");
            connection_close(LINE, con.fd);
        }
        else
        {
            DEBUG << "did write " << w << " bytes" /*<<"<" << buff << ">"*/ << endl;
            cout << "flen: " << con.data.flen << endl;
            cout << "wlen: " << con.write_size << endl;
            if (w > 0)
            {
                buff = substr(LINE, buff, w, buff.length());
                // DEBUG << "did write " << w << " bytes" /*<<"<" << buff << ">"*/ << endl;
                // cout << con << endl;
                // cout << "remaining <" << con.writbuff << ">" << endl;
                con.write_size += w;
                con.timeout = time(NULL);
                // cout << "write_size: " << con.write_size << " flen: " << con.data.flen << endl;
            }
            // write the full con.writbuffer
            if (con.write_size == con.data.flen)
            {
                if (is_error(con.data))
                    connection_close(LINE, con.fd);
            }
        }
    }
    else if (con.data.method == GET_)
    {
        string &buff = (is_error(con.data) || con.data.header_parsed) ? con.writbuff : con.readbuff;

        w = write(fd, buff.c_str(), buff.length());
        if (w < 0)
        {
            // throw Error(LINE, "write failed");
            connection_close(LINE, con.fd);
        }
        else
        {
            if (w > 0)
            {
                buff = substr(LINE, buff, w, buff.length());
                // DEBUG << "did write " << w << " bytes" /*<<"<" << buff << ">"*/ << endl;
                // cout << con << endl;
                // cout << "remaining <" << con.writbuff << ">" << endl;
                con.write_size += w;
                con.timeout = time(NULL);
                // cout << "write_size: " << con.write_size << " flen: " << con.data.flen << endl;
            }
            if (con.write_size == con.data.flen)
            {
                DEBUG << "write the full size " << END;
                cout << con << endl;
                if (is_error(con.data))
                    connection_close(LINE, con.fd);
                else if (con.readbuff.empty())
                    connection_close(LINE, con.fd);
                else
                {
                    DEBUG << "but there is remaining body" << END;
                    cout << con << endl;
                    if (con.read_fd != con.fd) // this line is added because it segvault when get directory
                        connection_close(LINE, con.read_fd);
                    con.data = (Data){};
                    con.action = READ_;
                    con.read_fd = con.fd;
                    con.write_fd = con.fd;
                    con.write_size = 0;
                    if (parse_header(con))
                        connection_set_response(LINE, con);

                    // TODO: if body doesnt starts with method uri http_version -> bad request

                    cout << con << endl;
                    // throw Error(LINE, "");
                }
            }
            else
                con.action = READ_;

            if (w == 0) // TODO: this condition should never be true
            {
                DEBUG << con << endl;
                // throw Error(LINE, "reached end of writing");
            }
        }
    }
    else if (con.data.method == POST_)
    {
        string &buff = (is_error(con.data) || con.write_fd == con.fd) ? con.writbuff : con.readbuff;
        w = write(fd, buff.c_str(), buff.length());
        if (w < 0)
        {
            // throw Error(LINE, "write failed");
            connection_close(LINE, con.fd);
        }
        else
        {
            if (w > 0)
            {
                buff = substr(LINE, buff, w, buff.length());
                // DEBUG << "did write " << w << " bytes" /*<<"<" << buff << ">"*/ << endl;
                // cout << "remaining <" << con.writbuff << ">" << endl;
                con.write_size += w;
                con.timeout = time(NULL);
            }
            // cout << "write_size: " << con.write_size << " flen: " << con.data.flen << endl;
            if (con.write_size == con.data.flen)
            {
                if (con.fd != fd) // POST full file
                {
                    DEBUG << "write the full size " << END;
                    cout << GREEN << con << END;
                    con.data.status = HTTP_CREATED;
                    string response = "<div style=\"display:flex; justify-content:center;"
                                      "align-items:center; color:blue; font-size: large;\">"
                                      "file created succefully"
                                      "</div>";
                    // to be checked, I change clen here
                    con.data.clen = response.length();
                    string header = generate_header(con, "text/html", con.data.clen);
                    con.writbuff = header + response;
                    con.data.flen = con.writbuff.length();
                    con.write_size = 0;
                    con.action = WRITE_;
                    con.write_fd = con.fd;
                    connection_close(LINE, fd); // close file
                    return;
                }
                else // finish POST response
                {
                    DEBUG << con << endl;

                    con.data = (Data){};
                    if (is_error(con.data))
                        connection_close(LINE, con.fd);
                    else if (con.readbuff.empty())
                        connection_close(LINE, con.fd);
                    else if (con.readbuff.length())
                    {
                        DEBUG << "but there is remaining body" << END;
                        cout << con.readbuff << endl;
                        con.data = (Data){};
                        con.action = READ_;
                        con.read_fd = con.fd;
                        con.write_fd = con.fd;
                        con.write_size = 0;
                        if (parse_header(con))
                            connection_set_response(LINE, con);
                        else
                            ERROR << "header not parsed" << END;
                        cout << "<" << con << ">" << endl;
                        // throw Error("");

                        // TODO: if body doesnt starts with method uri http_version -> bad request

                        // cout << con << endl;
                        // throw Error(LINE, "");
                    }
                }
            }
            if (w == 0)
            {
                con.action = READ_;
                cout << GREEN << "POST write 0 byte" << endl;
            }
        }
    }
    // else if (con.data.method == DELETE_)
    // {
    //     w = write(fd, con.writbuff.c_str(), con.writbuff.length());
    //     if (w < 0)
    //     {
    //         // throw Error(LINE, "write failed");
    //         connection_close(LINE, con.fd);
    //     }
    //     else
    //     {
    //         if (w > 0)
    //         {
    //             con.writbuff = substr(LINE, con.writbuff, w, con.writbuff.length());
    //             // DEBUG << "did write " << w << " bytes" /*<<"<" << buff << ">"*/ << endl;
    //             // cout << con << endl;
    //             // cout << "remaining <" << con.writbuff << ">" << endl;
    //             con.write_size += w;
    //             con.timeout = time(NULL);
    //             // cout << "write_size: " << con.write_size << " flen: " << con.data.flen << endl;
    //         }
    //         // write the full con.writbuffer
    //         if (con.write_size == con.data.flen)
    //         {
    //             DEBUG << "write the full size for " << END;
    //             cout << con << endl;
    //             if (is_error(con.data))
    //                 connection_close(LINE, con.fd);
    //             else if (con.readbuff.empty())
    //                 connection_close(LINE, con.fd);
    //             else if (con.readbuff.length())
    //             {
    //                 DEBUG << "but there is remaining body" << END;
    //                 cout << con.readbuff << endl;
    //                 con.data = (Data){};
    //                 con.action = READ_;
    //                 con.read_fd = con.fd;
    //                 con.write_fd = con.fd;
    //                 con.write_size = 0;
    //                 if (parse_header(con))
    //                     connection_set_response(LINE,con);

    //                 // TODO: if body doesnt starts with method uri http_version -> bad request

    //                 cout << con << endl;
    //                 // throw Error(LINE, "");
    //             }
    //         }
    //         else
    //             con.action = READ_;

    //         if (w == 0) // TODO: this condition should never be true
    //         {
    //             DEBUG << con << endl;
    //             DEBUG << "did write " << w << " bytes" /*<<"<" << buff << ">"*/ << endl;
    //             // throw Error(LINE, "reached end of writing");
    //         }
    //     }
    // }
    else
        throw Error("write to unknow method");
}

bool connection_timeout(Connection &con)
{
    bool cond = false;
    if (con.data.keep_alive)
        cond = time(NULL) - con.timeout > KEEP_ALIVE_TIMEOUT;
    else
        cond = time(NULL) - con.timeout > SOCK_TIMEOUT;
    if (cond)
        cout << RED "timeout" << END;

    return cond;
}

void see_pfds_size()
{
    cout << "pfds.size: " << pfds.size() << endl;
    for (size_t i = 0; i < pfds.size(); i++)
    {
        cout << "   " << pfds[i].fd << ":" << to_string(types[pfds[i].fd]) << endl;
        if (cons.count(pfds[i].fd) && to_string(types[pfds[i].fd]) != "SERVER")
            cout << "   " << cons[pfds[i].fd] << endl;
    }
    cout << endl;
}

void WebServer(mServers &srvs)
{
    // SIGNALS
    struct sigaction sa = {};
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);

    // INIT SERVERS
    size_t servs_size = 0;
    mServers::iterator it = srvs.begin();
    while (it != srvs.end())
    {
        size_t i = 0;
        while (i < it->second.size())
        {
            if (create_server_connection(it->second[i]) == 0)
                servs_size++;
            i++;
        }
        it++;
    }
    if (servs_size == 0)
        throw Error("failed to create servers");

#if 1
    // WEBSERVING
    ssize_t timing = -1;
    cout << "Start webserver" << endl;
    while (1)
    {
        timing = (timing + 1) % 10000;
        if (!timing)
            see_pfds_size();
        int ready = poll(pfds.data(), pfds.size(), -1);
        if (ready < 0)
        {
            ERROR << "poll failed" << RESET;
            continue;
        }
        else if (ready >= 0)
        {
            ssize_t i = 0;
            while (i < pfds.size())
            {
                if (pfds[i].revents & POLLERR)
                {
                    // ERROR << "==============================================================" << END;
                    ERROR << "POLLERR in " << pfds[i].fd << RESET;
                    connection_close(LINE, pfds[i].fd);
                    // ERROR << "==============================================================" << END;
                    i = 0;
                    // throw Error("");
                    continue;
                }
                else if (i < servs_size && pfds[i].revents & POLLIN)
                    connection_accept(pfds[i].fd);
                else if (i >= servs_size)
                {
                    Connection &con = connection_find(pfds[i].fd);
                    if (pfds[i].revents & POLLIN && con.action == READ_ && pfds[i].fd == con.read_fd)
                        connection_read(con, pfds[i].fd);
                    else if (pfds[i].revents & POLLOUT && con.action == WRITE_ && pfds[i].fd == con.write_fd)
                        connection_write(con, pfds[i].fd);
                    // else if (connection_timeout(con))
                    // {
                    //     ERROR << "timeout 1 on " << con.fd << RESET;
                    //     connection_close(LINE, con.fd);
                    // }
                }
                i++;
            }
        }
        // else if(ready == 0)
        // {
        ssize_t i = servs_size;
        while (i < pfds.size())
        {
            if (cons.count(pfds[i].fd) || pairs.count(pfds[i].fd))
            {
                Connection &con = connection_find(pfds[i].fd);
                if (connection_timeout(con) && con.data.status != HTTP_TIMEOUT)
                {
#if 0
                    connection_close(LINE, con.fd);
#else
                    con.data.status = HTTP_TIMEOUT;
                    if (!con.data.matched_location)
                        connection_set_response(LINE, con);
                    con.timeout = time(NULL);
                    con.data.header_parsed = true;
                    cout << con << endl;
                    // throw Error("");
                    // generate_response(LINE, con, *con.data.matched_location);
#endif
                }
                else if (connection_timeout(con) && con.data.status == HTTP_TIMEOUT)
                    connection_close(LINE, con.fd);
            }
            i++;
        }
        // }
    }
#endif
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
            // printf("address: %s\n", host);
        }
    }
    freeifaddrs(ifaddr);
}
// TODO: ulimit is 63282
// TODO: handle this case siege -t 60s -b http://127.0.0.1:17000
int main(int argc, char **argv)
{
    try
    {
        if (argc != 2)
            throw Error(string("Invalid number of arguments"));
        set_machine_ip_address();
        mServers mservs = parse_config(argv[1]);
        init_memetypes();
        WebServer(mservs);
    }
    catch (exception &err)
    {
        cerr << RED "Error: " << err.what() << RESET << endl;
    }
}