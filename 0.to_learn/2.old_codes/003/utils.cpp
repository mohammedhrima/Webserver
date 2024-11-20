#include "header.hpp"

string to_string(size_t size)
{
    stringstream stream;
    stream << size;
    return stream.str();
}

string to_string(HTTP_V version)
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
// TODO: optimize it
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

string to_lower(string &str)
{
    string res;
    for (size_t i = 0; i < str.length(); i++)
        res += tolower(str[i]);
    return res;
}

state get_state(string &path)
{
    state state_;
    stat(path.c_str(), &state_);
    return state_;
}

string update_request_host(string listen)
{
    struct hostent *hostInfo = gethostbyname(listen.c_str());
    if (hostInfo == NULL)
    {
        cerr << "error 1 in " << __func__ << endl;
        return listen;
    }
    struct in_addr **addressList = (struct in_addr **)(hostInfo->h_addr_list);
    if (addressList[0] != NULL)
    {
        cerr << "error 2 in " << __func__ << endl;
        return listen;
    }
    return string(inet_ntoa(*addressList[0]));
}