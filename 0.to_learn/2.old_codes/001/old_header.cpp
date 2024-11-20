#include "header.hpp"

bool parse_header(Connection &con)
{
    ssize_t e, s;
    Data &data = con.data;
    string &buff = data.requbuff;
    ssize_t pos = find(buff, CRLF);
    while (pos != string::npos)
    {
        // PARSE METHOD
        if (!data.method)
        {
            if (compare(buff, "POST "))
            {
                data.method = POST_;
                data.uri = data.cut(strlen((char *)"POST "), e);
                if (data.check(LINE, data.uri.empty() || data.uri[0] != '/', HTTP_BAD_REQUEST, "invalid uri " + data.uri))
                    break;
                data.uri = parse_hexadecimal(data.uri);
                ssize_t q_pos = find(data.uri, (char *)"?");
                if (q_pos != string::npos)
                {
                    // found queries
                    cout << "found ?" << endl;
                    s = q_pos;
                    while (!isspace(data.uri[s]) && s < data.uri.length())
                        s++;
                    data.queries = substr(LINE, data.uri, q_pos + 1, s);
                    data.uri = substr(LINE, data.uri, 0, q_pos);
                    cout << "queries <" << data.queries << ">" << endl;
                }
                cout << "POST to uri <" << data.uri << ">" << endl;

                if (compare(e, buff, " HTTP/1.1\r\n") || compare(e, buff, " HTTP/1.1\n"))
                    data.http_v = HTTP1_1;
                else if (compare(e, buff, " HTTP/1.0\r\n") || compare(e, buff, " HTTP/1.0\n"))
                    data.http_v = HTTP1_0;
                else
                    data.check(LINE, true, HTTP_BAD_REQUEST, "Invalid HTTP version");
            }
            else if (compare(buff, "GET "))
            {
                data.method = GET_;
                data.uri = data.cut(strlen((char *)"GET "), e);
                // TODO: check if uri doesn't start with /, whould it be considred bad request ?
                if (data.check(LINE, data.uri.empty() || data.uri[0] != '/', HTTP_BAD_REQUEST, "Invalid uri " + data.uri))
                    break;
                data.uri = parse_hexadecimal(data.uri);

                ssize_t q_pos = find(data.uri, (char *)"?");
                if (q_pos != string::npos)
                {
                    // found queries
                    cout << "found ?" << endl;
                    s = q_pos;
                    while (!isspace(data.uri[s]) && s < data.uri.length())
                        s++;
                    data.queries = substr(LINE, data.uri, q_pos + 1, s);
                    data.uri = substr(LINE, data.uri, 0, q_pos);
                    cout << "queries <" << data.queries << ">" << endl;
                }
                data.uri = clear_path(data.uri);
                cout << "GET to uri <" << data.uri << ">" << endl;
                if (compare(e, buff, " HTTP/1.1\r\n") || compare(e, buff, " HTTP/1.1\n"))
                    data.http_v = HTTP1_1;
                else if (compare(e, buff, " HTTP/1.0\r\n") || compare(e, buff, " HTTP/1.0\n"))
                    data.http_v = HTTP1_0;
                else if (compare(e, buff, " HTTP/")) // check if there is digits after /
                    data.check(true, true, HTTP_INSUPPORTED_HTTP, " HTTP version");
                else
                    data.check(LINE, true, HTTP_BAD_REQUEST, "Invalid HTTP version");
            }
            else if (data.check(LINE, true, HTTP_METHOD_NOT_EMPLEMENTED, "NOT IMPLEMENTED"))
                break;
        }
        else if (compare(to_lower(buff), "connection: "))
        {
            string connection = data.cut(strlen((char *)"Connection: "), e);
            if (connection == "keep-alive")
                data.keep_alive = true;
            data.connection_found = true;
        }
        /*
            TODOS:
                + HEADERS:
                    + trim spaces in values in all headers keys, values, check RFC
                    + overide key if repeadted
                    + send TIMEOUT reponse befre closing
                    + followed two new lines (request ended)
                        + like \n\n => \r\n\r\n
                    + if no host, bad request
                    + do something with content-length
                + GET:
        */
        //
        // TODO: ask zakaria to_lower, about this one
        else if (compare(to_lower(buff), "host: "))
        {
            data.host = data.cut(strlen((char *)"Host: "), e);
            cout << "found Host <" << data.host << ">" << endl;
            size_t i = 0;
            bool cond = data.host.length() > LIMIT_HOSTNAME;
            // TODO: to be checked, maybe it should be removed
            while (!cond && i < data.host.length())
            {
                cond = !isdigit(data.host[i]) && !isalpha(data.host[i]) && !strchr(":-.", data.host[i]);
                i++;
            }
            if (data.check(LINE, cond, HTTP_BAD_REQUEST, "Invalid Host " + data.host))
                break;
            data.host_found = true;
        }
        else if (compare(to_lower(buff), "content-type: "))
        {
            // parse_header_value(buff);
            data.ctype = data.cut(strlen((char *)"Content-Type: "), e);
            cout << "found Content-type <" << data.ctype << ">" << endl;
            data.check(LINE, data.ctype.empty(), HTTP_BAD_REQUEST, "Content-Type empty");
            data.ctype_found = true;
        }
        else if (compare(to_lower(buff), "content-length: "))
        {
            // TODO: test 0
            // test no numericl values
            string val = data.cut(strlen((char *)"Content-Length: "), e);
            cout << "found Content-Length, val <" << val << "> check " << (val.empty() || !_isdigits(val)) << endl;
            bool cond = data.check(LINE, val.empty(), HTTP_LENGTH_REQUIRED, "Content-Length empty"); // 411
            !cond &&data.check(LINE,
                               ((data.clen = atol(val.c_str())) < 0) || !_isdigits(val), HTTP_BAD_REQUEST,
                               "Invalid Content-length:"); // 400
            cout << "data.clen: " << data.clen << endl;
            data.flen = data.clen;
            data.clen_found = true;
        }
        else if (compare(to_lower(buff), "transfer-encoding: ") && data.method == POST_)
        {
            string trans = data.cut(strlen((char *)"Transfer-Encoding: "), e);
            cout << "found Transfer-Encoding, trans <" << trans << ">" << endl;
            data.check(LINE, trans.empty(), HTTP_BAD_REQUEST, "Transfer-Encoding empty");
            if (trans == "chunked")
            {
                data.trans = CHUNKED_;
                cout << "Transfer-Encoding: chunked " << endl;
            }
            data.trans_found = true;
        }
        if (compare(buff, CRLF))
        {
            if (data.is_error())
            {
                data.header_parsed = true;
                break;
            }
            if (!(data.method == POST_ && data.trans == CHUNKED_))
            {
                buff = substr(LINE, buff, strlen(CRLF), buff.length());
            }
            if (data.method == POST_ && data.trans != CHUNKED_ && !data.clen_found)
            {
                data.status = HTTP_LENGTH_REQUIRED;
                data.cause = "content-Length is missing";
            }
            data.header_parsed = true;
            break;
        }
        buff = substr(LINE, buff, pos + strlen(CRLF), buff.length());
        pos = find(buff, CRLF);
    }
    //  throw Error(LINE, "debuging");
    return data.header_parsed;
}