#include "header.hpp"

map<string, string> memetype;
map<int, Connection *> tmp_cons;

map<int, int> pairs;
map<int, Connection> cons;
vector<Connection> cons_vec;
vector<pollfd> pfds;
size_t pos;
size_t line = 1;
map<int, Type> types;

Connection new_connection(int fd, Type type, size_t port, string &srv_addr)
{
    Connection con = {};
    con.init();
    con.type = type;
    con.data.port = port;
    con.read_fd = -1;
    con.write_fd = -1;
    if (con.type == SERVER_)
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
        addr.sin_port = htons(port);
        if (bind(fd, (sockaddr *)&addr, sizeof(addr)))
        {
            cerr << RED "bind failed" RESET << endl;
            con.fd = -1;
            return con;
        }
        if (listen(fd, LISTEN_LEN))
        {
            cerr << RED "listen failed" RESET << endl;
            con.fd = -1;
            return con;
        }
        con.fd = fd;
        pfds.push_back((pollfd){.fd = con.fd = fd, .events = POLLIN | POLLOUT});
        // cout << "New server connection has fd " << con.fd << " " << endl;
        con.read_fd = con.fd;
        pairs[con.fd] = con.fd;
        types[con.fd] = SERVER_;
    }
    else if (con.type == CLIENT_)
    {
        sockaddr_storage addr;
        socklen_t len = sizeof(addr);
        con.fd = accept(fd, (sockaddr *)&addr, &len);
        con.data.srv_address = srv_addr;
        if (con.fd < 0)
        {
            cerr << RED << "Failed to accept neq connection";
            con.fd = -1;
            return con;
        }
        else if (con.fd > 0)
        {
            inet_ntop(AF_INET, &(((struct sockaddr_in *)&addr)->sin_addr), con.address, INET_ADDRSTRLEN);
            pfds.push_back((pollfd){.fd = con.fd, .events = POLLIN | POLLOUT});
            int flags = fcntl(con.fd, F_GETFL, 0);
            if (flags == -1)
            {
                cerr << RED "fcntl 0: failed" RESET << endl;
                con.fd = -1;
                return con;
            }
            flags |= O_NONBLOCK;
            if (fcntl(con.fd, F_SETFL, flags) < 0)
            {
                cerr << RED "fcntl 1: failed" RESET << endl;
                con.fd = -1;
                return con;
            }
            // cout << "New client connection has fd " << con.fd << " has addr " << con.address << endl;
            // cout << con_state(con) << endl;
            con.read_fd = con.fd;
            pairs[con.fd] = con.fd;
            types[con.fd] = CLIENT_;
        }
    }
    con.data.update_timeout();
    return con;
}

state get_state(string &path)
{
    state state_;
    stat(path.c_str(), &state_);
    return state_;
}

void generate_response(int line, Connection &con)
{
    Data &data = con.data;
    // cout << "call generate response from line " << line << " " << (int)data.status << " " << to_string(data.method) << " " << endl;
    Status status = data.status;

    // TODO: check all status and set here the values
    if (data.is_error())
    {
        data.action = WRITE_;
        con.write_fd = con.fd;

        // cout << LINE << " is error status: " << (int)data.status << " cause: " << data.cause << endl;
        // cout << RED << "======================================================" RESET << endl;
        if (data.location)
            plocation(*data.location, 15);
        if (data.location && data.location->errors.count((int)data.status))
        {
            // cout << LINE << " open error page: " << data.location->errors[data.status] << endl;
            con.data.st = get_state(data.location->errors[data.status]);
            bool isaccess = (con.data.st.st_mode & IS_ACCESS) && (con.data.st.st_mode & IS_FILE);
            bool opened_succefully = isaccess && con.open_error_page(data.location->errors[data.status]) > 0;
            if (isaccess && opened_succefully)
                return;
        }
        // throw Error(LINE, "");
        // throw Error("Error page not found");
        // cout << LINE << RED " response with generated error page: " RESET << endl;
        string response;
        if (status == HTTP_BAD_REQUEST)
            response = "BAD REQUEST";
        else if (status == HTTP_FORBIDEN)
            response = "FORBIDEN";
        else if (status == HTTP_NOT_FOUND)
            response = "NOT FOUND";
        else if (status == HTTP_METHOD_NOT_ALLOWED)
            response = "METHOD METHOD NOT ALLOWED";
        else if (status == HTTP_TIMEOUT)
            response = "TIMEOUT";
        else if (status == HTTP_INTERNAL_SERVER)
            response = "INTERNAL ERROR";
        else if (status == HTTP_METHOD_NOT_EMPLEMENTED)
            response = "NOT IMPLEMENTED";
        else
            throw Error("handle this error status " + to_string(status));

        response = "<div style=\"display:flex; justify-content:center;"
                   "align-items:center; color:blue; font-size: large;\"> Generated " +
                   response + (data.cause.length() ? " because " + data.cause : "") + "</div>";

        string header = generate_header(con.data, "text/html", response.length());
        data.clen = response.length();

        data.respbuff = header + response;
        data.flen = data.respbuff.length();

        data.action = WRITE_;
        // if(con.write_fd != con.fd)
        //     con.close_connection(LINE, con.write_fd);
        con.read_fd = -1;
        data.write_size = 0;
        con.write_fd = con.fd;
        data.finish_connection = true;
    }
    else if (status == HTTP_SUCCESS)
    {
        if (data.method == POST_)
        {
            data.action = WRITE_;
            string response = "<div style=\"display:flex; justify-content:center;"
                              "align-items:center; color:blue; font-size: large;\">"
                              "FILE created succefully"
                              "</div>";
            // to be checked, I change clen here
            data.clen = response.length();
            string header = generate_header(con.data, "text/html", data.clen);
            data.respbuff = header + response;

            data.write_size = 0;
            data.flen = data.respbuff.length();
            data.finish_connection = true;
        }
    }
    else if (status == HTTP_MOVE_PERMANENTLY)
    {
        data.action = WRITE_;
        data.clen = 0;
        string header = generate_header(con.data, "text/html", data.clen);
        data.respbuff = header;

        data.write_size = 0;
        data.flen = data.respbuff.length();
        con.write_fd = con.fd;
        data.finish_connection = true;
        // cout << RED << to_string((Status)status) << RESET << endl;
        // cout << "header is <" << data.respbuff << ">" << endl;
    }
}

Status set_status(int line, Location &location, string &path, state &state_, Method method, string uri)
{
    // cout << "called in line " << line << ", set status from " << path << endl;
    Status status = HTTP_NOT_FOUND;
    if (!location.methods[method])
        return HTTP_METHOD_NOT_ALLOWED;
    if (stat(path.c_str(), &state_) == 0)
    {
        if (!(state_.st_mode & IS_ACCESS))
            status = HTTP_FORBIDEN;
        else if (state_.st_mode & IS_FILE)
        {
            status = HTTP_SUCCESS;
            if (access(path.c_str(), method == POST_ ? W_OK : R_OK))
                status = HTTP_FORBIDEN;
        }
        else if (state_.st_mode & IS_DIR && uri[uri.length() - 1] == '/')
            status = HTTP_SUCCESS;
        else if (state_.st_mode & IS_DIR)
        {
            // throw Error("debuging");
            status = HTTP_MOVE_PERMANENTLY;
        }
    }
    return status;
}

// TODO: handle return here
string generate_header(Data &data, string ctype, size_t size)
{
    string header = to_string(data.http_v) + " " + to_string((int)data.status);
    header += (data.is_error()                 ? " NOK\r\n"
               : (data.status == HTTP_SUCCESS) ? " OK\r\n"
                                               : "\r\n");
    header += "Content-Type: " + ctype + "\r\n" +
              "Content-Length: " + to_string(size) + "\r\n";

    if (data.status == HTTP_MOVE_PERMANENTLY)
        header += "Location: http://" + data.listen + ":" + to_string(data.port) + "/" + data.uri + "/\r\n";
    // TODO: use connection close
    header += "\r\n";
    return header;
}

void handle_cgi(Connection &con, string path, Location &loc)
{
    // TODO: add access
    con.data.cgi_output = clear_path(loc.root + "/" + rand_name() + ".cgi");
    // cout << "cgi output      " << con.data.cgi_output << endl;
    // cout << "cgi has queries " << con.data.queries << endl;
    // throw Error("debuging");
    con.data.cgi_pid = fork();
    if (con.data.cgi_pid == 0)
    {
        // cout << GREEN "this is CGI" RESET << endl;
        int fd = open(con.data.cgi_output.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0777);
        if (fd < 0)
            exit(-1);
        std::vector<std::string> env;
        // if (con.data.method == POST_)
        // {
        //     // env.push_back("CONTENT_LENGTH=" + to_string(request.body_bytes_read));
        //     // if (!request.content_type.empty())
        //     //     env.push_back("CONTENT_TYPE=" + request.content_type);
        // }

        env.push_back("QUERY_STRING=" + con.data.queries);
        env.push_back("PATH_INFO=" + con.data.cgi_exec);
        env.push_back("REQUEST_METHOD=" + to_string(con.data.method));
        env.push_back("GATEWAY_INTERFACE=CGI/1.1");
        env.push_back("REMOTE_ADDR=" + string(con.address));
        // env.push_back("SCRIPT_NAME=" + );
        env.push_back("REMOTE_HOST=");

        // env.push_back("SERVER_NAME=" + client.interface + ":" + to_string(client.server_port));
        // TODO: fix it
        env.push_back("SERVER_PORT=" + to_string(con.data.port));

        string protocol = string("SERVER_PROTOCOL=") + (con.data.http_v == HTTP1_1 ? "HTTP/1.1" : "HTTP/1.0");
        env.push_back(protocol.c_str());
        env.push_back("SERVER_SOFTWARE=");
        env.push_back("SCRIPT_FILENAME=" + con.data.uri);
        env.push_back("REDIRECT_STATUS=CGI");

        // TODO: bonus
        // if (!request.cookie.empty())
        // env.push_back("HTTP_COOKIE=" + request.cookie);
        std::vector<char *> envp(env.size() + 1);
        for (size_t i = 0; i < env.size(); i++)
            envp[i] = (char *)env[i].c_str();
        envp[env.size()] = 0;

        // cout << "has exec path " << con.data.cgi_exec << endl;
        // cout << "execute       " << path << endl;
        // cout << "cgi filename  " << con.data.cgi_fname << endl;
        // cout << "cgi output    " << con.data.cgi_output << endl;
        // cout << "env: " << endl;
        for (size_t i = 0; i < envp.size(); i++)
            cout << envp[i] << endl;

        // child process
        if (dup2(fd, 1) < -1)
            exit(-1);
        close(fd);
        char *arg[3];
        // arg[0] = (char *)con.data.cgi_exec.c_str();
        // arg[1] = (char *)path.c_str();
        // arg[2] = NULL;
        arg[0] = (char *)con.data.cgi_exec.c_str();
        arg[1] = (char *)path.c_str();
        arg[2] = NULL;
        execve(con.data.cgi_exec.c_str(), arg, envp.data());
        cerr << "fork failed" << endl;
        // while(1);
        exit(-1);
    }
    else if (con.data.cgi_pid < 0) // TODO: check errors every where
                                   // add cgi timeout
    {
        throw Error("fork failed");
        con.data.status = HTTP_INTERNAL_SERVER;
        return generate_response(LINE, con);
    }

    // // cout << "did exit with " << status << endl;
    // throw Error("cgi finished");
    // exit(1);
}

// TODO: test it carefully
bool match_location(string &conf_location, string &uri)
{
    size_t i = 0;
    while (i < conf_location.length() && i < uri.length() && conf_location[i] == uri[i])
        i++;
    return (i == conf_location.length() || (i == conf_location.length() - 1 && i == uri.length()));
}

void fetch_config(Connection &con, vector<Server> &srvs)
{
    // cout << "call fetch config" << endl;
    Data &data = con.data;
    Method &method = data.method;
    string &uri = data.uri;

    string match;
    Location *matched_location = NULL;
    Server *match_server;

    vector<Location *> locations;
    bool second = false;
    size_t i = 0;
    while (i < srvs.size())
    {
        Server &srv = srvs[i];
        if ((srv.name == data.host || second) && srv.port == data.port)
        {
            // cout << "fetch: " << uri << " from " << srv.name << endl;
            map<string, Location>::iterator it;
            for (it = srv.location.begin(); it != srv.location.end(); it++)
            {
                string loc_key = it->first; // TODO: check >= , it may cause problems
                if (match_location(loc_key, uri) && loc_key.length() >= match.length())
                {
                    // cout << "location matched with " << loc_key << endl;
                    match = loc_key;
                    matched_location = &it->second;
                    match_server = &srv;
                }
            }
            break;
        }
        i++;
        if (second == false && i == srvs.size() && matched_location == NULL)
        {
            second = true;
            i = 0;
        }
    }
    if (matched_location)
    {
        Status status;
        bool is_dir;
        Location &loc = *matched_location;
        con.data.listen = match_server->listen;
        // cout << to_string(method) << ": location key <" << loc.key << ">" << endl;
        // cout << "substract " << uri << " from " << loc.key.length() << " to " << uri.length() << endl;
        string match_uri = uri.length() > loc.key.length() ? substr(LINE, uri, loc.key.length(), uri.length()) : "";
        string path;
        if (con.data.is_error())
        {
            data.location = &loc; // TODO: to be checked
            generate_response(LINE, con);
            // throw Error("fetch error pages ");
        }
        else if (method == GET_)
        {
            data.location = &loc;
            if (!loc.methods[method])
            {
                data.status = HTTP_METHOD_NOT_ALLOWED;
                generate_response(LINE, con);
                return;
            }
            cout << "> has <" << loc.src << "> look for " << match_uri << endl;
            path = clear_path(loc.src + "/" + match_uri);
            cout << "GET: " << path << endl;
            data.status = set_status(LINE, loc, path, data.st, method, con.data.uri);
            if (data.st.st_mode & IS_DIR && data.status == HTTP_SUCCESS)
            {
                cout << "found dir " << path << endl;
                string index_path;
                state tmp_state;
                Status status = HTTP_NOT_FOUND;
                bool index_file_found = false;
                if (loc.index.length())
                {
                    index_path = clear_path(path + "/" + loc.index);
                    // TODO: check if  index is cgi
                    status = set_status(LINE, loc, index_path, tmp_state, GET_, con.data.uri);
                    index_file_found = (status == HTTP_SUCCESS && tmp_state.st_mode & IS_FILE);
                    cout << GREEN << "status: " << to_string(status)
                         << " found index file: " << index_file_found << RESET << endl;
                }
                if (!index_file_found)
                {
                    // TODO: to be checked with NGINX
                    index_path = clear_path(path + "/index.html");
                    status = set_status(LINE, loc, index_path, tmp_state, GET_, con.data.uri);
                    index_file_found = (status == HTTP_SUCCESS && tmp_state.st_mode & IS_FILE);
                    cout << GREEN << to_string(status) << RESET << endl;
                    cout << GREEN << "status: " << to_string(status)
                         << " found index file: " << index_file_found << RESET << endl;
                }
                if (index_file_found)
                {
                    data.status = HTTP_SUCCESS;
                    path = index_path;
                    con.data.st = tmp_state;
                }
                cout << "line " << LINE << " index file found " << !index_file_found
                     << " autoindex " << (loc.autoindex == on) << endl;
                if (!index_file_found && loc.autoindex == on)
                {
                    con.open_dir(path, uri);
                    return;
                }
                else if (!index_file_found)
                    con.data.status = HTTP_FORBIDEN;
            }
            if (data.st.st_mode & IS_DIR && data.status == HTTP_MOVE_PERMANENTLY)
            {
                // cout <<RED "move permanenetly "RESET << path << endl;
                generate_response(LINE, con);
                return;
            }
            if (data.st.st_mode & IS_FILE && data.status == HTTP_SUCCESS)
            {
                cout << "found file " << path << endl;
                data.clen = data.st.st_size;
                // TODO: file might be cgi and st_size == 0
                if (con.file_is_cgi(path, loc))
                {
                    handle_cgi(con, path, loc);
                    return;
                }
                else if (data.st.st_size == 0 && con.open_file(path) > 0)
                {
                    con.data.action = WRITE_;
                    return;
                }
                else if (con.open_file(path) > 0)
                    return;
            }
        }
        else if (method == POST_)
        {
            con.data.location = &loc;
            if (!loc.methods[method])
            {
                data.status = HTTP_METHOD_NOT_ALLOWED;
                generate_response(LINE, con);
                return;
            }
            string ext;
            if (memetype.count(data.ctype))
                ext = memetype[data.ctype];
            path = clear_path(loc.dest + "/" + match_uri + "/" + rand_name() + ext);
            // cout << "POST: " << path << endl;

            if (con.file_is_cgi(path, loc))
            {
                handle_cgi(con, path, loc);
                return;
            }
            else if (con.open_file(path) > 0)
            {
                con.data.location = &loc;
                if (!con.data.is_error() && data.clen == 0)
                {
                    // keep this message will be used in chunked
                    // cout << RED "empty file" RESET << endl;
                    con.close_connection(LINE, con.write_fd);
                    generate_response(LINE, con);
                    con.write_fd = con.fd;
                }
                return;
            }
        }
        else if (method == DELETE_)
        {
            con.data.location = &loc;
            if (!loc.methods[method])
            {
                data.status = HTTP_METHOD_NOT_ALLOWED;
                generate_response(LINE, con);
                return;
            }
        }
    }
    // cout << RED "NOT FOUND " RESET << con_state(con) << endl;
    // keep it like this, used when asking for a file that does
    // not exists
    if (con.data.is_error())
    {
        // con.data.status = HTTP_NOT_FOUND;
        generate_response(LINE, con);
    }
}

void Webserve(vector<Server> &conf_servs)
{
    // SIGNALS
    struct sigaction sa = {};
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);

    // INIT SERVERS
    size_t i = 0;
    size_t servs_size = 0;
    while (i < conf_servs.size())
    {
        Connection con = new_connection(-1, SERVER_, conf_servs[i].port, conf_servs[i].listen);
        if (con.fd > 0)
        {
            cons_vec.push_back(con);
            cons[con.fd] = cons_vec[cons_vec.size() - 1];
            servs_size++;
        }
        i++;
    }
    if (!servs_size)
        throw Error("server failed to start");
    cout << "servs size " << cons_vec.size() << endl;

    // WEBSERVING
    ssize_t timing = -1;
    cout << "Start webserver" << endl;
    while (1)
    {
        timing = (timing + 1) % 1000000;
        if (!timing)
            cout << "pfds size " << pfds.size() << endl;
        int ready = poll(pfds.data(), pfds.size(), -1);
        if (ready < 0)
        {
            cerr << RED "poll failed" RESET << endl;
            continue;
        }
        else if (ready > 0)
        {
            ssize_t i = -1, r;
            while (++i < pfds.size())
            {
                if (!pairs.count(pfds[i].fd) || pairs.count(pfds[i].fd) > 1)
                    throw Error("pair not found"); // to be removed
                Connection &con = cons[pairs[pfds[i].fd]];
                cout << con_state(con) <<endl;
                if (pfds[i].revents & POLLERR)
                {
                    cout << RED "POLLERR in " << pfds[i].fd << RESET << endl;
                    con.close_connection(LINE, con.fd);
                    continue;
                }
                if (i < servs_size && pfds[i].revents & POLLIN && con.type == SERVER_)
                {
                    Connection client = new_connection(pfds[i].fd, CLIENT_, con.data.port, con.data.srv_address);
                    cons_vec.push_back(client);
                    cons[client.fd] = cons_vec[cons_vec.size() - 1];
                }
                else if (i >= servs_size)
                {
                    if (con.data.is_cgi && !con.data.cgi_finished)
                    {
                        int status = 1;
                        if (waitpid(con.data.cgi_pid, &status, WNOHANG) > 0)
                        {
                            // cout << LINE << " did exit with " << WIFEXITED(status) << endl;
                            // BAD GATEWAY
                            // throw Error("debuging");
                            // fetch_config(sock);
                            // throw Error("CGI failed");
                            if (!WIFEXITED(status))
                            {
                                throw Error("CGI failed");
                            }
                            else
                            {
                                // TODO: check file size if 0
                                // bad gateway
                                con.open_cgi_file(con.data.cgi_output);
                                con.data.cgi_finished = true;
                            }
                        }
                    }
                    else if (pfds[i].revents & POLLIN && con.data.action == READ_ && pfds[i].fd == con.read_fd)
                    {
                        // cout << "POLLIN: " << con_state(con) << endl;
                        r = con.readbuff(pfds[i].fd);
                        if ((r > 0 && !con.data.header_parsed && parse_header(con)))
                        {
                            cout << "before fetching config " << con_state(con) << endl;
                            fetch_config(con, conf_servs);
                            if (con.data.is_error() && con.data.action == READ_)
                                con.data.action = WRITE_;
                            cout << "after fetching config " << con_state(con) << endl;
                        }
                    }
                    else if (pfds[i].revents & POLLOUT && con.data.action == WRITE_ && pfds[i].fd == con.write_fd)
                    {
                        // cout << "POLLOUT: " << con_state(con) << endl;
                        r = con.writebuff(pfds[i].fd);
                    }
                }
                cout << RED "=================================================================="RESET <<endl;
            }
        }
    }
}

int main(int argc, char **argv)
{
    vector<Server> conf_servs;
    try
    {
        if (argc != 2)
            throw Error(string("Invalid number of arguments"));
        parse_config(conf_servs, argv[1]);
        for (size_t i = 0; exts_struct[i].type.length(); ++i)
        {
            memetype[exts_struct[i].type] = exts_struct[i].ext;
            memetype[exts_struct[i].ext] = exts_struct[i].type;
        }
        size_t i = 0;
        while (i < conf_servs.size())
        {
            pserver(conf_servs[i]);
            i++;
        }
        Webserve(conf_servs);
    }
    catch (exception &err)
    {
        cerr << RED "Error: " << err.what() << RESET << endl;
    }
}