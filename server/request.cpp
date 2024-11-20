/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhrima <mhrima@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/07 16:53:19 by mhrima            #+#    #+#             */
/*   Updated: 2024/06/07 16:53:20 by mhrima           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.hpp"

ssize_t find_sep(string &buff, string &sep)
{
    string sep0 = CRLF;
    string sep1 = LF;

    ssize_t pos0 = find(buff, (char *)sep0.c_str());
    ssize_t pos1 = find(buff, (char *)sep1.c_str());

    if (pos0 == (ssize_t)string::npos || pos1 < pos0)
    {
        sep = sep1;
        return pos1;
    }
    sep = sep0;
    return pos0;
}

bool is_bad_uri1(string uri)
{
    string str = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                 "abcdefghijklmnopqrstuvwxyz"
                 "0123456789"
                 "-._~:/?#[]@!$&'()*+,;=%";

    for (size_t i = 0; i < uri.length(); i++)
        if (!strchr(str.c_str(), uri[i]))
            return true;
    return false;
}

bool is_bad_uri2(string uri)
{
    size_t dots_pos = find(uri, (char *)"..");
    bool cond = false;
    cond = cond || (dots_pos != std::string::npos && (dots_pos < uri.length() - 2 && uri[dots_pos + 2] == '/'));
    dots_pos = uri.find("..");
    cond = cond || (dots_pos != std::string::npos && dots_pos == 0); // at beginning
    dots_pos = uri.find("/..");
    cond = cond || (dots_pos != std::string::npos && dots_pos == uri.size() - 3); // at end
    dots_pos = uri.find("/../");
    cond = cond || dots_pos != std::string::npos;
    return cond;
}

void parse_first_line(Connection &con, string line, Header &header)
{
    size_t s = 0, e = 0;
    bool cond;

    // PARSE URI
    e = find(line, (char *)" ");
    cond = e == 0 || e == string::npos;
    if (check_error(LINE, con, cond, HTTP_BAD_REQUEST, "invalid request header"))
        return;

    string uri = substr(LINE, line, 0, e);
    if (check_error(LINE, con, is_bad_uri1(uri), HTTP_BAD_REQUEST, "Invalid uri: " + uri))
        return;

    uri = parse_hexadecimal(uri);
    line = substr(LINE, line, e + 1, line.length());
    s = e = 0;

    if (check_error(LINE, con, is_bad_uri2(uri), HTTP_BAD_REQUEST, "Invalid uri: " + uri))
        return;
    if (check_error(LINE, con, uri.length() > LIMIT_URI, HTTP_URI_TO_LARGE, "uri to large"))
        return;

    if (starts_with(uri, "http://"))
    {
        string host;
        s = uri.find("/", strlen("http://"));
        cond = s == string::npos;
        if (check_error(LINE, con, cond, HTTP_BAD_REQUEST, "Invalid url: " + uri))
            return;
        host = substr(LINE, uri, strlen("http://"), s);
        uri = substr(LINE, uri, s, uri.length());
        s = host.find(":");
        if (s != string::npos)
            host = substr(LINE, host, 0, s);
        if (check_error(LINE, con, host.empty(), HTTP_BAD_REQUEST, "empty host in uri"))
            return;
        if (host == machine_ip_address)
            host = "127.0.0.1";
        header.hosts.push_back(host);
    }
    size_t q_pos = find(uri, (char *)"?");
    if (q_pos != string::npos)
    {
        // found queries
        s = q_pos;
        while (!isspace(uri[s]) && s < uri.length())
            s++;
        header.queries = substr(LINE, uri, q_pos + 1, s);
        uri = substr(LINE, uri, 0, q_pos);
    }
    header.uri = clear_path(uri);
    if (check_error(LINE, con, header.uri.empty(), HTTP_BAD_REQUEST, "empty uri"))
        return;
    if (check_error(LINE, con, header.uri[0] != '/', HTTP_BAD_REQUEST, "uri doesn't start with /, " + header.uri))
        return;

    // PARSE HTTP version
    if (check_error(LINE, con, !starts_with(line, "HTTP/"), HTTP_BAD_REQUEST, "expected HTTP/version"))
        return;
    line = substr(LINE, line, strlen((char *)"HTTP/"), line.length());
    size_t dot = find(line, (char *)".");
    if (check_error(LINE, con, dot == string::npos, HTTP_BAD_REQUEST, "Invalid HTTP: . not found"))
        return;

    string l = substr(LINE, line, 0, dot);
    string r = substr(LINE, line, dot + 1, line.length());
    if (check_error(LINE, con, !_isdigits(l) || !_isdigits(r), HTTP_BAD_REQUEST, "expected HTTP/d.d"))
        return;
    long l0 = atol(l.c_str());
    long r0 = atol(r.c_str());
    if (check_error(LINE, con, l0 != 1 || (r0 != 0 && r0 != 1), HTTP_INSUPPORTED_HTTP, "Insupported HTTP version"))
        return;
    if (r0 == 0)
        header.version = HTTP1_0;
    if (r0 == 1)
        header.version = HTTP1_1;
}

bool first_line_valid(string line)
{
    size_t first_space = line.find(' ');
    if (first_space == string::npos || first_space == 0)
        return false;
    size_t second_space = line.find(' ', first_space + 1);
    if (second_space == string::npos || second_space == first_space + 1)
        return false;
    if (line.find(' ', second_space + 1) != string::npos)
        return false;

    return !line.substr(0, first_space).empty() &&
           !line.substr(first_space + 1, second_space - first_space - 1).empty() &&
           !line.substr(second_space + 1).empty();
}

void parse_header(Connection &con, string &buff, Header &header)
{
    if (header.conds["parsed"] || is_error(con.res.status) || buff.empty())
        return;

    GLOG("parse header") << endl;
    string sep;
    bool cond = false;
    size_t pos = find_sep(buff, sep);
    while (pos != string::npos)
    {
        string line = substr(LINE, buff, 0, pos);
        GLOG("parse") << "<" << line << ">" << END;
        if (!header.method)
        {
            string strs[] = {"GET ", "POST ", "DELETE ", ""};
            Method methods[] = {GET_, POST_, DELETE_, (Method)0};
            for (size_t i = 0; strs[i].length(); i++)
            {
                if (starts_with(line, strs[i]))
                {
                    // INFO GREEN "parse first header line <" << request_line << ">" << RESET << endl;
                    header.method = methods[i];
                    line = substr(LINE, line, strs[i].length(), line.length());
                    parse_first_line(con, line, header);
                    break;
                }
            }
            if (is_error(con.res.status))
                break;
            if (!header.method)
            {
                if (check_error(LINE, con, first_line_valid(line), HTTP_NOT_IMPLEMENTED, "Unknown method"))
                    break;
                else
                    check_error(LINE, con, true, HTTP_BAD_REQUEST, "expecting: METHOD /URI HTTP_VERSION");
                break;
            }
        }
        else if (line.length())
        {
            size_t e = line.find_first_of(":");
            cond = false;
            if (check_error(LINE, con, e == string::npos, HTTP_BAD_REQUEST, "Invalid header format"))
                break;
            string key = _tolower(trim(substr(LINE, line, 0, e)));
            string value = trim(substr(LINE, line, e + 1, line.length()));
            // e = find(value, (char *)";");
            // if (key != "cookie" && e != string::npos)
                // value = substr(LINE, value, 0, e);
            if (check_error(LINE, con, key.empty(), HTTP_BAD_REQUEST, "key empty"))
                break;
            if (check_error(LINE, con, value.empty(), HTTP_BAD_REQUEST, key + " has empty value"))
                break;
            if (check_error(LINE, con, header.found[key], HTTP_BAD_REQUEST, "repeated " + key))
                break;
            else
                header.found[key] = true;
            if (key == "host")
            {
                cond = value.length() > LIMIT_HOSTNAME;
                if (!check_error(LINE, con, cond, HTTP_BAD_REQUEST, "Invalid Host " + value))
                    header.hosts.insert(header.hosts.begin(), value);
            }
            else if (key == "content-length")
            {
                ssize_t len = atol(value.c_str());
                cond = !_isdigits(value) || (header.method != POST_ && len == 0) || len < 0;
                check_error(LINE, con, cond, HTTP_BAD_REQUEST, "Invalid Content-length: " + value);
                header.content_len = (size_t)len;
            }
            else if (key == "content-type")
                header.content_type = value;
            else if (key == "connection")
                header.connection = value;
            else if (key == "transfer-encoding")
            {
                value = _tolower(value);
                cond = value != "chunked" && value != "binary";
                if (check_error(LINE, con, cond, HTTP_NOT_IMPLEMENTED, key + ":" + value + " not implemented"))
                    break;
                header.transfer = value;
            }
            else if (key == "cookie")
            {
                if (header.cookies.size())
                    header.cookies[0] = value;
                else
                    header.cookies.push_back(value);
            }
            if (is_error(con.res.status))
                break;
        }
        else if (line.empty())
        {
            buff = substr(LINE, buff, sep.length(), buff.length());
            check_error(LINE, con, header.hosts.empty(), HTTP_BAD_REQUEST, "Host is required");
            cond = header.method == POST_ && header.transfer != "chunked" && !header.found["content-length"];
            check_error(LINE, con, cond, HTTP_LENGTH_REQUIRED, "Content-Length is missing");
            if (header.transfer == "chunked" && !starts_with(buff, "\r\n"))
            {
                header.content_len = 0;
                buff = "\r\n" + buff;
            }
            header.conds["parsed"] = true;
            GLOG("request parsed") << END;
            GLOG("Method") << to_string(header.method) << END;
            GLOG("uri") << header.uri << END;
            GLOG("HTTP version") << to_string(header.version) << END;
            GLOG("Method") << to_string(header.method) << END;
            for (size_t i = 0; i < header.hosts.size(); i++)
                GLOG("Host (" + to_string(i) + ")") << header.hosts[i] << END;
            GLOG("Content-Length") << header.content_len << END;
            GLOG("Content-Type") << header.content_type << END;
            GLOG("Transfer") << header.transfer << END;
            GLOG("Connection") << header.connection << END;
            if (is_error(con.res.status))
                GLOG("is_error") << "status: " << to_string(con.res.status) << ", cause: " << con.res.cause << END;
            break;
        }
        buff = substr(LINE, buff, pos + sep.length(), buff.length());
        pos = find_sep(buff, sep);
    }
}
