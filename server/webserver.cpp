/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserver.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhrima <mhrima@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/07 16:53:13 by mhrima            #+#    #+#             */
/*   Updated: 2024/06/07 16:53:13 by mhrima           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.hpp"

void connection_close(int line, int fd, string cause)
{
    CLOG("close connection") << "from line " << line << " fd:" << fd << (cause.length() ? " cause: " + cause : "") << END;
    if (clients.count(fd))
    {
        if (clients[fd].fd == fd)
        {
            if (clients[fd].res.cgi.pid != 0)
            {
                GLOG("kill pid") << clients[fd].res.cgi.pid << endl;
                if (clients[fd].res.cgi.pid > 0)
                    kill(clients[fd].res.cgi.pid, SIGKILL);
                if (clients[fd].res.cgi.input.name.length())
                    remove(clients[fd].res.cgi.input.name.c_str());
                if (clients[fd].res.cgi.output.name.length())
                    remove(clients[fd].res.cgi.output.name.c_str());
            }
            if (clients[fd].read_fd != fd && clients[fd].read_fd > 0)
                connection_close(-1, clients[fd].read_fd, cause);
            if (clients[fd].write_fd != fd && clients[fd].write_fd > 0)
                connection_close(-1, clients[fd].write_fd, cause);
        }
        else if (clients[fd].read_fd == fd)
            clients[fd].read_fd = clients[fd].fd;
        else if (clients[fd].write_fd == fd)
            clients[fd].write_fd = -1;
        clients.erase(fd);
    }
    if (files.count(fd))
        files.erase(fd);
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

void set_fd(Connection &con, int fd, Action action, string cause)
{
    if (action == READ_)
    {
        CLOG("set fd") << "client " << con.fd << " read from " << fd << END;
        if (con.read_fd > 0 && con.read_fd != con.fd)
            connection_close(LINE, con.read_fd, cause);
        con.read_fd = fd;
    }
    else if (action == WRITE_)
    {
        CLOG("set fd") << "client " << con.fd << " write to " << fd << END;
        if (con.write_fd > 0 && con.write_fd != con.fd)
            connection_close(LINE, con.write_fd, cause);
        con.write_fd = fd;
    }
}

void handle_body(Connection &con)
{
    if (!con.req.header.conds["parsed"] || con.req.header.conds["body_checked"])
        return;
    GLOG("handle body") << endl;

    bool cond = (con.req.header.method == GET_ || con.req.header.method == DELETE_);
    cond = cond && (con.req.header.content_len || con.req.header.transfer == "chunked");
    if (cond)
    {
        if (con.req.header.transfer == "chunked")
        {
            if (con.req.header.content_len == 0)
            {
                size_t pos1 = con.req.buffer.find("\r\n", 0);
                if (pos1 == string::npos)
                {
                    con.action = READ_;
                    return;
                }
                size_t pos2 = con.req.buffer.find("\r\n", pos1 + 2);
                if (pos2 == string::npos)
                {
                    con.action = READ_;
                    return;
                }
                string val = substr(LINE, con.req.buffer, pos1 + 2, pos2);

                char *ptr = NULL;
                long chunk_size = strtol(val.c_str(), &ptr, 16);
                if (check_error(LINE, con, *ptr != '\0', HTTP_INTERNAL_SERVER, "parsing chunk size in" + string(FUNC)))
                {
                    RLOG("handle body") << "something went wrong parsing chunk size in " + string(FUNC) << END;
                    return;
                }
                con.req.header.content_len = chunk_size;
                if (chunk_size == 0)
                {
                    pos2 = con.req.buffer.find("\r\n", pos2 + 2);
                    if (pos2 == string::npos)
                    {
                        GLOG("handle body") << "didn't found the 2nd" << END;
                        con.action = READ_;
                        return;
                    }
                    con.req.header.conds["body_checked"] = true;
                }
                con.req.buffer = substr(LINE, con.req.buffer, pos2 + 2, con.req.buffer.length());
            }
            else
            {
                if (con.req.buffer.length() >= con.req.header.content_len)
                {
                    con.req.buffer = substr(LINE, con.req.buffer, con.req.header.content_len, con.req.buffer.length());
                    con.req.header.content_len = 0;
                }
                else
                {
                    con.req.header.content_len -= con.req.buffer.length();
                    con.req.buffer.clear();
                    con.action = READ_;
                }
                GLOG("remaining") << "chunk-size: " << con.req.header.content_len << endl;
            }
        }
        else
        {
            if (con.req.buffer.length() >= con.req.header.content_len)
            {
                con.req.buffer = substr(LINE, con.req.buffer, con.req.header.content_len, con.req.buffer.length());
                con.req.header.content_len = 0;
            }
            else
            {
                con.req.header.content_len -= con.req.buffer.length();
                con.req.buffer.clear();
                con.action = READ_;
            }
            GLOG("remaining") << "content-length: " << con.req.header.content_len << endl;
            if (con.req.header.content_len == 0)
                con.req.header.conds["body_checked"] = true;
        }
    }
    else
        con.req.header.conds["body_checked"] = true;
}

bool match_location(string &conf_location, string uri)
{
    size_t i = 0;
    while (i < conf_location.length() && i < uri.length() && conf_location[i] == uri[i])
        i++;
    return (
        (i == conf_location.length() && (i == uri.length() || (i > 0 && uri[i - 1] == '/'))) ||
        (i == conf_location.length() - 1 && i == uri.length()));
}

void find_location(Connection &con, string &match)
{
    vServer &vservs = servers[con.server_port];
    string name;
    size_t i = 0;
    int round = 0;

    while (i < vservs.size())
    {
        Server &serv = vservs[i];
        if (con.server_address == serv.listen)
        {
            if (con.req.header.hosts.empty() || round == 2) // case: timeout and request don't have hosts
            {
                con.res.loc = &serv.locations[""];
                check_error(LINE, con, true, HTTP_NOT_FOUND, "location not found");
                return;
            }
            else
            {
                size_t j = 0;
                Header &header = con.req.header;
                while (j < header.hosts.size())
                {
                    GLOG("server_name") << serv.name << endl;
                    if (round == 0)
                        GLOG("round " + to_string(round)) << "compare host " << header.hosts[j] << " to " << serv.name << endl;
                    if (round == 1)
                        GLOG("round " + to_string(round)) << endl;

                    bool cond = (round == 0 && serv.name == header.hosts[j]) || round == 1;
                    if (cond)
                    {
                        map<string, Location>::iterator it;
                        for (it = serv.locations.begin(); it != serv.locations.end(); it++)
                        {
                            string key = it->first;
                            name = serv.name;
                            if (match_location(key, header.uri) && key > match)
                            {
                                GLOG("valid location") << key << endl;
                                match = key;
                                con.res.loc = &it->second;
                            }
                        }
                        if (round == 0 && name.length())
                        {
                            if (check_error(LINE, con, con.res.loc == NULL, HTTP_NOT_FOUND, "location not found"))
                            {
                                con.res.loc = &serv.locations[""];
                                name = serv.name;
                            }
                            return;
                        }
                    }
                    j++;
                }
            }
        }
        i++;
        if (round < 2 && i == vservs.size() && con.res.loc == NULL)
        {
            i = 0;
            round++;
        }
    }
}

void set_status(int line, Connection &con, Path &path)
{
    // DEBUG << FUNC << " called from line " << line << " set status from path " << path.name << END;
    (void)line;
    con.res.status = HTTP_NOT_FOUND;
    path.state = (State){};
    if (access(path.name.c_str(), F_OK) != 0)
    {
        con.res.status = HTTP_NOT_FOUND;
        con.res.cause = path.name;
    }
    else if (con.req.header.method == GET_)
    {
        if (access(path.name.c_str(), R_OK) != 0)
        {
            con.res.status = HTTP_FORBIDEN;
            con.res.cause = "getting " + path.name;
        }
        else
        {
            if (stat(path.name.c_str(), &path.state) == 0)
            {
                if (path.state.st_mode & IS_FILE)
                    con.res.status = HTTP_OK;
                else if (path.state.st_mode & IS_DIR)
                {
                    if (con.req.header.uri[con.req.header.uri.length() - 1] == '/')
                        con.res.status = HTTP_OK;
                    else
                    {
                        con.res.status = HTTP_MOVE_PERMANENTLY;
                        con.res.header.uri = con.req.header.uri + "/";
                    }
                }
            }
            else
            {
                con.res.status = HTTP_INTERNAL_SERVER;
                con.res.cause = "stat failed";
            }
        }
    }
    else if (con.req.header.method == POST_)
    {
        if (access(path.name.c_str(), W_OK) != 0)
        {
            con.res.status = HTTP_FORBIDEN;
            con.res.cause = "posting to " + path.name;
        }
        else
        {
            if (stat(path.name.c_str(), &path.state) == 0)
            {
                if (path.state.st_mode & IS_FILE)
                    con.res.status = HTTP_CREATED;
                else if (path.state.st_mode & IS_DIR)
                {
                    string ext;
                    if (con.req.header.content_type.length() && memetype.count(con.req.header.content_type))
                        ext = memetype[con.req.header.content_type];
                    con.res.path.name = clear_path(path.name + "/" + rand_name() + ext);
                    con.res.status = HTTP_CREATED;
                }
            }
            else
            {
                con.res.status = HTTP_INTERNAL_SERVER;
                con.res.cause = "stat failed";
            }
        }
    }
    else if (con.req.header.method == DELETE_)
    {
        if (access(path.name.c_str(), W_OK) != 0)
        {
            con.res.status = HTTP_FORBIDEN;
            con.res.cause = "deleting " + path.name;
        }
        else
            con.res.status = HTTP_NO_CONTENT;
    }
}

void fetch_config(Connection &con)
{
    bool cond = !con.req.header.conds["parsed"] || con.req.header.conds["fetched"] || !con.req.header.conds["body_checked"];
    if (cond)
        return;
    GLOG("fetch config") << endl;
    // DEBUG << !con.req.header.conds["parsed"] << END;
    // DEBUG << con.req.header.conds["fetched"] << END;
    // DEBUG << con.req.header.conds["body_checked"] << END;

    con.req.header.conds["fetched"] = true;

    string match;
    if (con.res.loc == NULL)
        find_location(con, match);
    if (con.req.header.method && !is_error(con.res.status))
    {
        if (!con.res.loc->methods[con.req.header.method])
        {
            con.res.status = HTTP_METHOD_NOT_ALLOWED;
            con.res.cause = to_string(con.req.header.method);
            return;
        }
        else if (con.res.loc->ret_status)
        {
            con.res.status = con.res.loc->ret_status;
            con.res.header.uri = con.res.loc->ret_location;
            return;
        }
        else if (con.req.header.method == GET_)
        {
            string match_uri;
            if (con.req.header.uri.length() > match.length())
                match_uri = substr(LINE, con.req.header.uri, match.length(), con.req.header.uri.length());
            con.res.path.name = clear_path(con.res.loc->src + "/" + match_uri);
            set_status(LINE, con, con.res.path);
            if (con.res.path.state.st_mode & IS_DIR)
            {
                if (is_move_dir(con.res.status))
                    return;
                else if (con.res.status == HTTP_OK)
                {
                    if (con.res.loc->autoindex != on)
                    {
                        con.res.status = HTTP_FORBIDEN;
                        con.res.cause = "autoindex is off";
                        return;
                    }
                    if (con.res.loc->index.length())
                    {
                        con.res.path.name = clear_path(con.res.path.name + "/" + con.res.loc->index);
                        set_status(LINE, con, con.res.path);
                    }
                }
            }
        }
        else if (con.req.header.method == POST_)
        {
            if (con.res.loc->limit && con.res.loc->limit <= con.req.header.content_len)
            {
                con.res.status = HTTP_CONTENT_TO_LARGE;
                con.res.cause = "limit is " + to_string(con.res.loc->limit) +
                                " but file has " + to_string(con.req.header.content_len);
                return;
            }
            string match_uri;
            if (con.req.header.uri.length() > match.length())
                match_uri = substr(LINE, con.req.header.uri, match.length(), con.req.header.uri.length());

            // cout << "uri: " << con.req.header.uri << endl;
            // cout << "match: " << match << endl;
            // cout << "match uri: " << match_uri << endl;
            con.res.path.name = clear_path(con.res.loc->dest + "/" + match_uri);
            set_status(LINE, con, con.res.path);
        }
        else if (con.req.header.method == DELETE_)
        {
            string match_uri;
            if (con.req.header.uri.length() > match.length())
                match_uri = substr(LINE, con.req.header.uri, match.length(), con.req.header.uri.length());
            con.res.path.name = clear_path(con.res.loc->root + "/" + match_uri);
            set_status(LINE, con, con.res.path);
        }
    }
}

void generate_header(Connection &con)
{
    string version = to_string(con.res.header.version);
    if (version.empty())
        version = "HTTP/1.0";
    string header = version + " " + to_string((int)con.res.status) + " " + to_string(con.res.status) + "\r\n";
    header += (con.res.header.content_type.length() ? ("Content-Type: " + con.res.header.content_type + "\r\n") : "");
    header += (con.res.header.content_len ? ("Content-Length: " + to_string(con.res.header.content_len) + "\r\n") : "");
    for (size_t i = 0; i < con.req.header.cookies.size(); i++)
        header += "set-cookie:" + con.req.header.cookies[i] + "\r\n";
    if (con.req.header.method == POST_)
        header += "Access-Control-Allow-Origin:*\r\n";
    if (is_move_dir(con.res.status))
        header += "Location: " + con.res.header.uri + "\r\n";
    header += "\r\n";
    con.res.buffer = header;
}

void generate_html_page(Connection &con, string &msg)
{
    msg = "<div style=\"display:flex; justify-content:center;"
          "align-items:center; color:blue; font-size: large;\">" +
          msg +
          "</div>\n";
    con.res.header.content_type = "text/html";
    con.res.header.content_len = msg.length();
    generate_header(con);
    con.res.buffer += msg;
    con.res.full_size = con.res.buffer.length();
    con.action = WRITE_;
    con.res.read_size = 0;
    con.res.write_size = 0;
}

void open_file(Connection &con)
{
    if (is_error(con.res.status))
    {
        int fd = open(con.res.path.name.c_str(), O_RDONLY);
        if (fd < 0)
        {
            con.read_fd = -1;
            cerr << RED << "could not open error page" RESET << endl;
            return;
        }
        add_pfd(fd);

        size_t dot = con.res.path.name.find_last_of(".");
        string ext;
        if (dot != string::npos)
        {
            ext = substr(LINE, con.res.path.name, dot, con.res.path.name.length());
            if (memetype.count(ext))
                con.res.header.content_type = memetype[ext];
        }
        if (con.res.header.content_type.empty())
            con.res.header.content_type = "application/octet-stream";

        con.res.header.content_len = con.res.path.state.st_size;
        generate_header(con);
        con.res.full_size = con.res.path.state.st_size + con.res.buffer.length();
        con.res.read_size = 0;
        con.res.write_size = 0;

        set_fd(con, fd, READ_, "set source file for error page");
        set_fd(con, con.fd, WRITE_, "set destination file for error page");
        files[fd] = con.fd;
        CLOG("open file") << con.res.path.name << " (error) fd " << fd << " from client " << con.fd << END;

        if (con.res.path.state.st_size == 0)
        {
            connection_close(LINE, fd, "error page is empty");
            con.action = WRITE_;
        }
        else
            con.action = READ_;
    }
    else if (con.res.is_cgi)
    {
        if (con.res.path.name == con.res.cgi.output.name)
        {
            int fd = open(con.res.path.name.c_str(), O_RDONLY);
            if (fd < 0)
            {
                con.res.status = HTTP_INTERNAL_SERVER;
                con.res.cause = "openning CGI output " + con.res.path.name;
                return;
            }
            add_pfd(fd);

            size_t dot = con.res.path.name.find_last_of(".");
            string ext;
            if (dot != string::npos)
            {
                ext = substr(LINE, con.res.path.name, dot, con.res.path.name.length());
                if (memetype.count(ext))
                    con.res.header.content_type = memetype[ext];
            }
            if (con.res.header.content_type.empty())
                con.res.header.content_type = "application/octet-stream";

            con.res.full_size = con.res.path.state.st_size;
            con.res.read_size = 0;
            con.res.write_size = 0;

            set_fd(con, fd, READ_, "set source  cgi output");
            set_fd(con, con.fd, WRITE_, "set destination cgi output");
            files[fd] = con.fd;
            CLOG("open file") << con.res.path.name << " (CGI output) fd " << fd << " from client " << con.fd << END;

            if (con.res.path.state.st_size == 0)
            {
                connection_close(LINE, fd, "CGI output is empty");
                generate_header(con);
                con.res.full_size = con.res.buffer.length();
                con.action = WRITE_;
            }
            else
                con.action = READ_;
        }
        else if (con.res.path.name == con.res.cgi.input.name)
        {
            int fd = open(con.res.path.name.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
            if (fd < 0)
            {
                con.res.status = HTTP_INTERNAL_SERVER;
                con.res.cause = "openning CGI input file " + con.res.path.name;
                return;
            }
            add_pfd(fd);

            if (con.req.buffer.length()) // if request has body post it, then read
                con.action = WRITE_;
            else
                con.action = READ_;

            set_fd(con, con.fd, READ_, "set source file for cgi input");
            set_fd(con, fd, WRITE_, "set destination file for cgi input");
            files[fd] = con.fd;
            CLOG("open file") << con.res.path.name << " (CGI POST input) fd " << fd << " from client " << con.fd << END;

            con.res.read_size = 0;
            con.res.write_size = 0;
            con.res.full_size = con.req.header.content_len;
            if (con.req.header.content_len == 0 && con.req.header.transfer != "chunked")
                connection_close(LINE, fd, "CGI input is empty");
        }
    }
    else if (con.req.header.method == GET_)
    {
        int fd = open(con.res.path.name.c_str(), O_RDONLY);
        if (fd < 0)
        {
            con.res.status = HTTP_INTERNAL_SERVER;
            con.res.cause = "openning GET file " + con.res.path.name;
            return;
        }
        add_pfd(fd);

        size_t dot = con.res.path.name.find_last_of(".");
        if (dot != string::npos)
        {
            con.res.header.content_type = substr(LINE, con.res.path.name, dot, con.res.path.name.length());
            if (memetype.count(con.res.header.content_type))
                con.res.header.content_type = memetype[con.res.header.content_type];
        }
        if (con.res.header.content_type.empty())
            con.res.header.content_type = "application/octet-stream";

        con.res.header.content_len = con.res.path.state.st_size;
        generate_header(con);
        con.res.full_size = con.res.path.state.st_size + con.res.buffer.length();
        con.res.read_size = 0;
        con.res.write_size = 0;

        set_fd(con, fd, READ_, "set source for GET file");
        set_fd(con, con.fd, WRITE_, "set destination for GET file");
        files[fd] = con.fd;
        CLOG("open file") << con.res.path.name << " (GET) fd " << fd << " from client " << con.fd << END;

        if (con.res.path.state.st_size == 0)
        {
            connection_close(LINE, fd, "file is empty");
            con.action = WRITE_;
        }
        else
            con.action = READ_;
    }
    else if (con.req.header.method == POST_)
    {
        int fd = open(con.res.path.name.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (fd < 0)
        {
            con.res.status = HTTP_INTERNAL_SERVER;
            con.res.cause = "openning POST file " + con.res.path.name;
            return;
        }
        add_pfd(fd);

        if (con.req.buffer.length()) // if request has body post it, then read
            con.action = WRITE_;
        else
            con.action = READ_;

        set_fd(con, con.fd, READ_, "set source file for POST");
        set_fd(con, fd, WRITE_, "set destination file for POST");
        files[fd] = con.fd;
        CLOG("open file") << con.res.path.name << " (POST) fd " << fd << " from client " << con.fd << END;

        con.res.read_size = 0;
        con.res.write_size = 0;
        con.res.full_size = con.req.header.content_len;
        if (con.req.header.content_len == 0 && con.req.header.transfer != "chunked")
        {
            set_fd(con, con.fd, WRITE_, "empty POST file created");
            set_fd(con, -1, READ_, "empty POST file created");
            string response = "empty " + con.res.path.name + " created succefully";
            generate_html_page(con, response);
        }
    }
}

void open_dir(Connection &con)
{
    string buff;

    CLOG("open dir") << con.res.path.name << " from client " << con.fd << END;
    dirent *entry;
    DIR *dir = opendir(con.res.path.name.c_str());
    if (dir == NULL)
    {
        con.res.status = HTTP_INTERNAL_SERVER;
        con.res.cause = "openning " + con.res.path.name;
        return;
    }
    buff = "<ul>\n";
    while ((entry = readdir(dir)))
    {
        if (string(entry->d_name) == ".." || string(entry->d_name) == ".")
            continue;
        buff += "<a href=\"http://" +
                clear_path(con.server_address + ":" +
                           to_string(con.server_port) + "/" + con.req.header.uri + "/" + string(entry->d_name)) +
                "\">" +
                string(entry->d_name) + "</a><br>\n";
    }
    buff += "</ul>\n";
    closedir(dir);
    con.res.header.content_type = "text/html";
    con.res.header.content_len = buff.length();

    generate_header(con);
    con.res.buffer += buff;
    set_fd(con, con.fd, WRITE_, "set destination for GET dir");
    con.res.full_size = con.res.buffer.length();
    con.res.read_size = 0;
    con.res.write_size = 0;
    con.action = WRITE_;
}

void handle_move_dir(Connection &con)
{
    CLOG("set move dir") << con.res.header.uri << " from client " << con.fd << END;
    generate_header(con);
    set_fd(con, con.fd, WRITE_, "set destination for move dir");
    con.res.full_size = con.res.buffer.length();
    con.res.read_size = 0;
    con.res.write_size = 0;
    con.action = WRITE_;
}

void delete_(Connection &con, string path, bool &success_once)
{
    if (access(path.c_str(), F_OK) != 0)
    {
        con.res.status = HTTP_NOT_FOUND;
        con.res.cause = "NOT found " + path;
        return;
    }
    if (access(path.c_str(), W_OK) != 0)
    {
        con.res.status = HTTP_FORBIDEN;
        con.res.cause = "permission denied to delete " + path;
        return;
    }
    State state = get_state(path);
    if (state.st_mode & IS_FILE)
    {
        if (remove(path.c_str()) < 0)
        {
            con.res.status = HTTP_INTERNAL_SERVER;
            con.res.cause = "failed to delete file " + path;
        }
        else
            success_once = true;
    }
    else if (state.st_mode & IS_DIR)
    {
        dirent *entry;
        DIR *dir = opendir(path.c_str());
        if (dir == NULL)
        {
            con.res.status = HTTP_INTERNAL_SERVER;
            con.res.cause = "failed to open dir " + path;
            return;
        }
        while ((entry = readdir(dir)))
        {
            if (string(entry->d_name) == "." || string(entry->d_name) == "..")
                continue;
            delete_(con, path + "/" + string(entry->d_name), success_once);
        }
        closedir(dir);
        if (rmdir(path.c_str()) != 0)
        {
            con.res.status = HTTP_INTERNAL_SERVER;
            con.res.cause = "failed to delete dir " + path;
        }
        else
            success_once = true;
    }
}

void set_error(Connection &con)
{
    RLOG("set error") << to_string(con.res.status) + " " << (con.res.cause.length() ? con.res.cause : "") << endl;
    con.req = (Request){};

    if (con.res.loc && con.res.loc->errors.count((int)con.res.status))
    {
        con.res.path.name = clear_path(con.res.loc->root + "/" + con.res.loc->errors[(int)con.res.status]);
        con.res.path.state = get_state(con.res.path.name);
        bool cond = (con.res.path.state.st_mode & IS_ACCESS) && (con.res.path.state.st_mode & IS_FILE);
        if (cond)
            open_file(con);
        cond = cond && con.read_fd > 0;
        if (cond)
        {
            GLOG("set error") << "response with openned error page" << END;
            return;
        }
    }
    set_fd(con, con.fd, WRITE_, "setting error");
    set_fd(con, -1, READ_, "setting error");
    RLOG("set error") << "response with generated error page" << END;
    string response = "Generated: " + to_string(con.res.status) + " " + (con.res.cause.length() ? con.res.cause : "");
    generate_html_page(con, response);
}

void check_is_cgi(Connection &con)
{
    Location &loc = *con.res.loc;
    map<string, string>::iterator it = loc.cgi.begin();
    for (; it != loc.cgi.end(); it++)
    {
        if (ends_with(con.res.path.name, it->first))
        {
            con.res.cgi.exec = it->second;
            con.res.cgi.file.name = con.res.path.name;
            if (con.req.header.method == POST_)
                con.res.cgi.input.name = clear_path(con.res.loc->root + "/" + rand_name() + ".cgi_input");
            con.res.cgi.output.name = clear_path(con.res.loc->root + "/" + rand_name() + ".cgi_output");
            con.res.is_cgi = true;
            return;
        }
    }
    return;
}

void handle_cgi(Connection &con)
{
    con.res.cgi.pid = fork();
    if (con.res.cgi.pid == 0)
    {
        int out = -1;
        int in = -1;
        out = open(con.res.cgi.output.name.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0777);
        if (out < 0)
            exit(-1);
        vector<std::string> env;
        env.push_back("REQUEST_METHOD=" + to_string(con.req.header.method));
        env.push_back("SCRIPT_FILENAME=" + con.res.cgi.file.name);
        env.push_back("SCRIPT_NAME=" + con.req.header.uri);
        env.push_back("QUERY_STRING=" + con.req.header.queries);
        env.push_back("GATEWAY_INTERFACE=CGI/1.1");
        env.push_back("SERVER_PROTOCOL=" + to_string(con.req.header.version));
        env.push_back("SERVER_SOFTWARE=");
        env.push_back("REMOTE_ADDR=" + con.address);
        env.push_back("SERVER_NAME=" + con.server_address);
        env.push_back("SERVER_PORT=" + to_string(con.server_port));
        env.push_back("REDIRECT_STATUS=CGI");
        env.push_back("PATH_INFO=" + con.req.header.uri);
        if (con.req.header.cookies.size())
            env.push_back("HTTP_COOKIE=" + con.req.header.cookies[0]);

        GLOG("exec path") << con.res.cgi.exec << endl;
        GLOG("exec file") << con.res.cgi.file.name << endl;
        GLOG("cgi input") << con.res.cgi.input.name << endl;
        GLOG("cgi output") << con.res.cgi.output.name << endl;
        for (size_t i = 0; i < env.size(); i++)
            GLOG("env " + to_string(i)) << env[i] << endl;
        if (con.req.header.method == POST_)
        {
            State state = get_state(con.res.cgi.input.name);
            env.push_back("CONTENT_LENGTH=" + to_string(state.st_size));
            if (con.req.header.content_type.length())
                env.push_back("CONTENT_TYPE=" + con.req.header.content_type);
            in = open(con.res.cgi.input.name.c_str(), O_RDONLY);
            if (in < 0)
            {
                close(out);
                exit(-1);
            }
            if (dup2(in, STDIN) < 0)
            {
                close(in);
                close(out);
                exit(-1);
            }
        }
        if (dup2(out, STDOUT) < 0)
        {
            close(in);
            close(out);
            exit(-1);
        }
        std::vector<char *> envp(env.size() + 1);
        for (size_t i = 0; i < env.size(); i++)
            envp[i] = (char *)env[i].c_str();
        envp[env.size()] = NULL;
        char *arg[3];
        arg[0] = (char *)con.res.cgi.exec.c_str();
        arg[1] = (char *)con.res.cgi.file.name.c_str();
        arg[2] = NULL;
        execve(con.res.cgi.exec.c_str(), arg, envp.data());
        exit(-1);
    }
    else if (check_error(LINE, con, con.res.cgi.pid < 0, HTTP_BAD_GATEWAY, "fork failed"))
    {
    }
}

void set_response(Connection &con)
{
    bool cond = con.req.header.conds["response"] || !con.req.header.conds["fetched"];
    if (cond)
        return;
    GLOG("set response") << endl;

    con.req.header.conds["response"] = true;
    con.res.header.version = con.req.header.version;
    if (!is_error(con.res.status))
    {
        if (is_move_dir(con.res.status))
            handle_move_dir(con);
        else if (con.req.header.method == GET_)
        {
            if ((con.res.path.state.st_mode & IS_FILE) && con.res.status == HTTP_OK)
            {
                check_is_cgi(con);
                if (con.res.is_cgi)
                    handle_cgi(con);
                else
                    open_file(con);
            }
            else if (con.res.path.state.st_mode & IS_DIR && con.res.status == HTTP_OK)
                open_dir(con);
        }
        else if (con.req.header.method == POST_ && con.res.status == HTTP_CREATED)
        {
            check_is_cgi(con);
            if (con.res.is_cgi)
                con.res.path = con.res.cgi.input;
            open_file(con);
        }
        else if (con.req.header.method == DELETE_)
        {
            if (con.res.status == HTTP_NO_CONTENT)
            {
                bool success_once = false;
                delete_(con, con.res.path.name, success_once);
                if (success_once)
                {
                    con.res.status = HTTP_NO_CONTENT;
                    con.res.cause = "";
                    set_fd(con, con.fd, WRITE_, "Delete ended succefully");
                    string response = "Deleted succefully";
                    generate_html_page(con, response);
                }
            }
        }
    }
    if (is_error(con.res.status))
        set_error(con);
}

void read_from_client(Connection &con, bool refresh)
{
    char buff[BUFFSIZE];
    ssize_t r = recv(con.fd, buff, BUFFSIZE, MSG_DONTWAIT);

    if (r <= 0)
    {
        string msg = r < 0 ? " failed" : " 0 byte";
        con.remove = true;
        con.cause = string(FUNC) + msg;
    }
    else if (r > 0)
    {
        con.req.buffer.append(buff, r);
        con.req.read_size += r;
        con.timing = refresh ? time(NULL) : con.timing;
        CLOG("read from client") << r << " bytes, rsize: " << con.req.read_size << END;

        if (con.req.header.method == POST_ && con.req.header.conds["response"])
        {
            con.action = WRITE_;
        }
    }
}

void write_to_client(Connection &con)
{
    ssize_t w;
    w = send(con.write_fd, con.res.buffer.c_str(), con.res.buffer.length(), MSG_DONTWAIT);

    if (w <= 0)
    {
        string msg = w < 0 ? " failed" : " 0 byte";
        con.remove = true;
        con.cause = string(FUNC) + msg;
    }
    else if (w > 0)
    {
        con.res.buffer = substr(LINE, con.res.buffer, w, con.res.buffer.length());
        con.res.write_size += w;
        con.timing = time(NULL);
        CLOG("write to client") << w << " bytes, wsize: " << con.res.write_size << ", fsize: " << con.res.full_size << END;

        if (con.res.buffer.empty())
            con.action = READ_;

        if (con.res.write_size == con.res.full_size)
        {
            if (is_error(con.res.status) || con.req.buffer.empty())
            {
                con.remove = true;
                con.cause = "write full response";
            }
            else
            {
                set_fd(con, con.fd, READ_, "write full response but there is a remaing body");
                set_fd(con, -1, WRITE_, "write full response but there is a remaing body");
                con.req.header = (Header){};
                con.req.read_size = con.req.buffer.length();
                con.res = (Response){};
            }
        }
    }
}

void read_from_file(Connection &con)
{
    char rbuff[BUFFSIZE];
    ssize_t r = read(con.read_fd, rbuff, BUFFSIZE);

    if (check_error(LINE, con, r <= 0, HTTP_INTERNAL_SERVER, "read from file failed"))
    {
        string msg = r == 0 ? " 0 byte" : " failed";
        RLOG("read from file") << con.read_fd << msg << " from client " << con.fd << END;
        set_fd(con, -1, READ_, "read from file" + msg);
        set_fd(con, con.fd, WRITE_, "read from file" + msg);
    }
    else if (r > 0)
    {
        con.res.buffer.append(rbuff, r);
        con.res.read_size += r;
        con.timing = time(NULL);
        CLOG("read from file") << r << " bytes, rsize: " << con.res.read_size << ", fsize: " << con.res.path.state.st_size << END;

        con.action = WRITE_;
        if ((__off_t)con.res.read_size == con.res.path.state.st_size)
        {
            set_fd(con, con.fd, READ_, "read full size from file");
            if (con.res.is_cgi)
                remove(con.res.cgi.output.name.c_str());
        }
    }
}

void write_to_file(Connection &con)
{
    ssize_t w;
    ssize_t bytes;
    if (con.req.header.transfer == "chunked")
    {
        if (con.req.header.content_len == con.res.write_size)
        {
            size_t pos1 = con.req.buffer.find("\r\n", 0);
            if (pos1 == string::npos)
            {
                con.action = READ_;
                return;
            }
            size_t pos2 = con.req.buffer.find("\r\n", pos1 + 2);
            if (pos2 == string::npos)
            {
                con.action = READ_;
                return;
            }
            string val = substr(LINE, con.req.buffer, pos1 + 2, pos2);

            char *ptr = NULL;
            long chunk_size = strtol(val.c_str(), &ptr, 16);
            if (check_error(LINE, con, *ptr != '\0', HTTP_INTERNAL_SERVER, "parsing chunk size in" + string(FUNC)))
            {
                RLOG("write to file") << "something went wrong parsing chunk size" << END;
                return;
            }
            if (chunk_size == 0)
            {
                pos2 = con.req.buffer.find("\r\n", pos2 + 2);
                if (pos2 == string::npos)
                {
                    DEBUG << "didn't found the 2nd" << END;
                    con.action = READ_;
                    return;
                }
                con.req.buffer = substr(LINE, con.req.buffer, pos2 + 2, con.req.buffer.length());
                if (con.res.is_cgi)
                {
                    set_fd(con, -1, WRITE_, "POST: write full chunk for CGI input");
                    handle_cgi(con);
                }
                else
                {
                    set_fd(con, con.fd, WRITE_, "POST write full chunk size");
                    string response = "chunked " + con.res.path.name + " created succefully";
                    generate_html_page(con, response);
                }
                return;
            }
            con.res.write_size = 0;
            con.req.header.content_len = chunk_size;
            con.req.buffer = substr(LINE, con.req.buffer, pos2 + 2, con.req.buffer.length());
        }
        bytes = con.req.header.content_len - con.res.write_size;
        bytes = bytes < (ssize_t)con.req.buffer.length() ? bytes : (ssize_t)con.req.buffer.length();
    }
    else
        bytes = con.req.header.content_len < con.req.buffer.length() ? con.req.header.content_len : con.req.buffer.length();

    w = write(con.write_fd, con.req.buffer.c_str(), bytes);
    if (check_error(LINE, con, w <= 0, HTTP_INTERNAL_SERVER, "write to file failed"))
    {
        string msg = w == 0 ? " 0 byte" : " failed";
        RLOG("write to file") << con.write_fd << msg << " from client " << con.fd << END;
        set_fd(con, -1, READ_, "write to file" + msg);
        set_fd(con, con.fd, WRITE_, "write to file" + msg);
    }
    else if (w > 0)
    {
        con.req.buffer = substr(LINE, con.req.buffer, w, con.req.buffer.length());
        con.res.write_size += w;
        con.timing = time(NULL);
        CLOG("write to file") << w << " bytes, wsize: " << con.res.write_size << ", fsize: " << con.res.full_size << END;

        if (con.req.buffer.empty())
            con.action = READ_;
        if (con.req.header.method == POST_)
        {
            if (con.req.header.transfer == "chunked")
            {
                DEBUG << FUNC << " " << w << " bytes wsize: " << con.res.write_size << " limit: " << con.res.loc->limit << END;
                bool cond = con.res.loc->limit && con.res.loc->limit <= con.res.write_size;
                string msg = "limit is " + to_string(con.res.loc->limit) + " but chunk wrote " + to_string(con.res.write_size);
                if (check_error(LINE, con, cond, HTTP_CONTENT_TO_LARGE, msg))
                {
                    // con.req.header.conds["response"] = false;
                    return;
                }
            }
            else if (con.res.write_size == con.res.full_size)
            {
                if (con.res.is_cgi)
                {
                    set_fd(con, -1, WRITE_, "POST: write full size for CGI input");
                    handle_cgi(con);
                }
                else
                {
                    set_fd(con, con.fd, WRITE_, "POST write full size");
                    string response = con.res.path.name + " created succefully";
                    generate_html_page(con, response);
                }
            }
        }
    }
}

bool connection_timeout(Connection &con)
{
    // cout << "check timeout for " << con << " " << time(NULL) - con.timeout << endl;
    bool cond = false;
    // if (con.remove)
    //     cond = time(NULL) - con.timeout > SOCK_REMOVE_TIMEOUT;
    // else
    // else
    if (con.res.is_cgi)
        cond = time(NULL) - con.timing > CGI_TIMEOUT;
    else if (con.req.header.connection == "keep-alive")
        cond = time(NULL) - con.timing > KEEP_ALIVE_TIMEOUT;
    else
        cond = time(NULL) - con.timing > SOCK_TIMEOUT;
    if (cond)
        RLOG("timeout") << "on " << con << END;
    return cond;
}

bool is_first_response_line(Connection &con, string line)
{
    // RLOG("") << LINE << "<" << line << ">" << END;
    if (!starts_with(line, "HTTP/"))
        return false;
    line = line.substr(5);
    size_t pos = line.find(".");

    // RLOG("") << LINE << "<" << line << ">" << END;
    if (pos == string::npos)
        return false;

    string major_version = line.substr(0, pos);
    // RLOG("") << LINE << major_version << END;
    if (!_isdigits(major_version))
        return false;

    line = line.substr(pos + 1);
    pos = line.find(" ");
    // RLOG("") << LINE << "<" << line << ">" << END;
    if (pos == string::npos)
        return false;

    string minor_version = line.substr(0, pos);
    // RLOG("") << LINE << "<" << line << ">" << END;
    if (!_isdigits(minor_version))
        return false;

    line = line.substr(pos + 1);
    pos = line.find(" ");
    // RLOG("") << LINE << "<" << line << ">" << END;
    if (pos == string::npos)
        return false;

    string status_code = line.substr(0, pos);
    // RLOG("") << LINE << "<" << line << ">" << END;
    if (!_isdigits(status_code))
        return false;

    line = line.substr(pos + 1);
    string status_message = line;
    long major = atol(major_version.c_str());
    long minor = atol(minor_version.c_str());
    if (status_message.find(" ") == string::npos)
        return false;

    string message = to_string((Status)atol(status_code.c_str()));
    if (message.empty())
        message = status_message;

    if (major == 1 && minor == 0)
        con.res.cgi.header["cgi header"] = "HTTP/1.0 " + status_code + " " + message + "\n";
    else if (major == 1 && minor == 1)
        con.res.cgi.header["cgi header"] = "HTTP/1.1 " + status_code + " " + message + "\n";
    else
        con.res.cgi.header["cgi header"] = to_string(con.req.header.version) + " " + status_code + " " + message + "\n";
    return true;
}

void set_key_value(Connection &con, string &line)
{
    size_t pos = line.find(":");
    if (check_error(LINE, con, pos == string::npos, HTTP_BAD_GATEWAY, "Invalid header line " + line))
        return;
    string key = _tolower(trim(line.substr(0, pos)));
    string value = trim(line.substr(pos + 1));
    if (check_error(LINE, con, key.empty(), HTTP_BAD_GATEWAY, "header has empty key"))
        return;
    if (check_error(LINE, con, value.empty(), HTTP_BAD_GATEWAY, "header " + key + " empty"))
        return;
    if (check_error(LINE, con, con.res.cgi.header.count(key), HTTP_BAD_GATEWAY, "repeatead key: " + key))
        return;
    // RLOG("set key") << key << " with value " << value << END;
    if (key == "set-cookie")
        con.res.cgi.cookies.push_back(value);
    else
        con.res.cgi.header[key] = value;
    return;
}

void parse_cgi_header(Connection &con, string header)
{
    string sep;
    size_t pos = find_sep(header, sep);
    while (pos != string::npos)
    {
        string line = header.substr(0, pos);
        // RLOG("") << "<" << line << ">" << END;
        if (con.res.cgi.header["cgi header"].empty())
        {
            if (!is_first_response_line(con, header))
            {
                con.res.cgi.header["cgi header"] = to_string(con.req.header.version) + " 200 OK\n";
                set_key_value(con, line);
            }
        }
        else if (line.length())
            set_key_value(con, line);
        if (is_error(con.res.status))
            break;
        header = header.substr(pos + sep.length());
        pos = find_sep(header, sep);
    }
    // cout << con.res.cgi.header["cgi header"] << endl;
}

void set_cgi_header(Connection &con)
{
    /*
        + add first line header
        + content-length
        + content-type
        + body is full file
    */
    con.res.cgi.header["cgi header"] = "HTTP/1.1 200 OK\n";
    con.res.cgi.header["content-type"] = "application/octet-stream";
    con.res.cgi.header["content-length"] = to_string(con.res.path.state.st_size);

    string res_header = con.res.cgi.header["cgi header"];
    map<string, string>::iterator it = con.res.cgi.header.begin();
    for (; it != con.res.cgi.header.end(); it++)
        if (it->first != "cgi header")
            res_header += it->first + ":" + it->second + "\n";
    for (size_t i = 0; i < con.res.cgi.cookies.size(); i++)
        res_header += "set-cookie: " + con.res.cgi.cookies[i] + "\n";
    res_header += "\n";

    // CLOG("") << "header is <" << res_header << ">" << endl;
    // CLOG("") << "body is <" << con.res.buffer << ">" << endl;

    // cout << "content-len should be: " << con.res.path.state.st_size - (con.res.read_size - buff.length()) << endl;
    con.res.full_size = res_header.length() + con.res.path.state.st_size;
    con.res.buffer = res_header + con.res.buffer;
    con.res.header.conds["cgi_header_parsed"] = true;
    con.action = WRITE_;
}

void handle_cgi_output(Connection &con)
{
    if (!con.res.is_cgi || con.res.cgi.pid != -1 || is_error(con.res.status) || con.res.buffer.empty() || con.res.header.conds["cgi_header_parsed"])
        return;
    // string sep;
    // size_t pos = find_sep(con.res.buffer, sep);
    // string &buff = con.res.buffer;
    // string line;

    // RLOG("") << con.res.read_size << END;
    // RLOG("") << con.res.cgi.output.state.st_size << END;

    string sep = "\r\n\r\n";
    size_t pos = find(con.res.buffer, (char *)sep.c_str());
    if (pos == string::npos)
    {
        sep = "\n\n";
        pos = find(con.res.buffer, (char *)sep.c_str());
    }
    if (pos != string::npos)
    {
        GLOG("handle cgi output") << "parse cgi header" << END;
        string header = con.res.buffer.substr(0, pos + sep.length());
        con.res.buffer = con.res.buffer.substr(pos + sep.length());
        parse_cgi_header(con, header);
        if (!is_error(con.res.status))
        {
            if (!con.res.cgi.header.count("content-type"))
                con.res.cgi.header["content-type"] = "application/octet-stream";
            con.res.cgi.header["content-length"] = to_string(con.res.path.state.st_size - (con.res.read_size - con.res.buffer.length()));

            string res_header = con.res.cgi.header["cgi header"];
            map<string, string>::iterator it = con.res.cgi.header.begin();
            for (; it != con.res.cgi.header.end(); it++)
                if (it->first != "cgi header")
                    res_header += it->first + ":" + it->second + "\n";
            for (size_t i = 0; i < con.res.cgi.cookies.size(); i++)
                res_header += "set-cookie: " + con.res.cgi.cookies[i] + "\n";
            res_header += "\n";

            // CLOG("") << "header is <" << res_header << ">" << endl;
            // CLOG("") << "body is <" << con.res.buffer << ">" << endl;

            // cout << "content-len should be: " << con.res.path.state.st_size - (con.res.read_size - buff.length()) << endl;
            con.res.path.state.st_size = atol(con.res.cgi.header["content-length"].c_str());
            con.res.full_size = res_header.length() + con.res.path.state.st_size;
            con.res.buffer = res_header + con.res.buffer;

            con.res.header.conds["cgi_header_parsed"] = true;
            con.action = WRITE_;
        }
    }
    else if ((__off_t)con.res.read_size == con.res.cgi.output.state.st_size || con.res.read_size > BUFFSIZE)
    {
        GLOG("handle cgi output") << "set cgi header" << END;
        set_cgi_header(con);
    }
    else
        con.action = READ_;
}

void connection_handle(pollfd pfd)
{
    if (clients.count(pfd.fd))
    {
        Connection &client = clients[pfd.fd];
        if (client.res.is_cgi && client.res.cgi.pid > 0 && !is_error(client.res.status))
        {
            int status = 0;
            int val = waitpid(client.res.cgi.pid, &status, WNOHANG);
            if (check_error(LINE, client, val < 0, HTTP_BAD_GATEWAY, "waitpid failed"))
                pfd.revents = POLLIN;
            else if (val > 0)
            {
                if (check_error(LINE, client, !WIFEXITED(status) || status < 0, HTTP_BAD_GATEWAY, "child failed"))
                    pfd.revents = POLLIN;
                else if (status == 0)
                {
                    pfd.revents = POLLIN;
                    client.res.cgi.pid = -1;
                    client.res.cgi.output.state = get_state(client.res.cgi.output.name);
                    client.res.path = client.res.cgi.output;
                    open_file(client);
                }
            }
        }
        else if (pfd.revents & POLLIN)
        {
            if (client.action == READ_ && client.read_fd == pfd.fd)
                read_from_client(client, true);
            else if (client.action == WRITE_ && client.fd == pfd.fd)
                read_from_client(client, false);
        }
        else if (pfd.revents & POLLOUT && client.action == WRITE_ && client.write_fd == pfd.fd)
            write_to_client(client);
        if (!client.remove && connection_timeout(client))
        {
            if (client.res.is_cgi && client.res.status != HTTP_GATEWAY_TIMEOUT)
            {
                client.res.status = HTTP_GATEWAY_TIMEOUT;
                client.res.cause = "CGI timeout";
                pfd.revents = POLLIN;
                client.req.header.conds["body_checked"] = true;
                client.req.header.conds["fetched"] = true;
                client.req.header.conds["response"] = false;
                client.timing = time(NULL);
                kill(client.res.cgi.pid, SIGKILL);
                client.res.cgi.pid = -1;
            }
            else if (!client.res.is_cgi && client.res.status != HTTP_TIMEOUT)
            {
                client.res.status = HTTP_TIMEOUT;
                client.res.cause = "timeout";
                pfd.revents = POLLIN;
                client.req.header.conds["body_checked"] = true;
                client.req.header.conds["fetched"] = true;
                client.req.header.conds["response"] = false;
                client.timing = time(NULL);
            }
            else
            {
                client.cause = "timeout";
                client.remove = true;
            }
        }
        if (client.remove)
            connection_close(LINE, client.fd, client.cause);
        else
        {
            if (pfd.revents)
            {
                parse_header(client, client.req.buffer, client.req.header);
                handle_body(client);
                fetch_config(client);
                set_response(client);
            }
        }
    }
    else if (files.count(pfd.fd))
    {
        int file_fd = pfd.fd;
        if (!clients.count(files[file_fd]))
            return;
        Connection &client = clients[files[file_fd]];
        if (pfd.revents & POLLIN && client.action == READ_ && client.read_fd == pfd.fd)
            read_from_file(client);
        else if (pfd.revents & POLLOUT && client.action == WRITE_ && client.write_fd == pfd.fd)
            write_to_file(client);
        if (pfd.revents)
            handle_cgi_output(client);
    }
}

void WebServer()
{
    // fd_ = open("random.txt", O_WRONLY | O_CREAT | O_TRUNC, 0777);
    // SIGNALS
    struct sigaction sa = {};
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);

    // INIT SERVERS
    size_t servs_size = 0;
    map<ssize_t, vServer>::iterator it = servers.begin();
    while (it != servers.end())
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

    // WEBSERVING
    ssize_t timing = -1;
    GLOG("Start webserver") << endl;

    while (1)
    {
        timing = (timing + 1) % 100000;
        if (!timing)
            see_pfds_size();
        int ready = poll(pfds.data(), pfds.size(), POLLTIMEOUT);
        if (ready < 0)
        {
            throw Error("POLL FAILED");
            continue;
        }
        if (ready == 0)
            continue;
        size_t i = 0;
        while (i < pfds.size())
        {
            pollfd pfd = pfds[i];
            if (pfd.revents & POLLERR)
                connection_close(LINE, pfd.fd, "POLLER");
            else if (pfd.revents & POLLRDHUP)
                connection_close(LINE, pfd.fd, "POLLRDHUP");
            else if (pfd.revents & POLLHUP)
                connection_close(LINE, pfd.fd, "POLLHUP");
            else if (i < servs_size && pfd.revents & POLLIN)
                connection_accept(pfd.fd);
            else if (i >= servs_size)
                connection_handle(pfd);

            if (i < pfds.size() && pfd.fd == pfds[i].fd) // in case removing a file
                i++;
        }
    }
}