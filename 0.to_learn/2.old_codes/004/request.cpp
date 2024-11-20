#include "header.hpp"

// PARSE HEADER
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
    ssize_t dots_pos = find(url, (char *)"..");
    bool cond = false;
    // djdjdjfd../
    cond = cond || (dots_pos != std::string::npos && (dots_pos < url.length() - 2 && url[dots_pos + 2] == '/'));

    dots_pos = url.find("..");
    cond = cond || (dots_pos != std::string::npos && dots_pos == 0); // at beginning

    dots_pos = url.find("/..");
    cond = cond || (dots_pos != std::string::npos && dots_pos == url.size() - 3); // at end

    dots_pos = url.find("/../");
    cond = cond || dots_pos != std::string::npos;

    return cond;
}

bool check(int line, Connection &con, bool cond, Status status_, string cause_)
{
    if (cond && !is_error(con.data))
    {
        cout << RED "check did found error in line " << line << RESET << endl;
        con.data.cause = cause_;
        con.data.status = status_;
        con.action = WRITE_;
        con.data.header_parsed = true;
        ERROR << "clear buffer in check"<< END;
        con.buff.clear();
    }
    return cond;
}

void parse_request_header(string &line, Connection &con, Method method)
{
    Data &data = con.data;
    string uri;
    size_t s = 0, e = 0;
    data.method = method;

    // PARSE URI
    e = line.find(' ');
    if (check(LINE, con, e == 0 || e == string::npos, HTTP_BAD_REQUEST, "invalid request header"))
        return;
    uri = substr(LINE, line, 0, e);
    line = substr(LINE, line, e + 1, line.length());
    string &queries = data.queries;
    s = 0;
    e = 0;

    uri = parse_hexadecimal(uri);
    // if (
    check(LINE, con, is_bad_url(uri), HTTP_BAD_REQUEST, "Invalid uri");
    // )
    // return;
#if 1
    if (starts_with(uri, "http://"))
    {
        string host;
        s = uri.find("/", strlen("http://"));
        if (s != string::npos)
        {
            host = substr(LINE, uri, strlen("http://"), s);
            uri = substr(LINE, uri, s, uri.length());
        }
        else
        {
            host = substr(LINE, uri, strlen("http://"), s);
            uri = substr(LINE, uri, s + 1, uri.length());
        }
        // TODO: clear_path host and uri
        s = host.find(":");
        if (s != string::npos)
            host = substr(LINE, host, 0, s);
        if (!check(LINE, con, host.empty(), HTTP_BAD_REQUEST, "empty host in uri"))
        {
            if (host == machine_ip_address)
                host = "127.0.0.1";
            data.hosts.push_back(host);
        }
    }
#endif
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
    data.uri = clear_path(uri);
    check(LINE, con, data.uri.empty(), HTTP_BAD_REQUEST, "uri empty uri");
    check(LINE, con, data.uri[0] != '/', HTTP_BAD_REQUEST, "uri doesn't start with / <" + data.uri + ">");

    // PARSE HTTP version
    if (check(LINE, con, !starts_with(line, "HTTP/"), HTTP_BAD_REQUEST, "Invalid HTTP"))
        return;
    size_t dot = 0;
    line = substr(LINE, line, 5, line.length());
    dot = find(line, (char *)".");
    if (check(LINE, con, dot == string::npos, HTTP_BAD_REQUEST, "Invalid HTTP: . not found"))
        return;

    string l = substr(LINE, line, 0, dot);
    string r = substr(LINE, line, dot + 1, line.length());
    if (check(LINE, con, !_isdigits(l) || !_isdigits(r), HTTP_BAD_REQUEST, "Invalid HTTP"))
        return;
    long l0 = atol(l.c_str());
    long r0 = atol(r.c_str());
    if (check(LINE, con, l0 != 1 || (r0 != 0 && r0 != 1), HTTP_INSUPPORTED_HTTP, "Insupported HTTP version"))
        return;
    if (r0 == 0)
        con.data.http_version = HTTP1_0;
    if (r0 == 1)
        con.data.http_version = HTTP1_1;
}

ssize_t find_sep(string &buff, string &sep)
{
    string sep0 = CRLF;
    string sep1 = LF;

    ssize_t pos0 = find(buff, (char *)sep0.c_str());
    ssize_t pos1 = find(buff, (char *)sep1.c_str());

    if (pos0 == string::npos || pos1 < pos0)
    {
        sep = sep1;
        return pos1;
    }
    sep = sep0;
    return pos0;
}

bool parse_header(Connection &con)
{
    ssize_t e, s;
    Data &data = con.data;
    string &buff = con.readbuff;
    ssize_t pos;
    string sep;

    pos = find_sep(con.readbuff, sep);
    while (pos != string::npos)
    {
        string request_line = substr(LINE, buff, 0, pos);
        cout << RED << "<" << request_line << ">" << endl;
        if (!data.method)
        {
            struct
            {
                string str;
                Method enum_;
            } methods[] = {{"GET ", GET_}, {"POST ", POST_}, {"DELETE ", DELETE_}, {"", (Method)0}};
            size_t i = 0;
            while (methods[i].str.length())
            {
                if (starts_with(request_line, methods[i].str))
                {
                    // INFO GREEN "parse first header line <" << request_line << ">" << RESET << endl;
                    request_line = substr(LINE, request_line, methods[i].str.length(), request_line.length());
                    parse_request_header(request_line, con, methods[i].enum_);
                    break;
                }
                i++;
            }
            if (is_error(con.data))
            {
                // cerr << "line: " RED << LINE << " is error" << RESET << endl;
                break;
            }
            if (!data.method)
            {
                // throw Error("method not emplemented");
                check(LINE, con, true, HTTP_METHOD_NOT_IMPLEMENTED, "Unknown method");
                break;
            }
            cout << con << endl;
            // throw Error(LINE, "");
        }
        else if (request_line.length())
        {
            ssize_t epos = request_line.find_first_of(":");
            ssize_t spos = 0;
            bool cond = false;
            Data &data = con.data;

            if (epos != string::npos)
            {
                string key = trim(request_line.substr(spos, epos - spos));
                string value = trim(request_line.substr(epos + 1, request_line.length()));
                vector<string> line_values = split_by_space(value);
                string &val = line_values[0];
                spos = find(val, (char *)";");
                if (spos != string::npos) // skip comment
                    val = substr(LINE, val, 0, spos);
                if (to_lower(key) == "host")
                {
                    if (check(LINE, con, data.host_found, HTTP_BAD_REQUEST, "repeated host "))
                        break;
                    string host = val;
                    size_t i = 0;
                    cond = host.length() > LIMIT_HOSTNAME;
                    // while (!cond && i < host.length())
                    // {
                    //     cond = !isdigit(host[i]) && !isalpha(host[i]) && !strchr(":-.", host[i]);
                    //     i++;
                    // }
                    size_t pos2 = find(host, (char *)":");
                    if (pos2 != string::npos)
                        host = substr(LINE, host, 0, pos2);
                    // cout << "found Host in header <" << host << ">" << endl;
                    check(LINE, con, cond, HTTP_BAD_REQUEST, "Invalid Host " + host);
                    data.hosts.push_back(host);
                    data.host_found = true;
                }
                if (to_lower(key) == "content-length")
                {
                    ssize_t len = atol(val.c_str());
                    cond = val.empty() || !_isdigits(val) || (data.method != POST_ && len == 0) || len < 0;
                    cond = check(LINE, con, cond, HTTP_BAD_REQUEST, "Invalid Content-length: " + val);
                    data.clen_found = true;
                    data.clen = len;
                    // cout << "data.clen: " << data.clen << endl;
                    data.flen = data.clen;
                }
                if (to_lower(key) == "content-type")
                {
                    data.ctype = val;
                    data.ctype_found = true;
                    check(LINE, con, val.empty(), HTTP_BAD_REQUEST, "Content-Type empty");
                }
                if (to_lower(key) == "connection")
                {
                    string connection = val;
                    if (connection == "keep-alive")
                        data.keep_alive = true;
                    data.connection_found = true;
                    check(LINE, con, val.empty(), HTTP_BAD_REQUEST, "connection empty");
                }
                if (to_lower(key) == "transfer-encoding")
                {
                    // cout << "found Transfer-Encoding, trans <" << val << ">" << endl;
                    if (val == "chunked")
                    {
                        data.trans = CHUNKED_;
                        // cout << "Transfer-Encoding: chunked " << endl;
                    }
                    data.trans_found = true;
                    check(LINE, con, val.empty(), HTTP_BAD_REQUEST, "Transfer-Encoding empty");
                }
            }
        }
        if (request_line.empty())
        {
            cout << "reached the end of header" << endl;
            if (is_error(data))
            {
                data.header_parsed = true;
                break;
            }
            // if (!(data.method == POST_ && data.trans == CHUNKED_))
            buff = substr(LINE, buff, sep.length(), buff.length());
            // check for host
            bool cond = !data.host_found;
            check(LINE, con, cond, HTTP_BAD_REQUEST, "Host is required");
            // check for content-length
            if (!cond)
            {
                cond = (data.method == POST_ && data.trans != CHUNKED_ && !data.clen_found);
                check(LINE, con, cond, HTTP_LENGTH_REQUIRED, "content-Length is missing");
            }
            data.header_parsed = true;
            // cout << GREEN;
            // cout << "Method         : <" << to_string(con.data.method) << ">" << endl;
            // cout << "uri            : <" << con.data.uri << ">" << endl;
            // cout << "HTTP version   : <" << to_string(con.data.http_version) << ">" << endl;
            // cout << "Host           : <";
            // for (size_t i = 0; i < con.data.hosts.size(); i++)
            //     cout << con.data.hosts[i] << " , ";
            // cout << ">" << endl;
            // cout << "Content-Length : <" << con.data.clen << ">" << endl;
            // cout << "Content-Type   : <" << con.data.ctype << ">" << endl;
            // cout << "keep-alive     : <" << boolalpha << con.data.keep_alive << ">" << endl;
            // cout << "======================================================================" << endl;
            // cout << con << endl;
            // cout << "======================================================================" << endl;
            // cout << RESET;
            break;
        }
        buff = substr(LINE, buff, pos + sep.length(), buff.length());
        pos = find_sep(buff, sep);
    }
    return data.header_parsed;
}

#if 0
map<string, string> memetype;
int main()
{
    Connection con = {};
    con.readbuff = "GET http://localhost/abc HTTP/1.1\r\n"
                   "Host: name1\n"
                   "Content-Type: text/html\n"
                   "\n"
                   "GET http://localhost/abc HTTP/1.1\r\n"
                   "Host: name1\n"
                   "Content-Type: text/html\n"
                   "\n"
                   ;
    parse_header(con);
}
#endif