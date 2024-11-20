#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdbool.h>
#include <ctype.h>

#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <sys/epoll.h>
#include <netdb.h>

#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define RESET "\033[0m"

#define READBYTES 1024
#define ADDRLEN 4096
#define MAXLEN 4096

#define BUFF 1024
#define HOST "127.0.0.1"
#define SPORT "17000"
#define CPORT 17000
#define MAX_EVENTS 10
#define EPOLL_TIMEOUT 1000

#define END "\r\n"
#define HEADER_END "\r\n\r\n"

typedef struct addrinfo addrinfo;
typedef struct sockaddr_storage sockaddr_storage;
typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct timeval timeval;
typedef struct epoll_event epoll_event;
typedef struct in_addr in_addr;
typedef enum Method Method;
typedef enum Action Action;
typedef struct Socket Socket;

#define check(cond, ...) errput_(__LINE__, (cond), __VA_ARGS__)
#define debug(...) debug_(__LINE__, __VA_ARGS__)

struct
{
    char *ext;
    char *type;
} extentions[] = {
    {".aac", "audio/aac"},
    {".abw", "application/x-abiword"},
    {".apng", "image/apng"},
    {".arc", "application/x-freearc"},
    {".avif", "image/avif"},
    {".avi", "video/x-msvideo"},
    {".azw", "application/vnd.amazon.ebook"},
    {".bin", "application/octet-stream"},
    {".bmp", "image/bmp"},
    {".bz", "application/x-bzip"},
    {".bz2", "application/x-bzip2"},
    {".cda", "application/x-cdf"},
    {".csh", "application/x-csh"},
    {".css", "text/css"},
    {".csv", "text/csv"},
    {".doc", "application/msword"},
    {".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
    {".eot", "application/vnd.ms-fontobject"},
    {".epub", "application/epub+zip"},
    {".gz", "application/gzip"},
    {".gif", "image/gif"},
    {".htm", "text/html"},
    {".html", "text/html"},
    {".ico", "image/vnd.microsoft.icon"},
    {".ics", "text/calendar"},
    {".jar", "application/java-archive"},
    {".jpeg", "image/jpeg"},
    {".jpg", "image/jpeg"},
    {".js", "text/javascript"},
    {".json", "application/json"},
    {".jsonld", "application/ld+json"},
    {".mid", "audio/midi"},
    {".midi", "audio/midi"},
    {".mjs", "text/javascript"},
    {".mp3", "audio/mpeg"},
    {".mp4", "video/mp4"},
    {".mpeg", "video/mpeg"},
    {".mpkg", "application/vnd.apple.installer+xml"},
    {".odp", "application/vnd.oasis.opendocument.presentation"},
    {".ods", "application/vnd.oasis.opendocument.spreadsheet"},
    {".odt", "application/vnd.oasis.opendocument.text"},
    {".oga", "audio/ogg"},
    {".ogv", "video/ogg"},
    {".ogx", "application/ogg"},
    {".opus", "audio/opus"},
    {".otf", "font/otf"},
    {".png", "image/png"},
    {".pdf", "application/pdf"},
    {".php", "application/x-httpd-php"},
    {".ppt", "application/vnd.ms-powerpoint"},
    {".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
    {".rar", "application/vnd.rar"},
    {".rtf", "application/rtf"},
    {".sh", "application/x-sh"},
    {".svg", "image/svg+xml"},
    {".tar", "application/x-tar"},
    {".tif", "image/tiff"},
    {".tiff", "image/tiff"},
    {".ts", "video/mp2t"},
    {".ttf", "font/ttf"},
    {".txt", "text/plain"},
    {".vsd", "application/vnd.visio"},
    {".wav", "audio/wav"},
    {".weba", "audio/webm"},
    {".webm", "video/webm"},
    {".webp", "image/webp"},
    {".woff", "font/woff"},
    {".woff2", "font/woff2"},
    {".xhtml", "application/xhtml+xml"},
    {".xls", "application/vnd.ms-excel"},
    {".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
    {".xml", "application/xml"},
    {".xul", "application/vnd.mozilla.xul+xml"},
    {".zip", "application/zip"},
    {".3gp", "video/3gpp; audio/3gpp"},
    {".3g2", "video/3gpp2; audio/3gpp2"},
    {".7z", "application/x-7z-compressed"},
    {0, 0} // End of the array
};

enum Method
{
    GET_ = 11,
    POST_,
    DELETE_,
};

enum Action
{
    READ_ = 22,
    WRITE_
};

struct Socket
{
    /*
        TODOS:
            + you might have file with 0 length but you have to create it
    */
    Socket *left;
    Socket *right;

    // bool ready;

    /*=========== action part =========*/
    unsigned char *content;
    Method method;
    Action action;
    int socket_fd;
    size_t current_length;
    // size_t start_connection;

    /*============ filename ==========*/
    char *filename;
    FILE *fileptr;
    bool header_sent;
    size_t Content_length;
    char *Content_type;
};

#endif

// UTILS
void errput_(int line, int cond, char *fmt, ...)
{
    if (cond)
    {
        int errno_save;
        va_list ap;
        errno_save = errno;

        va_start(ap, fmt);
        printf("%s%d: Error: ", RED, line);
        vfprintf(stdout, fmt, ap);
        fflush(stdout);
        if (errno_save != 0)
        {
            printf(" (errno = %d) : %s\n", errno_save, strerror(errno));
            fflush(stdout);
        }
        printf("%s", RESET);
        va_end(ap);
        exit(1);
    }
}

void debug_(int line, char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    printf("line %d: ", line);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
}

int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void *find(void *ptr, void *to_find, size_t ptr_len, size_t to_find_len)
{
    int i = 0;
    unsigned char *left = (unsigned char *)ptr;
    unsigned char *right = (unsigned char *)to_find;
    while (i < ptr_len - to_find_len)
    {
        int j = 0;
        if (left[i] == right[j])
        {
            while (j < to_find_len && i + j < ptr_len && left[i + j] == right[j])
                j++;
            if (j == to_find_len)
                return left + i + j;
        }
        i++;
    }
    return NULL;
}

char *strjoin(char *string1, char *string2)
{
    size_t len = 0;
    len = string1 ? len + strlen(string1) : len;
    len = string2 ? len + strlen(string2) : len;

    char *res = calloc(len + 1, sizeof(char));
    string1 &&strcpy(res, string1);
    string2 &&strcpy(res + strlen(res), string2);
    return res;
}

void *memejoin(void *left, void *right, size_t llen, size_t rlen)
{
    unsigned char *result = calloc(llen + rlen + 1, 1);
    memcpy(result, left, llen);
    memcpy(result + llen, right, rlen);
    return result;
}

Socket *new_socket(Socket *new, int fd)
{
    new->action = READ_;
    new->socket_fd = fd;
    new->Content_length = -1;
    // new->method = method;
    // new->filename = filename;
    // new->length = length;
    return new;
}

void set_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    check(flags == -1, "fcntl");
    flags |= O_NONBLOCK;
    check(fcntl(fd, F_SETFL, flags) < 0, "fcntl");
}

int main()
{
    int server_fd, opt = 1, epoll_fd, bytes, num_events, client_fd, s, e;
    char address[ADDRLEN], content[BUFF], header[MAXLEN], *currDir;
    unsigned char *tmp, *tmp0;
    char *type, *len_str; //, *filename;
    debug("server: Configuring local address...\n");
    addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    addrinfo *addr;
    /*
        getaddrinfo params:
          + char *hostname / address as string
          + char *servie / port number as string
          + addrinfo *hints
          + addrinfo *result
  */
    check(getaddrinfo(NULL, SPORT, &hints, &addr) != 0, "failed getting address infos");

    debug("server: Creating socket...\n");
    check((server_fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) < 0, "socket creation");
    debug("server: Setting socket...\n");
    check(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(int)) == -1, "seting socket");
    debug("server: Binding...\n");
    check(bind(server_fd, addr->ai_addr, addr->ai_addrlen) != 0, "bind failed");
    freeaddrinfo(addr);
    debug("server: Listening...\n");
    check(listen(server_fd, SOMAXCONN) != 0, "listen failed");
    debug("server: create epoll instance\n");
    check((epoll_fd = epoll_create1(0)) < 0, "epoll_create1 failed");
    debug("server: adding server socket to epoll\n");
    epoll_event event = (epoll_event){
        .events = EPOLLIN | EPOLLOUT,
        .data.fd = server_fd,
    };
    // EPOLERROR, EPOLLHANGUP
    check(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0, "epoll_ctl failed");
    epoll_event *events = calloc(MAX_EVENTS, sizeof(epoll_event));
    size_t sock_len = MAX_EVENTS;
    Socket *sockets = calloc(sock_len + 1, sizeof(Socket));

    new_socket(&sockets[server_fd], server_fd);
    check(events == NULL || sockets == NULL, "calloc failed");
    debug("server: waiting listening on PORT(%s)...\n", SPORT);
    int evpos = 0;
    while (1)
    {
        // TODO: add epoll timeout
        debug("%sserver: waiting for event (evpos:%d)... %s\n", GREEN, evpos, RESET);
        check((num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1)) < 0, "epoll_wait failed");
        debug("%safter waiting events (%d)%s\n", GREEN, num_events, RESET);

        for (int i = 0; i < num_events; i++)
        {
            // new connection
            if (events[i].data.fd == server_fd)
            {
                // accept client connection
                sockaddr_storage client_addr;
                socklen_t clen = sizeof(client_addr);
                check((client_fd = accept(server_fd, (sockaddr *)&client_addr, &clen)) < 0, "accept failed");
                set_non_blocking(client_fd);
                // add client socket to epoll
                event.events = EPOLLIN | EPOLLOUT;
                event.data.fd = client_fd;
                check(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0, "epoll_ctl failed");
                // TODO: check if inet_ntop failed
#if 0
                inet_ntop(AF_INET, &(((sockaddr_in *)&client_addr)->sin_addr), address, INET_ADDRSTRLEN);
#else
                inet_ntop(AF_INET, (sockaddr_in *)&client_addr, address, INET_ADDRSTRLEN);
#endif
                debug("server: new connection from fd:(%d), address:(%s)\n", client_fd, address);
                if (client_fd > sock_len)
                    sockets = realloc(sockets, (sock_len *= 2) * sizeof(Socket));
                new_socket(&sockets[client_fd], client_fd);
            }
            else // EPOLLIN or EPOLLOUT
            {
                Socket *sock = &sockets[events[i].data.fd];
                if (events[i].events & EPOLLIN) // if (sock->action == READ_)
                {
                    debug("Enter EPOLL IN\n");
                    check((bytes = recv(sock->socket_fd, content, BUFF, 0)) < 0, "recv failed");
                    if (bytes > 0)
                    {
                        s = 0;
                        e = s;
                        check(sock->current_length < 0, "Error asking for allocation");
                        tmp = memejoin(sock->content, content, sock->current_length, bytes);
                        sock->content = tmp;
                        sock->current_length += bytes;
                        tmp = find(sock->content, END, sock->current_length, strlen(END));
                        if (!sock->method && tmp)
                        {
                            if (sock->current_length > strlen("GET /") && memcmp(sock->content, "GET /", strlen("GET /")) == 0)
                            {
                                sock->method = GET_;
                                // FILEPATH
                                s = strlen("GET /");
                                e = s;
                                while (e < sock->current_length && !isspace(sock->content[e]))
                                    e++;
                                check(s == e, "Invalid request");
                                sock->filename = calloc(e - s + 1, sizeof(char));
                                memcpy(sock->filename, sock->content + s, e - s);
                                // HTTP VERSION
                                s = e;
                                check(memcmp(sock->content + s, " HTTP/1.0\r\n", strlen(" HTTP/1.0\r\n")) &&
                                          memcmp(sock->content + s, " HTTP/1.1\r\n", strlen(" HTTP/1.1\r\n")),
                                      "Invalid request");
                                s += strlen(" HTTP/1.0\r\n");
                                // TODO: to be checked
                                // substraction may cause problem
                                // length might be useless in GET request
                                sock->current_length -= (tmp - sock->content);
                                sock->content = tmp;
                                debug("remaing:'%.*s'\n", sock->current_length, sock->content);
                            }
                            else if (sock->current_length > strlen("POST /") && memcmp(sock->content, "POST /", strlen("POST /")) == 0)
                            {
                                sock->method = POST_;
                                // FILE TYPE
                                tmp0 = find(sock->content, "Content-Type: ", sock->current_length, strlen("Content-Type: "));
                                check(!tmp0, "Content Type is required");
                                s = 0;
                                while (tmp0[s] && !isspace(tmp0[s]))
                                    s++;
                                type = calloc(s + 1, sizeof(char));
                                strncpy(type, (char *)tmp0, s);
                                debug("type: <%s>\n", type);
                                int i = 0;
                                while (extentions[i].type && strcmp(extentions[i].type, type))
                                    i++;
                                check(!extentions[i].ext, "Insupported memtype");
                                free(type);
                                // FILE LENGTH
                                tmp0 = find(sock->content, "Content-Length: ", sock->current_length, strlen("Content-Length: "));
                                check(!tmp0, "Content Length is required");
                                s = 0;
                                while (tmp0[s] && !isspace(tmp0[s]))
                                    s++;
                                len_str = calloc(s + 1, sizeof(char));
                                strncpy(len_str, (char *)tmp0, s);
                                debug("len: <%s> bytes\n", len_str);
                                char *ptr;
                                sock->Content_length = strtol(len_str, &ptr, 10);
                                check(ptr == NULL, "parsing number");
                                // FILE NAME
                                sock->filename = strjoin("post", extentions[i].ext);
                                debug("filename <%s>\n", sock->filename);
                                // POST FILE
                                tmp = find(sock->content, HEADER_END, sock->current_length, strlen(HEADER_END));
                                // debug("tmp: <%s>\n", tmp);
                                sock->content = tmp;
                                debug("sock->content: <%s>\n", sock->content);
                                sock->action = WRITE_;
                            }
                        }
                        else
                            check(1, "Invalid request");
                    }
                    else if (bytes == 0)
                    {
                        // TODO: check if all feiled are full, then change stat to EPOLLOUT
                        // TODO: to be checked
                        debug("%sfinish reading from socket%s\n", GREEN, RESET);
                    }
                    if (sock->method == GET_ && sock->filename)
                    {
                        debug("change event\n");
                    }
                    debug("Exit EPOLL IN\n");
                }
                else if (events[i].events & EPOLLOUT)
                // else if (sock->action == WRITE_) // check line 247
                {
                    debug("Enter EPOLL OUT\n");
                    if (sock->method == GET_)
                    {
                        // keep it, beacause I might send file chunk by chunk
                        if (!sock->fileptr)
                        {
                            currDir = "./";
                            tmp = (unsigned char *)strjoin(currDir, sock->filename);
                            debug("open '%s'\n", sock->filename);
                            sock->fileptr = fopen((char *)tmp, "r");
                            if (sock->fileptr) // file found
                            {
                                fseek(sock->fileptr, 0, SEEK_END);
                                sock->Content_length = ftell(sock->fileptr);
                                fseek(sock->fileptr, 0, SEEK_SET);
                                debug("file has length: %zu\n", sock->Content_length);

                                // create header
                                memset(header, 0, sizeof(header));
                                sprintf(header, "HTTP/1.0 200 OK\r\n");
                                sprintf(header + strlen(header), "Content-Type: text/html\r\n");
                                sprintf(header + strlen(header), "Content-Length: %zu\r\n\r\n", sock->Content_length);
                            }
                            else
                            {
                                // check(1, "opening file");
                                sock->fileptr = fopen("404.html", "r");
                                fseek(sock->fileptr, 0, SEEK_END);
                                sock->Content_length = ftell(sock->fileptr);
                                fseek(sock->fileptr, 0, SEEK_SET);
                                debug("file has length: %zu\n", sock->Content_length);

                                // create header
                                memset(header, 0, sizeof(header));
                                sprintf(header, "HTTP/1.0 400 NOK\r\n");
                                sprintf(header + strlen(header), "Content-Type: text/html\r\n");
                                sprintf(header + strlen(header), "Content-Length: %zu\r\n\r\n", sock->Content_length);
                            }

                            // check((sock->fileptr = open(tmp, O_RDONLY)) < 0, "opening file");
                        }
                        // header not sent
                        if (!sock->header_sent)
                        {
                            sock->header_sent = true; // TODO: check len
                            check((bytes = send(sock->socket_fd, header, strlen(header), 0)) < 0, "sending");
                            debug("send: '%s'", header);
                            debug("============ send %d bytes from header ============\n", bytes);
                            event.events = EPOLLIN | EPOLLOUT;
                            event.data.fd = client_fd;
                            check(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, events[i].data.fd, &event) < 0, "epoll_ctl");
                            debug("change event\n");
                        }
                        else
                        {
                            size_t size = sock->Content_length > BUFF ? BUFF : sock->Content_length;
                            memset(content, 0, BUFF);
                            check((bytes = fread(content, size, 1, sock->fileptr)) < 0, "reading from '%s'", sock->filename);
                            debug("did read from file '%s'\n", content);
                            check((bytes = send(sock->socket_fd, content, size, 0)) < 0, "sending");
                            sock->Content_length -= size;
                            debug("============ send %d bytes from body remaining %zu ============\n", bytes, sock->Content_length);
                            if (sock->Content_length <= 0)
                            {
                                check(sock->socket_fd != events[i].data.fd, "Error");
                                debug("close connection\n");
                                check(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) < 0, "epoll_ctl");
                                close(events[i].data.fd);
                                *sock = (Socket){0};
                            }
                        }
                    }
                    if (sock->method == POST_)
                    {
                        if (!sock->fileptr)
                        {
                            currDir = "./";
                            tmp = (unsigned char *)strjoin(currDir, sock->filename);
                            debug("open '%s'\n", sock->filename);
                            sock->fileptr = fopen((char *)tmp, "w");
                            if (sock->fileptr) // file found
                            {
                                debug("write '%s' to '%s'\n", sock->content, sock->filename);
                                fwrite(sock->content, 1, sock->Content_length, sock->fileptr);
                                fclose(sock->fileptr);
                            }
                            else
                            {
                                check(1, "opening file");
                            }
                            check(sock->socket_fd != events[i].data.fd, "Error");
                            debug("close connection\n");
                            check(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) < 0, "epoll_ctl");
                            close(events[i].data.fd);
                            *sock = (Socket){0};
                        }
                        sock->action = 0;
                    }
                    debug("Exit EPOLL OUT\n");
                }
                else
                    check(1, "Invalid event");
            }
        }
        evpos++;
    }
    free(events);
    debug("server: Closing socket...\n");
    close(server_fd);
    debug("server: Finished\n");
}