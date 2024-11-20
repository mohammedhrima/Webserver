#include "header.hpp"

// config location alwaays starts with / and ends with /
// TODO: http://127.0.0.1:17000/retjy
bool match_location(string &conf_location, string uri)
{
    size_t i = 0;
    while (i < conf_location.length() && i < uri.length() && conf_location[i] == uri[i])
        i++;
    return (
        (i == conf_location.length() && (i == uri.length() || uri[i - 1] == '/')) ||
        (i == conf_location.length() - 1 && i == uri.length()));
}

void set_status(int line, string &path, Connection &con)
{
    DEBUG << "set status from path " << path << END;
    DEBUG << con << END;
    // check allowed mathod here
    con.data.status = HTTP_NOT_FOUND;
    con.data.st = (state){};
    // TODO: this may cause segvault
    Location &location = *con.data.matched_location;
    if (!location.methods[con.data.method])
    {
        con.data.status = HTTP_METHOD_NOT_ALLOWED;
        return;
    }
    // int
    if (access(path.c_str(), F_OK) != 0)
    {
        con.data.status = HTTP_NOT_FOUND;
        con.data.cause = path;
        return;
    }
    if (con.data.method == GET_)
    {
        if (access(path.c_str(), R_OK) != 0)
        {
            con.data.status = HTTP_FORBIDEN;
            con.data.cause = path;
        }
        else
        {
            if (stat(path.c_str(), &con.data.st) == 0)
            {
                if (con.data.st.st_mode & IS_FILE)
                    con.data.status = HTTP_OK;
                else if (con.data.st.st_mode & IS_DIR)
                {
                    if (con.data.uri[con.data.uri.length() - 1] == '/')
                        con.data.status = HTTP_OK;
                    else
                        con.data.status = HTTP_MOVE_PERMANENTLY;
                }
            }
            else
            {
                con.data.status = HTTP_INTERNAL_SERVER;
                con.data.cause = "stat failed";
            }
        }
        return;
    }
    if (con.data.method == DELETE_)
    {
        if (access(path.c_str(), W_OK) != 0)
        {
            con.data.status = HTTP_FORBIDEN;
            con.data.cause = path;
            return;
        }
        else
            con.data.status = HTTP_OK;
    }
}

string generate_header(Connection &con, string ctype, size_t size)
{
    Data &data = con.data;
    string version = to_string(data.http_version);
    if (version.empty())
        version = "HTTP/1.0";
    string header = version + " " + to_string((int)data.status) + " " + to_string(data.status) + "\r\n";
    header += (ctype.length() ? "Content-Type: " + ctype + "\r\n" : "");
    header += (size ? "Content-Length: " + to_string(size) + "\r\n" : "");
    // header += "Connection: close\r\n";
    // if (con.data.hosts.size())  // TODO: this may cause problem
    //     header += "Host: " + con.data.hosts[0] + "\r\n"; // to be checked

    if (is_move_dir(con.data))
        header += "Location: " + data.uri + "/\r\n";
    header += "\r\n";
    return header;
}

int open_error_page(Connection &con, string &path)
{
    // TODO: error page could be also empty handle it
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
    con.buff = header;
    data.flen = data.clen + con.buff.length();

    con.action = READ_;
    pairs[fd] = con.fd;
    con.read_fd = fd;
    con.write_fd = con.fd;
    con.write_size = 0;
    con.read_size = 0;

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
            data.status = HTTP_INTERNAL_SERVER;
            data.cause = "openning GET file " + path;
            return fd;
        }
        cout << "New GET file connection " << path << " has fd " << fd << " from client " << con.fd << endl;
        pfds.push_back((pollfd){.fd = fd, .events = POLLIN | POLLOUT});
        types[fd] = FILE_;

        string header = generate_header(con, data.ctype, data.st.st_size);
        data.clen = 0;
        con.buff = header;
        data.flen = data.st.st_size + con.buff.length();

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
            data.status = HTTP_INTERNAL_SERVER;
            data.cause = "openning POST file " + path;
            return fd;
        }
        cout << "New POST file connection " << path << " has fd " << fd << " from client " << con.fd << endl;
        pfds.push_back((pollfd){.fd = fd, .events = POLLIN | POLLOUT});
        types[fd] = FILE_;
        if (con.buff.length()) // if request has body post it then read
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

    con.buff = "<ul>\n";
    entry = readdir(dir);
    entry = readdir(dir);
    while ((entry = readdir(dir)) != NULL)
    {
        string url = "<a href=\"http://" +
                     clear_path(con.srv_listen + ":" +
                                to_string(con.srv_port) + "/" + con.data.uri + "/" + string(entry->d_name)) +
                     "\">" +
                     string(entry->d_name) + "</a><br>\n";
        con.buff += url;
    }
    closedir(dir);
    con.buff += "</ul>\n";
    con.data.clen = con.buff.length();
    con.action = WRITE_;
    con.buff = generate_header(con, "text/html", con.data.clen) + con.buff;
    con.data.flen = con.buff.length();
    con.read_fd = con.fd;
    con.write_fd = con.fd;
    con.write_size = 0;
    return 0;
}

void generate_response(int line, Connection &con)
{
    // TODO: this may segvault
    Location &location = *con.data.matched_location;
    Data &data = con.data;
    cout << "call generate response from line " << line << " " << (int)data.status << " " << to_string(data.method) << " " << endl;
    Status status = data.status;

    if (is_error(data))
    {
        con.action = WRITE_;
        con.write_fd = con.fd;
        con.write_size = 0;
        con.read_size = 0;
        con.buff = "";

        cout << LINE << " is error status: " << (int)data.status << " cause: " << data.cause << endl;
        cout << RED SPLIT RESET << endl;
        cout << location << endl;
        if (location.errors.count((int)data.status))
        {
            string path = clear_path(clear_path(location.root + "/" + location.errors[data.status]));
            cout << LINE << " check error page: " << path << endl;
            data.st = get_state(path);
            bool isaccess = (data.st.st_mode & IS_ACCESS) && (data.st.st_mode & IS_FILE);
            bool opened_succefully = isaccess && open_error_page(con, path) > 0;
            if (isaccess && opened_succefully)
            {
                ERROR << " response with openned error page" << END;
                return;
            }
        }
        cout << LINE << RED " response with generated error page: " RESET << endl;
        string response = "<div style=\"display:flex; justify-content:center;"
                          "align-items:center; color:blue; font-size: large;\"> Generated " +
                          to_string(status) + (data.cause.length() ? " because " + data.cause : "") + "</div>\n";

        string header = generate_header(con, "text/html", response.length());
        data.clen = response.length();

        con.buff = header + response;
        con.data.flen = con.buff.length();
        cout << con.buff << endl;

        con.read_fd = -1;
        con.write_fd = con.fd;
    }
    else if (data.method == POST_ && status == HTTP_CREATED)
    {
        con.action = WRITE_;
        string response = "<div style=\"display:flex; justify-content:center;"
                          "align-items:center; color:blue; font-size: large;\">"
                          "file created succefully"
                          "</div>\n";
        // to be checked, I change clen here
        data.clen = response.length();
        string header = generate_header(con, "text/html", data.clen);
        con.buff = header + response;
        con.data.flen = con.buff.length();
        con.write_size = 0;
        con.read_size = 0;
        con.action = WRITE_;
        con.write_fd = con.fd;
    }
    else if (is_move_dir(data))
    {
        con.action = WRITE_;
        data.clen = 0;
        string header = generate_header(con, "", data.clen);
        con.buff = header;

        data.flen = con.buff.length();
        con.write_size = 0;
        con.read_fd = con.fd;
        con.write_fd = con.fd;
    }
    else if (data.method == DELETE_ && status == HTTP_OK)
    {
        con.action = WRITE_;
        string response = "<div style=\"display:flex; justify-content:center;"
                          "align-items:center; color:blue; font-size: large;\">"
                          "Deleted succefully"
                          "</div>\n";
        data.clen = response.length();
        string header = generate_header(con, "text/html", data.clen);
        con.buff = header + response;
        data.flen = con.buff.length();
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
        data.status = HTTP_FORBIDEN;
        data.cause = "permission denied to delete " + path;
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

void find_match(Connection &con, string &match)
{
    vServer &vservs = servers[con.srv_port];
    Data &data = con.data;
    ssize_t i = 0;
    bool found_server = false;
    int round = 0;
    string serv_name;

    while (i < vservs.size())
    {
        Server &serv = vservs[i];
        if (con.srv_listen == serv.listen)
        {
            if (data.hosts.size() == 0) // case: timeout and request don't have hosts
            {
                data.matched_location = &serv.locations[""];
                check(LINE, con, !is_error(data), HTTP_NOT_FOUND, "location not found, hosts empty");
                return;
            }
            else
            {
                cout << SPLIT << endl;
                size_t j = 0;
                while (j < data.hosts.size())
                {
                    if (round == 0)
                        cout << "round " << round << ": compare host " << data.hosts[j] << " to " << serv.name;
                    if (round == 1)
                        cout << "round " << round;
                    cout << ", server_name: " << serv.name << endl;

                    bool cond = (round == 0 && serv.name == data.hosts[j]) || round == 1;
                    if (cond)
                    {
                        map<string, Location>::iterator it;
                        for (it = serv.locations.begin(); it != serv.locations.end(); it++)
                        {
                            string key = it->first;
                            // cout << "check location " << key << " with uri <" << data.uri << endl;
                            // keep it > to skip empty location
                            serv_name = serv.name;
                            if (match_location(key, data.uri) && key > match)
                            {
                                cout << GREEN "valid location: " << key << " has server_name " << serv.name << RESET << endl;
                                match = key;
                                data.matched_location = &it->second;
                                // DEBUG << *data.matched_location << END;
                            }
                        }
                        if (round == 0 && serv_name.length() && data.matched_location == NULL)
                        {
                            // throw Error("");
                            data.matched_location = &serv.locations[""];
                            serv_name = serv.name;
                            check(LINE, con, !is_error(data), HTTP_NOT_FOUND, "location not found");
                            DEBUG << con << END;
                            return;
                        }
                    }
                    j++;
                }
            }
        }
        // TODO: handle of host ot found
        i++;
        if (round < 2 && i == vservs.size() && data.matched_location == NULL)
        {
            i = 0;
            round++;
        }
    }
    cout << SPLIT << endl;
}

void connection_set_response(int line, Connection &con)
{
    string match;
    // Location *matched_location = NULL;
    Data &data = con.data;
    string serv_name;
    cout << "call set response from line " << line << endl;

    if (!servers.count(con.srv_port)) // TODO: to be removed
    {
        ERROR << con << END;
        throw Error(LINE, "server with port " + to_string(con.srv_port) + "not found");
    }
    if (data.matched_location == NULL)
        find_match(con, match);

    if (data.method && !is_error(data))
    {
        if (con.data.matched_location->methods[data.method] != on)
        {
            throw Error(LINE, "method not allowed");
            data.status = HTTP_METHOD_NOT_ALLOWED;
            return generate_response(LINE, con);
        }
        else if (data.method == GET_)
        {
            con.buff = ""; // empty the buffer
            con.read_size = 0;
            con.write_size = 0;

            if (con.data.matched_location->is_ret)
            {
                data.status = con.data.matched_location->ret_status;
                data.uri = con.data.matched_location->ret_location;
                return generate_response(LINE, con);
            }
            string match_uri = data.uri.length() > match.length()
                                   ? substr(LINE, data.uri, match.length(), data.uri.length())
                                   : "";
            string path = clear_path(con.data.matched_location->src + "/" + match_uri);
            DEBUG << "path is " << path << END;
            set_status(LINE, path, con);
            if (data.st.st_mode & IS_DIR)
            {
                cout << "found dir " << path << endl;
                if (is_move_dir(data))
                {
                    cout << con << endl;
                    generate_response(LINE, con);
                    DEBUG << con << END;
                    // throw Error("");
                    return;
                }
                else if (data.status == HTTP_OK)
                {
                    if (con.data.matched_location->autoindex != on)
                    {
                        data.status = HTTP_FORBIDEN;
                        return generate_response(LINE, con);
                    }
                    bool index_file_found = false;
                    if (con.data.matched_location->index.length())
                    {
                        string index_path = clear_path(path + "/" + con.data.matched_location->index);
                        state tmp_state = get_state(index_path);
                        if ((tmp_state.st_mode & IS_ACCESS) && (tmp_state.st_mode & IS_FILE) &&
                            access(index_path.c_str(), R_OK | F_OK) == 0)
                        {
                            index_file_found = true;
                            data.st = tmp_state;
                            path = index_path;
                        }
                    }
                    if (!index_file_found && open_dir(con, path) > 0)
                        return;
                }
            }
            if (data.st.st_mode & IS_FILE && data.status == HTTP_OK)
            {
                cout << "found file " << path << endl;
                // data.clen = data.st.st_size;
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
            if (con.data.matched_location->has_limit && con.data.clen > con.data.matched_location->limit)
            {
                con.data.status = HTTP_CONTENT_TO_LARGE;
                con.data.cause = "limit is " + to_string(con.data.matched_location->limit) + " but file has " + to_string(con.data.clen);
                return generate_response(LINE, con);
            }
            string match_uri = data.uri.length() > match.length()
                                   ? substr(LINE, data.uri, match.length(), data.uri.length())
                                   : "";
            string ext;
            if (data.ctype.length() && memetype.count(data.ctype))
                ext = memetype[data.ctype];
            string path = clear_path(con.data.matched_location->dest + "/" + match_uri + "/" + rand_name() + ext);
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
                    con.buff = header + response;
                    con.data.flen = con.buff.length();

                    con.write_size = 0;
                    con.read_size = 0;
                    con.action = WRITE_;
                    connection_close(LINE, con.write_fd, "posting empty file");
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
            string path = clear_path(con.data.matched_location->src + "/" + match_uri);
            set_status(LINE, path, con);
            // TODO: check permession with access
            Delete(data, path);
            if (data.status == HTTP_OK)
            {
                cout << "deleted succefully" << endl;
                return generate_response(LINE, con);
            }
        }
    }
    if (is_error(data))
    {
        generate_response(LINE, con);
        return;
    }
}

void connection_close(int line, int fd, string cause)
{
    cout << "line " << line << ": connection close " << fd << (cause.length() ? " cause: " + cause : "") << endl;
    if (cons.count(fd))
    {
        if (cons[fd].fd == fd)
        {
            if (cons[fd].read_fd != fd && cons[fd].read_fd > 0)
                connection_close(-1, cons[fd].read_fd, cause);
            if (cons[fd].write_fd != fd && cons[fd].write_fd > 0)
                connection_close(-1, cons[fd].write_fd, cause);
        }
        else if (cons[fd].read_fd == fd)
            cons[fd].read_fd = cons[fd].fd;
        else if (cons[fd].write_fd == fd)
            cons[fd].write_fd = -1;
        cons.erase(fd);
    }
    if (pairs.count(fd))
        pairs.erase(fd);
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
    char rbuff[BUFFSIZE] = {0};
    ssize_t r = read(fd, rbuff, BUFFSIZE);

    if (r < 0)
    {
        DEBUG << con << END;
        con.timeout = time(NULL);
        if (fd == con.fd)
            connection_close(LINE, con.fd, "read failed");
        else
        {
            con.data.status = HTTP_INTERNAL_SERVER;
            con.data.cause = "read failed";
            connection_set_response(LINE, con);
        }
    }
    else if (r > 0)
    {
        // TODO: check header to large
        con.read_size += r;
        con.timeout = time(NULL);
        con.buff.append(rbuff, r);
        DEBUG << "read " << r << " bytes rsize: " << con.read_size << " clen: " << con.data.clen << END;
        if (con.data.header_parsed)
        {
            con.action = WRITE_;
            if (con.read_size == con.data.clen)
            {
                if (fd != con.fd)
                    con.read_fd = -1;
                connection_close(LINE, fd, "con.read_size == con.data.clen");
            }
        }
        else if (!con.data.header_parsed && parse_header(con))
            connection_set_response(LINE, con);
    }
    else
    {
        DEBUG << "did read 0 byte " << END;
        // if this condition is uncommented, put the reason here
        if (con.data.method == 0 || con.data.method == POST_)
            connection_close(LINE, con.read_fd, "did read 0");
    }
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
    if (!is_error(con.data) && con.data.method == POST_ && con.data.flen - con.write_size < con.buff.length())
        w = write(fd, con.buff.c_str(), con.data.flen - con.write_size);
    else
        w = write(fd, con.buff.c_str(), con.buff.length());
    if (w < 0)
    {
        DEBUG << con << END;
        if (fd == con.fd)
            connection_close(LINE, con.fd, "write failed");
        else
        {
            con.timeout = time(NULL);
            con.data.status = HTTP_INTERNAL_SERVER;
            con.data.cause = "write failed";
            connection_set_response(LINE, con);
        }
    }
    else if (w > 0)
    {
        con.write_size += w;
        con.timeout = time(NULL);
        DEBUG << "write " << w << " bytes wsize: " << con.write_size << " flen:" << con.data.flen << END;
        // DEBUG << "buff: <" << con.buff << ">" << END;
        con.buff = substr(LINE, con.buff, w, con.buff.length());
        if (con.data.method == 0 && con.write_size == con.data.flen)
            connection_close(LINE, con.fd, "NO METHOD: write full size");
        else if (con.data.method == GET_)
        {
            if (con.write_size == con.data.flen)
            {
                // INFO << con << RESET;
                connection_close(LINE, con.fd, "GET: write full size");
            }
            else if (con.buff.empty())
                con.action = READ_;
        }
        else if (con.data.method == POST_)
        {
            if (con.write_size == con.data.flen)
            {
                if (fd != con.fd)
                {
                    connection_close(LINE, fd, "POST: finish posting");
                    con.data.status = HTTP_CREATED;
                    generate_response(LINE, con);
                }
                else
                    connection_close(LINE, fd, "POST: finish response ");
            }
            else if (con.buff.empty())
                con.action = READ_;
        }
        else if (con.data.method == DELETE_)
        {
            if (con.write_size == con.data.flen)
                connection_close(LINE, fd, "DELETE: write full size");
            else if (con.buff.empty())
                con.action = READ_;
        }
    }
    else
    {
        // DEBUG << "did write 0 byte , con: " << con << END;
    }
}

bool connection_timeout(Connection &con)
{
    bool cond = false;
    if (con.data.keep_alive)
        cond = time(NULL) - con.timeout > KEEP_ALIVE_TIMEOUT;
    else
        cond = time(NULL) - con.timeout > SOCK_TIMEOUT;
    if (cond)
        cout << RED "timeout on " << con << END;

    return cond;
}

void see_pfds_size()
{
    cout << "pfds.size: " << pfds.size() << endl;
    // for (size_t i = 0; i < pfds.size(); i++)
    // {
    //     cout << "   " << pfds[i].fd << ":" << to_string(types[pfds[i].fd]) << endl;
    //     // if (cons.count(pfds[i].fd) && to_string(types[pfds[i].fd]) != "SERVER")
    //     //     cout << "   " << cons[pfds[i].fd] << endl;
    // }
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
    cout << GREEN SPLIT RESET << endl;
    while (1)
    {
        timing = (timing + 1) % 100000;
        // if (!timing)
        //     see_pfds_size();
        int ready = poll(pfds.data(), pfds.size(), -1);
        if (ready < 0)
        {
            ERROR << "poll failed" << END;
            continue;
        }
        else if (ready >= 0)
        {
            ssize_t i = 0;
            while (i < pfds.size())
            {
                if (pfds[i].revents & POLLERR)
                {
                    // ERROR << SPLIT << END;
                    ERROR << "POLLERR in " << pfds[i].fd << END;
                    connection_close(LINE, pfds[i].fd, "POLLER");
                    // ERROR << SPLIT << END;
                    i = 0;
                    // see_pfds_size();
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
                }
                i++;
            }
        }
        // else if (ready == 0)
        {
            ssize_t j = servs_size;
            while (j < pfds.size())
            {
                if (cons.count(pfds[j].fd))
                {
                    Connection &con = cons[pfds[j].fd];
                    if (connection_timeout(con) && con.data.status != HTTP_TIMEOUT)
                    {
                        // cout << "found timeout 1" << endl;
                        con.data.status = HTTP_TIMEOUT;
                        connection_set_response(LINE, con);
                        con.timeout = time(NULL);
                    }
                    else if (connection_timeout(con) && con.data.status == HTTP_TIMEOUT)
                    {
                        // cout << "found timeout 2" << endl;
                        connection_close(LINE, con.fd, "timeout CLIENT");
                    }
                }
                else if (pairs.count(pfds[j].fd) && !cons.count(pairs[pfds[j].fd]))
                {
                    // TODO: test GET big file and cancel it while getting it
                    connection_close(LINE, pfds[j].fd, "timeout FILE");
                }
                j++;
            }
        }
    }
#endif
}
