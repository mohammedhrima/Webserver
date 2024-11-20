/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   header.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhrima <mhrima@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/07 16:53:51 by mhrima            #+#    #+#             */
/*   Updated: 2024/06/07 16:53:52 by mhrima           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HEADER_HPP
#define UTILS_HEADER_HPP

#include <poll.h>
#include <fcntl.h>
#include <sys/types.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <sys/stat.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <map>
#include <iomanip>
#include <sstream>
#include <sys/time.h>
#include <cassert>
#include <signal.h>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <set>
#include <dirent.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <cstdio>

// MACROS
#define SOCK_TIMEOUT 5
// #define SOCK_REMOVE_TIMEOUT 1
#define KEEP_ALIVE_TIMEOUT 10
#define CGI_TIMEOUT 10
#define STDOUT 1
#define STDIN 0

#define POLLTIMEOUT 1000
#define LISTEN_LEN SOMAXCONN
#define BUFFSIZE 16000
#define MAX_HEADER_SIZE 8192

#define LIMIT_HOSTNAME 63
#define LIMIT_URI 4096

#define CR "\r"
#define LF "\n"
#define CRLF (char *)"\r\n"
#define CRLFCRLF (char *)"\r\n\r\n"
#define ADDRLEN 4096
#define MACRO_TO_STRING(f) #f
#define LINE __LINE__
#define FILE __FILE__
#define FUNC __func__
#define SPLIT "=========================================================================="

// AUTOINDEX / METHODS
#define none 0
#define on 1
#define off 2

// STATS
#define IS_ACCESS S_IRUSR
#define IS_FILE S_IFREG
#define IS_DIR S_IFDIR

// COLORS
#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define CYAN "\033[0;36m"
#define RESET "\033[0m"

// DEBUGING
#define DEBUG cout << GREEN << FUNC << ":" << LINE << ": " << RESET

#define RLOG(msg) cout << RED << setw(16) << msg << ": " << RESET
#define CLOG(msg) cout << CYAN << setw(16) << msg << ": " << RESET
#define GLOG(msg) cout << GREEN << setw(16) << msg << ": " << RESET

#define END RESET << endl;
// #define ERROR cerr << RED << setw(18) << "error: " << RESET

using namespace std;
typedef struct sockaddr_storage sockaddr_storage;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef struct addrinfo addrinfo;
typedef struct timeval timeval;
typedef struct dirent dirent;
typedef struct pollfd pollfd;
typedef struct stat State;

string to_string(size_t size);
class Error : public exception
{
private:
    string message;

public:
    Error(string msg) throw() : message(msg) {}
    Error(ssize_t line, string msg) throw()
    {
        message = "line " + to_string(line) + " " + msg;
    }
    virtual ~Error() throw() {}
    const char *what() const throw() { return message.c_str(); }
};

#endif