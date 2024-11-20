#include "header.hpp"

vector<string> split_by_space(const string &input)
{
    vector<string> tokens;
    size_t spos = 0;
    size_t epos = input.find_first_of("; ");
    if (epos == string::npos)
        tokens.push_back(input);
    while (epos != string::npos)
    {
        string token = input.substr(spos, epos - spos);
        tokens.push_back(token);
        if (input[epos] == ':')
            tokens.push_back(":");
        spos = input.find_first_not_of(" \n\r:", epos);
        epos = input.find_first_of(" \n\r:", spos);
    }

    // cout << "\n=====================================\n";
    // for (size_t i = 0; i < tokens.size(); i++)
    //     cout << "<" << tokens[i] << "> ";
    // cout << "\n=====================================\n";

    return tokens;
}

string trim(const string str)
{
    size_t s = 0;
    size_t e = str.length();

    while (s < e && isspace(str[s]))
        s++;
    while (e > s && isspace(str[e - 1]))
        e--;
    return str.substr(s, e - s);
}

bool is_bad_url(string &url)
{
    ssize_t dots_pos = url.find("..");
    bool cond = false;
    // djdjdjfd../
    cond = cond || dots_pos != std::string::npos && (dots_pos < url.size() - 2 && url[dots_pos + 2] == '/');
    cond = cond || url.find("..") == 0;                // at beginning
    cond = cond || url.rfind("/..") == url.size() - 3; // at end
    cond = cond || url.find("/../") != std::string::npos;

    return cond;
}

bool parse_uri(Data &data, string request_uri)
{
#if 0
    string &uri = data.uri;
#else
    string &uri = request_uri;
#endif

    string &queries = data.queries;
    string host;
    size_t s = 0;
    size_t e = 0;

    // cout << RED << "line " << LINE << " <" << uri << ">" << RESET << endl;
    if (is_bad_url(request_uri))
        return true;
#if 0
    if (starts_with(request_uri, "http://"))
    {
        // data.host_found_in_uri = true;
        data.host_found = true;
        s = request_uri.find("/", strlen("http://"));
        if (s != string::npos)
        {
            host = substr(LINE, request_uri, strlen("http://"), s);
            uri = substr(LINE, request_uri, s, request_uri.length());
        }
        else
        {
            host = substr(LINE, request_uri, strlen("http://"), s);
            uri = substr(LINE, request_uri, s + 1, request_uri.length());
        }
    }
    host = request_uri;
    if (host.empty())
        return data.check(LINE, true, HTTP_BAD_REQUEST, "empty host");
    s = host.find(":");
    if (s != string::npos)
        host = substr(LINE, host, 0, s);
        // cout << "found Host in uri <" << data.host << ">" << endl;
#endif
    if (uri.length())
    {
        uri = parse_hexadecimal(uri);
        ssize_t q_pos = find(uri, (char *)"?");
        if (q_pos != string::npos)
        {
            // found queries
            s = q_pos;
            while (!isspace(uri[s]) && s < uri.length())
                s++;
            queries = substr(LINE, uri, q_pos + 1, s);
            uri = substr(LINE, uri, 0, q_pos);
        }
    }
    data.uri = clear_path(uri);
    cout << FILE << " line " << LINE << " uri: " << data.uri << endl;

    return false;
}

bool invalid_http_version(string line, HTTP_V &http, Status &status)
{
    size_t i = 0;
    // cout << LINE << " " << line << endl;
    if (starts_with(line, "HTTP/"))
    {
        line = substr(LINE, line, 5, line.length());
        // cout << LINE << " <" << line << "> len: " << line.length() << endl;
        if (line.length() != 3 || !isdigit(line[0]) || line[1] != '.')
        {
            // cout << LINE << " " << line << endl;
            status = HTTP_BAD_REQUEST;
            return true;
        }
        if (isdigit(line[0]) && line[1] == '.')
        {
            // cout << LINE << " " << line << endl;
            if (line[0] == '1')
            {
                // cout << LINE << " " << line << endl;
                if (line[2] == '0')
                {
                    // cout << LINE << " " << line << endl;
                    http = HTTP1_0;
                    return false;
                }
                else if (line[2] == '1')
                {
                    // cout << LINE << " " << line << endl;
                    http = HTTP1_1;
                    return false;
                }
                else if (isdigit(line[2]))
                {
                    // cout << LINE << " " << line << endl;
                    status = HTTP_INSUPPORTED_HTTP;
                    return true;
                }
                else
                {
                    // cout << LINE << " " << line << endl;
                    status = HTTP_BAD_REQUEST;
                    return true;
                }
            }
            else
            {
                // cout << LINE << " " << line << endl;
                status = HTTP_INSUPPORTED_HTTP;
                return true;
            }
        }
        else
        {
            // cout << LINE << " " << line << endl;
            status = HTTP_BAD_REQUEST;
            return true;
        }
    }
    // cout << LINE << " " << line << endl;
    status = HTTP_BAD_REQUEST;
    return true;
}

bool parse_request_header(string &request_line, Connection &con, string method_str)
{
    Data &data = con.data;
    string http_v_str;
    string req_uri;

    size_t s = 0, e = 0;
    // INFO GREEN "parse request header " << request_line << RESET << endl;
    data.method = method_str == "POST "  ? POST_
                  : method_str == "GET " ? GET_
                                         : DELETE_;
    request_line = substr(LINE, request_line, method_str.length(), request_line.length());
    if (request_line[0] == ' ' || request_line.empty())
        return data.check(LINE, true, HTTP_BAD_REQUEST, "invalid http header");

    e = request_line.rfind(" ");
    if (e == 0)
        return data.check(LINE, true, HTTP_BAD_REQUEST, "empty uri");
    req_uri = substr(LINE, request_line, 0, e);
    http_v_str = substr(LINE, request_line, e + 1, request_line.length());
    // cout << GREEN "line " << LINE << " <" << req_uri << ">" << RESET << endl;
    // cout << GREEN "line " << LINE << " <" << http_v << ">" << RESET << endl;

    bool cond = invalid_http_version(http_v_str, data.http_v, data.status);
    if (cond)
        return data.check(LINE, true, data.status, data.cause);

    return (parse_uri(data, req_uri));
}

std::string getIpAddress(const std::string &hostname)
{
    struct addrinfo hints, *res;
    int status;
    char ipstr[INET_ADDRSTRLEN];
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((status = getaddrinfo(hostname.c_str(), NULL, &hints, &res)) != 0)
        return hostname;
    void *addr;
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    addr = &(ipv4->sin_addr);
    inet_ntop(res->ai_family, addr, ipstr, sizeof(ipstr));
    freeaddrinfo(res);
    return std::string(ipstr);
}

bool parse_header(Connection &con)
{
    ssize_t e, s;
    Data &data = con.data;
    string &buff = data.requbuff;
    ssize_t pos = find(buff, CRLF);
    while (pos != string::npos)
    {
        // PARSE METHOD
        string request_line = substr(LINE, buff, 0, pos);
        // cout << "<" << request_line << ">" << endl;
        // cout << GREEN "<" << buff << ">" RESET << endl;
        if (!data.method)
        {
            string methods[] = {"GET ", "POST ", "DELETE ", ""};
            size_t i = 0;
            while (methods[i].length())
            {
                if (starts_with(request_line, methods[i]))
                {
                    if (parse_request_header(request_line, con, methods[i]))
                        break;
                }
                i++;
            }
            if (data.is_error())
            {
                // cerr << "line: " RED << LINE << "is error" << RESET << endl;
                break;
            }
            else if (i == 4)
            {
                // throw Error("method not emplemented");
                data.check(LINE, true, HTTP_METHOD_NOT_EMPLEMENTED, "NOT IMPLEMENTED");
                break;
            }
        }
        else if (request_line.length())
        {
            ssize_t epos = request_line.find_first_of(":");
            ssize_t spos = 0;
            bool cond = false;
            Data &data = con.data;
            // TODO: skip new lines till \n\n or \r\n\r\n

            if (epos != string::npos)
            {
                string key = trim(request_line.substr(spos, epos - spos));
                string value = trim(request_line.substr(epos + 1, request_line.length()));
                vector<string> line_values = split_by_space(value);
                string &val = line_values[0];
                if (to_lower(key) == "host")
                {
                    data.host = val;
                    size_t i = 0;
                    cond = data.host.length() > LIMIT_HOSTNAME;
                    // TODO: to be checked, maybe it should be removed
                    while (!cond && i < data.host.length())
                    {
                        cond = !isdigit(data.host[i]) && !isalpha(data.host[i]) && !strchr(":-.", data.host[i]);
                        i++;
                    }
                    size_t pos2 = data.host.find(":");
#if 0
                    if (pos2 != string::npos)
                    {
                        data.domain = substr(LINE, data.host, 0, pos2);

                    }
                    // TODO: host could have port after host keyword
                    data.host = getIpAddress(data.host);
#else
                    data.host = substr(LINE, data.host, 0, pos2);
#endif
                    cout << "found Host in header <" << data.host << ">" << endl;
                    data.check(LINE, cond, HTTP_BAD_REQUEST, "Invalid Host " + data.host);
                }
                if (to_lower(key) == "content-length")
                {
                    // TODO: test 0
                    cond = val.empty() || !_isdigits(val) || (data.clen = atol(val.c_str())) < 0;
                    cond = data.check(LINE, cond, HTTP_BAD_REQUEST, "Invalid Content-length: " + val);
                    data.clen_found = true;
                    cout << "data.clen: " << data.clen << endl;
                    data.flen = data.clen;
                    // return cond;
                }
                if (to_lower(key) == "content-type")
                {
                    data.ctype = val;
                    data.ctype_found = true;
                    data.check(LINE, val.empty(), HTTP_BAD_REQUEST, "Content-Type empty");
                }
                if (to_lower(key), "connection")
                {
                    string connection = val;
                    if (connection == "keep-alive")
                        data.keep_alive = true;
                    data.connection_found = true;
                    data.check(LINE, val.empty(), HTTP_BAD_REQUEST, "connection empty");
                }
                if (to_lower(key) == "transfer-encoding")
                {
                    // TODO: handle GET with chunck body, search if required
                    // maybe it could be chunked;
                    cout << "found Transfer-Encoding, trans <" << val << ">" << endl;
                    if (val == "chunked")
                    {
                        data.trans = CHUNKED_;
                        cout << "Transfer-Encoding: chunked " << endl;
                    }
                    data.trans_found = true;
                    data.check(LINE, val.empty(), HTTP_BAD_REQUEST, "Transfer-Encoding empty");
                }
            }
        }
        if (request_line.empty())
        {
            cout << "reached the end" << endl;
            // throw Error(LINE, "debuging");
            if (data.is_error())
            {
                data.header_parsed = true;
                break;
            }
            if (!(data.method == POST_ && data.trans == CHUNKED_))
                buff = substr(LINE, buff, strlen(CRLF), buff.length());
            if (data.method == POST_ && data.trans != CHUNKED_ && !data.clen_found)
            {
                data.status = HTTP_LENGTH_REQUIRED;
                data.cause = "content-Length is missing";
            }
            if (!data.host_found)
            {
                data.status = HTTP_BAD_REQUEST;
                data.cause = "Host required";
            }
            data.header_parsed = true;
            break;
        }
        buff = substr(LINE, buff, pos + strlen(CRLF), buff.length());
        pos = find(buff, CRLF);
    }
    return data.header_parsed;
}

#if 0
int main()
{
    try
    {
        Connection con = {};
        con.data.requbuff = "GET http://example.com:8080/path/file%20name%3Fquery%3Dvalue HTTP/1.0\r\n"
                            "Host: mohammed\r\n"
                            "Content-Length: 123\r\n"
                            "Content-Type: text/html\r\n"
                            "\r\n";
        string &buff = con.data.requbuff;

        string request_line = buff;
        parse_header(con);
        cout << "method  : <" << to_string(con.data.method) << ">" << endl;
        cout << "uri     : <" << con.data.uri << ">" << endl;
        cout << "host    : <" << con.data.host << ">" << endl;
        cout << "queries : <" << con.data.queries << ">" << endl;

        // parse_header_value(buff, con);
    }
    catch (std::exception &err)
    {
        cerr << RED "Error: " << err.what() << RESET << endl;
    }
}
#endif