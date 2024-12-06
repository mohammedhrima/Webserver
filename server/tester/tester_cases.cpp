/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tester_cases.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhrima <mhrima@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/07 16:53:42 by mhrima            #+#    #+#             */
/*   Updated: 2024/06/07 17:04:04 by mhrima           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <vector>
using namespace std;

// string genrate_post_chunked_request(string uri, string host)
// {
//     string res = "POST / HTTP/1.1\n"
//         "host: random\n"
//         "Transfer-"
//         ;
//     return res;
// }

vector<string> generate_tests()
{
    string body = "name=mohammed&age=15";
    int i = 0;
    while (i < 10)
    {
        body += body;
        i++;
    };
    string tests[] = {
        "GET http://name2/files/001.txt HTTP/1.1\r\n"
        "Host: name1\r\n"
        "\r\n"
        "GET /test1/files/003.txt HTTP/1.1\r\n"
        "Host: name1\r\n"
        "\r\n"
        "Host: 127.0.0.1:17000\r\n"
        "GET /test1/files/001.txt HTTP/1.1\r\n"
        "Host: name1\r\n"
        "Content-Length: " +
            to_string(body.length()) + "\r\n"
                                       "\r\n" +
            body + 
        "GET /test1/files/003.txt HTTP/1.1\r\n"
        "Host: name1\r\n"
        "\r\n"
        ,

        // "GET /001.txt HTTP/1.1\n"
        // "Host: name\n\n"
        // "GET /001.txt HTTP/1.1\n"
        // "Host: name\n\n"
        // "GET /001.txt HTTP/1.1\n"
        // "Host: name\n\n",

        // "POST / HTTP/1.1\n"
        // "host: random\n"
        // "content-length: " +
        //     to_string(body.length()) + "\n\n" +
        //     body,

        // "fffffffffffflfkff\n",

        // "GET /%3001.txt HTTP/1.1\n"
        // "host: name1\n\n",

        // "GET /0%2001.txt HTTP/1.1\n"
        // "host:          name1\n\n",

        // "GET  /index.html HTTP/1.1\n"
        // "hosT: name2\n\n",

        // ,
        // "POST /cgi/post/post.py HTTP/1.1\n"
        // "content-length:" +
        //     to_string(body.length()) + "\n"
        //                                "host:name1\n\n" +
        //     body + "GET /files/file.html HTTP/1.1\n"
        //            "hosT:name1\n\n",
        // "GET /cgi/get/simple/hello.py HTTP/1.1\n"
        // "Host: name1\n\n"
    };
    vector<string> res;
    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
        res.push_back(tests[i]);

    return res;
}
