#include "header.hpp"

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
                curr.limit_body = limit;
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
                curr.dest = clear_path(get_value(arr, ":"));
                expect(arr, ";");
            }
            else if (props[i] == "source")
            {
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
                    curr.set_method(line, value);
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
                string return_location = get_value(arr);
                curr.is_return = true;
                curr.return_status = status;
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
                        srv.update_host(line);
                    }
                    else
                        srv.port = 80;
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
                    srv.location[key].key = key;
                    expect(arr, "{");
                    while (arr[pos] != "}")
                    {
                        if (arr[pos] == "\n")
                            pos++, line++;
                        else if (!parse_location(arr, srv.location[key]))
                            throw Error(line, "Unexpected <" + arr[pos] + ">");
                    }
                    expect(arr, "}");
                }
                break;
            }
        }
        if (found)
            continue;
        if (parse_location(arr, srv.location[loc]))
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
    if (serv.location[""].autoindex == none)
        serv.location[""].autoindex = off;

    map<string, Location>::iterator it;
    map<int, string>::iterator it0;
    for (it = serv.location.begin(); it != serv.location.end(); it++)
    {
        if (it->second.errors.empty())
            it->second.errors = serv.location[""].errors;
        if (it->second.root.empty())
            it->second.root = serv.location[""].root;
        if (it->second.dest.empty())
            it->second.dest = serv.location[""].dest;
        if (it->second.src.empty())
            it->second.src = serv.location[""].src;
        // if (it->second.index.empty())
        //     it->second.index = serv.location[""].index;
        if (it->second.limit_body == -1)
            it->second.limit_body = serv.location[""].limit_body;
        if (it->second.autoindex == none)
            it->second.autoindex = serv.location[""].autoindex;
        if (!it->second.root.empty())
        {
            it->second.dest = clear_path(it->second.root + "/" + it->second.dest);
            it->second.src = clear_path(it->second.root + "/" + it->second.src);
        }
    }
}

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

void parse_config(vector<Server> &servs, char *filename)
{
    if (!ends_with(string(filename), ".conf"))
        throw Error("Invalid Config file extention");
    ifstream file(filename);
    if (!file.is_open())
        throw Error(string("Failed to open file"));
    string text((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));
    vector<string> arr = tokenize(text);
#if 1
    for (size_t i = 0; i < arr.size(); i++)
        cout << "<" << arr[i] << ">";
#endif
    while (pos < arr.size())
    {
        if (arr[pos] == "\n")
        {
            line++;
            pos++;
        }
        else
            servs.push_back(parse(arr));
    }
    check_vector(servs);
    file.close();
}