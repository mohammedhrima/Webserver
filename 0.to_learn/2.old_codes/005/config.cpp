#include "header.hpp"

size_t pos;
size_t line;

vector<string> tokenize(string &text)
{
    vector<string> result;
    stringstream ss(text);
    string token;
    while (getline(ss, token, '\n'))
    {
        size_t pos = token.find('#');
        if (pos != string::npos)
            token = substr(LINE, token, 0, pos);
        pos = 0;
        while ((pos = token.find_first_of(" ;{}:", pos)) != string::npos)
        {
            if (pos)
                result.push_back(token.substr(0, pos));
            if (token[pos] != ' ')
                result.push_back(string(1, token[pos]));
            token.erase(0, pos + 1);
            pos = 0;
        }
        if (!token.empty())
            result.push_back(token);
        result.push_back("\n"); // used in parsing errors
    }
    return result;
}

void expect(vector<string> &arr, string val)
{
    if (pos < arr.size() && arr[pos] == val)
    {
        pos++;
        return;
    }
    if (pos < arr.size())
        throw Error(line, "Expected '" + val + "' got <" + (arr[pos] == "\n" ? "\\n" : arr[pos]) + ">");
    else
        throw Error(line, "Expected '" + val + "'");
}

string &get_value(vector<string> &arr)
{
    if (strchr(";{}:\n", arr[pos][0]))
        throw Error(line, "Unexpected <" + (arr[pos] == "\n" ? "\\n" : arr[pos]) + ">");
    return arr[pos++];
}

string &get_value(vector<string> &arr, string val)
{
    expect(arr, val);
    return get_value(arr);
}

void set_method(map<Method, int> &methods, string &value)
{
    if (value == "GET")
        methods[GET_] = on;
    else if (value == "POST")
        methods[POST_] = on;
    else if (value == "DELETE")
        methods[DELETE_] = on;
    else
        throw Error(line, "Invalid method");
}

string Host_to_IP(string name)
{
    struct hostent *hostInfo = gethostbyname(name.c_str());
    if (hostInfo == NULL)
        throw Error(line, "Invalid address");
    struct in_addr **addressList = (struct in_addr **)(hostInfo->h_addr_list);
    if (addressList[0] != NULL)
        return inet_ntoa(*addressList[0]);
    else
        throw Error(line, "Invalid address");
    return "";
}

bool parse_location(vector<string> &arr, Location &curr)
{
    string props[] = {/*server and locations attributes*/ "root", "autoindex", "source",
                      "destination", "index", "limit", "cgi", "methods", "errors", "return", ""};
    size_t i = 0;
    bool found = false;
    for (; props[i].length(); i++)
    {
        if (arr[pos] == props[i])
        {
            // cout << "found " << props[i] << endl;
            found = true;
            pos++;
            if (props[i] == "root")
            {
                curr.root = get_value(arr, ":");
                expect(arr, ";");
            }
            else if (props[i] == "limit")
            {
                string value = get_value(arr, ":");
                if (!_isdigits(value))
                    throw Error(line, "Invalid limit");
                char *ptr;
                ssize_t limit = strtol(value.c_str(), &ptr, 10);
                if (!ptr || limit <= 0)
                    throw Error(line, "Invalid limit");
                curr.limit = limit;
                curr.has_limit = true;
                expect(arr, ";");
            }
            else if (props[i] == "autoindex")
            {
                string value = get_value(arr, ":");
                if (value == "on")
                    curr.autoindex = on;
                else if (value == "off")
                    curr.autoindex = off;
                else
                    throw Error(line, "Invalid autoindex value <" + value + ">");
                expect(arr, ";");
            }
            else if (props[i] == "destination")
            {
                if (curr.location.empty())
                    throw Error(line, "destination should be only inside a location");
                curr.dest = clear_path(get_value(arr, ":"));
                expect(arr, ";");
            }
            else if (props[i] == "source")
            {
                if (curr.location.empty())
                    throw Error(line, "source should be only inside a location");
                curr.src = clear_path(get_value(arr, ":"));
                expect(arr, ";");
            }
            else if (props[i] == "index")
            {
                curr.index = clear_path(get_value(arr, ":"));
                expect(arr, ";");
            }
            else if (props[i] == "cgi")
            {
                string key = get_value(arr, ":");
                if (key[0] != '.')
                    throw Error(line, "Invalid cgi extention");
                string value = get_value(arr);
                curr.cgi[key] = value;
                expect(arr, ";");
            }
            else if (props[i] == "methods")
            {
                expect(arr, ":");
                get_value(arr);
                pos--;
                while (arr[pos] != ";")
                {
                    string value = get_value(arr);
                    set_method(curr.methods, value);
                }
                expect(arr, ";");
            }
            else if (props[i] == "return")
            {
                string value = get_value(arr, ":");
                if (!_isdigits(value))
                    throw Error(line, "Invalid status code");
                char *ptr;
                ssize_t status = strtol(value.c_str(), &ptr, 10);
                if (!ptr || (status != 301 && status != 307 && status != 308))
                    throw Error(line, "Invalid return status code");
                curr.ret_location = clear_path(get_value(arr));
                if (curr.ret_location[0] != '/')
                    curr.ret_location = "/" + curr.ret_location;
                curr.is_ret = true;
                curr.ret_status = (Status)status;
                expect(arr, ";");
            }
            else if (props[i] == "errors")
            {
                expect(arr, "{");
                while (arr[pos] == "\n")
                {
                    line++;
                    pos++;
                }
                while (arr[pos] != "}")
                {
                    int status = 0;
                    if (_isdigits(arr[pos]))
                    {
                        status = atoi(arr[pos++].c_str());
                        if (status < 400 || status > 599)
                            throw Error(line, "Invalid status");
                        curr.errors[status] = get_value(arr, ":");
                        expect(arr, ";");
                        while (arr[pos] == "\n")
                        {
                            line++;
                            pos++;
                        }
                    }
                    else
                        throw Error(line, "Expected number as status got <" + arr[pos] + ">");
                }
                expect(arr, "}");
            }
            break;
        }
    }
    return found;
}

Server parse(vector<string> &arr)
{
    Server srv;
    expect(arr, "{");
    string loc = "";
    srv.locations[""] = (Location){};
    while (pos < arr.size())
    {
        bool found = false;
        string props[] = {/*server attributes*/ "listen", "name", "location", ""};
        size_t i = 0;
        for (; i < 3; i++)
        {
            if (arr[pos] == props[i])
            {
                if (loc.length())
                    throw Error(line, "Invalid " + props[i] + " inside child location");
                pos++; // skip prop
                found = true;
                if (props[i] == "listen")
                {
                    srv.listen = get_value(arr, ":");
                    if (arr[pos] == ":")
                    {
                        pos++;
                        if (!_isdigits(arr[pos]))
                            throw Error(line, "Invalid port, port should contains only digits");
                        srv.port = atol(arr[pos++].c_str());
                        // 49152 to 65535
                        if (srv.port < 0 || srv.port > 65535)
                            throw Error(line, "Invalid port, port should contains only digits");
                    }
                    else
                        srv.port = 80;
                    srv.listen = Host_to_IP(srv.listen);
                    if (srv.listen == machine_ip_address)
                        srv.listen = "127.0.0.1";
                    expect(arr, ";");
                }
                else if (props[i] == "name")
                {
                    srv.name = get_value(arr, ":");
                    expect(arr, ";");
                }
                else if (props[i] == "location")
                {
                    string key = get_value(arr);
                    if (key[0] != '/')
                        key = "/" + key;
                    key = clear_path(key);
                    if (key[key.length() - 1] != '/')
                        key += "/";
                    if (srv.locations.count(key))
                        throw Error(line, "repeated location on same server");
                    srv.locations[key] = (Location){};
                    srv.locations[key].location = key;
                    expect(arr, "{");
                    while (arr[pos] != "}")
                    {
                        if (arr[pos] == "\n")
                            pos++, line++;
                        else if (!parse_location(arr, srv.locations[key]))
                            throw Error(line, "Unexpected <" + arr[pos] + ">");
                    }
                    expect(arr, "}");
                }
                break;
            }
        }
        if (found)
            continue;
        if (parse_location(arr, srv.locations[loc]))
            continue;
        if (arr[pos] == "}")
            break;
        else if (arr[pos] == "\n")
            pos++, line++;
        else
            throw Error(line, "Unexpected <" + arr[pos] + ">");
    }
    expect(arr, "}");
    return srv;
}

void check(Server &serv)
{
    cout << "call check " << endl;
    if (serv.name.empty())
        throw Error("Server name is required");
    if (serv.locations[""].autoindex == none)
        serv.locations[""].autoindex = off;
    if (serv.locations[""].index.empty())
        serv.locations[""].index = "index.html";
    // if (!starts_with(serv.locations[""].root, "./"))
    //     serv.locations[""].root = clear_path("./" + serv.locations[""].root);
    Method methods[] = {GET_, POST_, DELETE_};
    ssize_t i = 0;
    while (i < 3)
    {
        if (serv.locations[""].methods[methods[i]] == none)
            serv.locations[""].methods[methods[i]] = off;
        i++;
    }
    // TODO: iterate over error pages and there path to root
    // error pages should be only in empty location (server location)

    map<string, Location>::iterator it;
    // map<int, string>::iterator it0;

    for (it = serv.locations.begin(); it != serv.locations.end(); it++)
    {
        Location &loc = it->second;
        if (loc.errors.empty())
            loc.errors = serv.locations[""].errors;
        if (loc.root.empty())
            loc.root = serv.locations[""].root;
        // if (!starts_with(loc.root, "./"))
        //     loc.root = clear_path("./" + loc.root);
        // if (loc.dest.empty())
        //     loc.dest = serv.locations[""].dest;
        // if (loc.src.empty())
        //     loc.src = serv.locations[""].src;
        if (serv.locations[""].has_limit)
        {
            loc.has_limit = serv.locations[""].has_limit;
            loc.limit = serv.locations[""].limit;
        }
        if (loc.autoindex == none)
            loc.autoindex = serv.locations[""].autoindex;
        if (loc.index.empty())
            loc.index = serv.locations[""].index;
        i = 0;
        while (i < 3)
        {
            if (loc.methods[methods[i]] == none)
                loc.methods[methods[i]] = serv.locations[""].methods[methods[i]];
            i++;
        }
        if (!loc.root.empty())
        {
            loc.dest = clear_path(loc.root + "/" + loc.dest);
            loc.src = clear_path(loc.root + "/" + loc.src);
        }
        // it0 = loc.errors.begin();
        // while (it0 != loc.errors.end())
        // {
        //     it0->second = clear_path(loc.root + "/" + it0->second);
        //     it0++;
        // }
    }
}
// TODO: Host should not be -> value:port
void check_vector(vector<Server> &servs)
{
    cout << "call check vector" << endl;
    for (size_t i = 0; i < servs.size(); i++)
    {
        check(servs[i]);
        for (size_t j = i + 1; j < servs.size(); j++)
        {
            if (servs[i].listen == servs[j].listen && servs[i].port == servs[j].port && servs[i].name == servs[j].name)
                throw Error("servers have same host, port, and name");
        }
    }
}

mServers parse_config(char *filename)
{
    line = 1;
    vServer vservs;
    mServers mservs;

    if (!ends_with(string(filename), ".conf"))
        throw Error("Invalid Config file extention");
    ifstream file(filename);
    if (!file.is_open())
        throw Error(string("Failed to open file"));
    string text((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));
    vector<string> arr = tokenize(text);

    // for (size_t i = 0; i < arr.size(); i++)
    //     cout << "<" << arr[i] << ">";

    while (pos < arr.size())
    {
        if (arr[pos] == "\n")
        {
            line++;
            pos++;
        }
        else
            vservs.push_back(parse(arr));
    }
    file.close();
    check_vector(vservs);

    size_t i = 0;
    while (i < vservs.size())
    {
        mservs[vservs[i].port].push_back(vservs[i]);
        cout << vservs[i] << endl;
        i++;
    }
    cout << "END OF CONFIG" << endl;
    cout << GREEN SPLIT RESET << endl;
    return mservs;
}