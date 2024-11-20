/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhrima <mhrima@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/07 16:53:39 by mhrima            #+#    #+#             */
/*   Updated: 2024/06/07 16:53:39 by mhrima           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.hpp"

size_t pos;
size_t line;
vector<string> arr;

void tokenize(string &text)
{
    stringstream ss(text);
    string token;
    while (getline(ss, token, '\n'))
    {
        size_t i = token.find('#');
        if (i != string::npos)
            token = substr(LINE, token, 0, i);
        i = 0;
        while ((i = token.find_first_of(" ;{}:", i)) != string::npos)
        {
            if (i)
                arr.push_back(token.substr(0, i));
            if (token[i] != ' ')
                arr.push_back(string(1, token[i]));
            token.erase(0, i + 1);
            i = 0;
        }
        if (!token.empty())
            arr.push_back(token);
        arr.push_back("\n");
    }
}

void expect(string val)
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

string &get_value()
{
    if (strchr(";{}:\n", arr[pos][0]) || arr[pos][0] == '\0')
        throw Error(line, "Unexpected <" + (arr[pos] == "\n" ? "\\n" : arr[pos]) + ">");
    return arr[pos++];
}

string &get_value(string val)
{
    expect(val);
    return get_value();
}

void set_method(map<Method, bool> &methods, string &value)
{
    if (value == "GET")
        methods[GET_] = on;
    else if (value == "POST")
        methods[POST_] = on;
    else if (value == "DELETE")
        methods[DELETE_] = on;
    else
        throw Error(line, "Invalid method: " + value);
}

string Host_to_ip(string host)
{
    struct hostent *hostInfo = gethostbyname(host.c_str());
    if (hostInfo == NULL)
        throw Error(line, "Invalid address");
    struct in_addr **addressList = (struct in_addr **)(hostInfo->h_addr_list);
    if (addressList[0] != NULL)
        return inet_ntoa(*addressList[0]);
    else
        throw Error(line, "Invalid address: " + host);
    return "";
}

bool parse_location(Location &curr)
{
    string props[] = {"root", "autoindex", "source", "destination", "index",
                      "limit", "cgi", "methods", "errors", "return", ""};
    bool found = false;
    for (size_t i = 0; props[i].length(); i++)
    {
        if (arr[pos] == props[i])
        {
            found = true;
            pos++;
            if (props[i] == "root") // everywhere
            {
                curr.root = get_value(":");
                expect(";");
            }
            else if (props[i] == "limit") // everywhere
            {
                string value = get_value(":");
                if (!_isdigits(value))
                    throw Error(line, "Invalid limit");
                char *ptr;
                ssize_t limit = strtol(value.c_str(), &ptr, 10);
                if (*ptr != '\0' || limit <= 0)
                    throw Error(line, "Invalid limit");
                curr.limit = (size_t)limit;
                expect(";");
            }
            else if (props[i] == "autoindex") // everywhere
            {
                string value = get_value(":");
                if (value == "on")
                    curr.autoindex = true;
                else if (value == "off")
                    curr.autoindex = false;
                else
                    throw Error(line, "Invalid autoindex value <" + value + ">");
                expect(";");
            }
            else if (props[i] == "destination") // location
            {
                if (curr.location.empty())
                    throw Error(line, "destination should be only inside location");
                curr.dest = clear_path(get_value(":"));
                expect(";");
            }
            else if (props[i] == "source") // location
            {
                if (curr.location.empty())
                    throw Error(line, "source should be only inside location");
                curr.src = clear_path(get_value(":"));
                expect(";");
            }
            else if (props[i] == "index") // location
            {
                if (curr.location.empty())
                    throw Error(line, "cgi should be only inside location");
                curr.index = clear_path(get_value(":"));
                expect(";");
            }
            else if (props[i] == "cgi") // location
            {
                if (curr.location.empty())
                    throw Error(line, "cgi should be only inside location");
                string key = get_value(":");
                if (key[0] != '.')
                    throw Error(line, "Invalid cgi extention");
                string value = get_value();
                curr.cgi[key] = value;
                expect(";");
            }
            else if (props[i] == "methods") // location
            {
                if (curr.location.empty())
                    throw Error(line, "methods should be only inside location");
                expect(":");
                get_value();
                pos--;
                while (arr[pos] != ";")
                {
                    string value = get_value();
                    set_method(curr.methods, value);
                }
                expect(";");
            }
            else if (props[i] == "return") // location
            {
                if (curr.location.empty())
                    throw Error(line, "return should be only inside location");
                string value = get_value(":");
                if (!_isdigits(value))
                    throw Error(line, "Invalid status code");
                char *ptr;
                ssize_t status = strtol(value.c_str(), &ptr, 10);
                if (*ptr != '\0' || (status != 301 && status != 307 && status != 308))
                    throw Error(line, "Invalid return status code");
                curr.ret_location = clear_path(get_value());
                if (curr.ret_location[0] != '/')
                    curr.ret_location = "/" + curr.ret_location;
                curr.ret_status = (Status)status;
                expect(";");
            }
            else if (props[i] == "errors") // server scoop
            {
                if (!curr.location.empty())
                    throw Error(line, "errors should be only inside server scoop");
                expect("{");
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
                        curr.errors[status] = get_value(":");
                        expect(";");
                        while (arr[pos] == "\n")
                        {
                            line++;
                            pos++;
                        }
                    }
                    else
                        throw Error(line, "Expected number as status got <" + arr[pos] + ">");
                }
                expect("}");
            }
            break;
        }
    }
    return found;
}

Server parse()
{
    Server srv;
    expect("{");
    string loc = "";
    srv.locations[""] = (Location){};
    while (pos < arr.size())
    {
        bool found = false;
        string props[] = {/*server attributes*/ "listen", "name", "location", ""};
        for (size_t i = 0; props[i].length(); i++)
        {
            if (arr[pos] == props[i])
            {
                if (loc.length())
                    throw Error(line, "Invalid " + props[i] + " inside child location");
                pos++; // skip prop
                found = true;
                if (props[i] == "listen")
                {
                    srv.listen = get_value(":");
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
                    srv.listen = Host_to_ip(srv.listen);
                    if (srv.listen == machine_ip_address)
                        srv.listen = "127.0.0.1";
                    expect(";");
                }
                else if (props[i] == "name")
                {
                    srv.name = get_value(":");
                    if (!_isaldigits(srv.name))
                        throw Error(line, "Invalid host name");
                    expect(";");
                }
                else if (props[i] == "location")
                {
                    string key = get_value();
                    key = clear_path("/" + key + "/");
                    if (srv.locations.count(key))
                        throw Error(line, "repeated location on same server");
                    srv.locations[key] = (Location){};
                    srv.locations[key].location = key;
                    expect("{");
                    while (arr[pos] != "}")
                    {
                        if (arr[pos] == "\n")
                            pos++, line++;
                        else if (!parse_location(srv.locations[key]))
                            throw Error(line, "Unexpected <" + arr[pos] + ">");
                    }
                    expect("}");
                }
                break;
            }
        }
        if (found)
            continue;
        if (parse_location(srv.locations[loc]))
            continue;
        if (arr[pos] == "}")
            break;
        else if (arr[pos] == "\n")
            pos++, line++;
        else
            throw Error(line, "Unexpected <" + arr[pos] + ">");
    }
    expect("}");
    if (srv.locations.size() < 2)
        throw Error(line, "Server require at least one location");
    return srv;
}

void check_server(Server &serv)
{
    if (serv.name.empty())
        throw Error("Server name is required");
    if (serv.locations[""].autoindex == none)
        serv.locations[""].autoindex = off;
    if (!starts_with(serv.locations[""].root, "./"))
        serv.locations[""].root = clear_path("./" + serv.locations[""].root);
    map<string, Location>::iterator it;
    for (it = serv.locations.begin(); it != serv.locations.end(); it++)
    {
        Location &loc = it->second;
        loc.errors = serv.locations[""].errors;
        if (loc.root.empty())
            loc.root = serv.locations[""].root;
        if (!starts_with(loc.root, "./"))
            loc.root = clear_path("./" + loc.root);
        if (serv.locations[""].limit && !loc.limit)
            loc.limit = serv.locations[""].limit;
        if (loc.autoindex == none)
            loc.autoindex = serv.locations[""].autoindex;
        loc.dest = clear_path(loc.root + "/" + loc.dest);
        loc.src = clear_path(loc.root + "/" + loc.src);
    }
}

void check_vector(vector<Server> &servs)
{
    for (size_t i = 0; i < servs.size(); i++)
    {
        check_server(servs[i]);
        for (size_t j = i + 1; j < servs.size(); j++)
            if (servs[i].listen == servs[j].listen && servs[i].port == servs[j].port && servs[i].name == servs[j].name)
                throw Error("servers have same host, port, and name");
    }
}

void parse_config(char *filename)
{
    line = 1;
    vServer vservs;

    if (!ends_with(string(filename), ".conf"))
        throw Error("Invalid Config file extention");

    ifstream file(filename);
    if (!file.is_open())
        throw Error(string("Failed to open file"));

    string text((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));
    file.close();
    tokenize(text);
#if 0
    for (size_t i = 0; i < arr.size(); i++)
        cout << "arr[" << i << "] <" << arr[i] << ">" << endl;
#endif

    pos = 0;
    while (pos < arr.size())
    {
        if (arr[pos] == "\n")
        {
            line++;
            pos++;
        }
        else
            vservs.push_back(parse());
    }
    file.close();
    check_vector(vservs);

    size_t i = 0;
    while (i < vservs.size())
    {
        servers[vservs[i].port].push_back(vservs[i]);
        cout << RED "       server: " RESET << i << endl;
        cout << vservs[i] << endl;
        i++;
    }
}