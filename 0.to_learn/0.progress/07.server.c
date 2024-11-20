#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

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
#include <poll.h>

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
#define EPOLL_TIMEOUT 10000
#define CLIENT_TIMEOUT 5

#define END "\r\n"
#define HEADER_END "\r\n\r\n"

typedef struct addrinfo addrinfo;
typedef struct sockaddr_storage sockaddr_storage;
typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct timeval timeval;
typedef struct epoll_event epoll_event;
typedef struct in_addr in_addr;
typedef struct Socket Socket;
typedef struct Server Server;
typedef enum Method Method;
typedef enum Action Action;
typedef enum Transfer Transfer;
typedef struct timeval timeval;
typedef struct tm tm;

#define check(cond, ...) errput_(__LINE__, (cond), __VA_ARGS__)
#define debug(...) debug_(__LINE__, __VA_ARGS__)
#define allocate(size) allocate_(__LINE__, size)
#define openfile(filename, access_) openfile_(__LINE__, filename, access_)
#define readfile(fd, buff, size) readfile_(__LINE__, fd, buff, size)

#if 1
void errput_(int line, int cond, char *fmt, ...);
void debug_(int line, char *fmt, ...);
int is_regular_file(const char *path);
void *find(void *ptr, void *to_find, size_t ptr_len, size_t to_find_len);
char *strjoin(char *string1, char *string2);
void *memejoin(void *left, void *right, size_t llen, size_t rlen);
void *memdup(void *ptr, size_t size);
Socket *new_socket(Socket *new, int fd);
void set_non_blocking(int fd);

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
#endif

enum Method
{
    GET_ = 11,
    POST_,
    DELETE_,
};

enum Action
{
    // SETUP_ = 22,
    READ_ = 22,
    WRITE_
};

enum Transfer
{
    CHUNKED_ = 33,
    BOUNDARY_
};

struct Socket
{
    /*
        TODOS:
            + you might have file with 0 length but
            you have to create it
    */

    int socket_fd;
    unsigned char *content;

    Method method;
    Action action;
    Transfer transfer;

    char filename[50];
    int fd;
    bool header_sent;

    size_t full_len;
    size_t curr_len;

    time_t last_activity;
};

// UTILS
void errput_(int line, int cond, char *fmt, ...);
void debug_(int line, char *fmt, ...);
int is_regular_file(const char *path);
void *find(void *ptr, void *to_find, size_t ptr_len, size_t to_find_len);
char *strjoin(char *string1, char *string2);
void *memejoin(void *left, void *right, size_t llen, size_t rlen);
void *memdup(void *ptr, size_t size);
Socket *new_socket(Socket *new, int fd);
void set_non_blocking(int fd);
void *allocate_(int line, size_t size);
int openfile_(int line, char *filename, int access);
int readfile_(int line, int fd, char *buff, int size);

int main(void)
{
    int server_fd, client_fd, epoll_fd, evpos = 0, bytes, events_size, s, e, opt = 1;
    char address[ADDRLEN], content[BUFF], res_header[MAXLEN], *currDir = NULL, *ctype = NULL, *ptr = NULL;
    unsigned char *tmp = NULL, *chunk_size = NULL, *header = NULL;

    addrinfo hints = (addrinfo){
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_PASSIVE,
    };
    addrinfo *addr;
    check(getaddrinfo(NULL, SPORT, &hints, &addr) != 0, "gettaddrinfo");
    debug("server: Creating socket...\n");
    check((server_fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) < 0, "socket");
    debug("server: Setting socket...\n");
    check(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(int)) == -1, "setsockopt");
    debug("server: Binding...\n");
    check(bind(server_fd, addr->ai_addr, addr->ai_addrlen) != 0, "bind");
    freeaddrinfo(addr);
    debug("server: Listening...\n");
    check(listen(server_fd, SOMAXCONN) != 0, "listen");
    debug("server: create epoll instance\n");
    check((epoll_fd = epoll_create1(0)) < 0, "epoll_create1");
    debug("server: Adding server socket to epoll\n");
    epoll_event event = (epoll_event){
        .events = EPOLLIN | EPOLLOUT | EPOLLERR,
        .data.fd = server_fd,
    };
    // TODO: add EPOLLERROR, EPOLLHANGUP
    check(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0, "epoll_ctl");
    epoll_event events[MAX_EVENTS];
    size_t sockets_len = MAX_EVENTS;
    Socket *sockets = calloc(sockets_len + 1, sizeof(Socket));
    check(sockets == NULL, "calloc failed");
    new_socket(&sockets[server_fd], server_fd);
    debug("server: listening on PORT(%s)...\n", SPORT);
    // TODO: check EWOULDBLOCK
    while (1)
    {
        check((events_size = epoll_wait(epoll_fd, events, MAX_EVENTS, EPOLL_TIMEOUT)) < 0, "epoll_wait");
        for (int i = 0; i < events_size; i++)
        {
            if (events[i].data.fd == server_fd) // new connection
            {
                // accept client connection
                sockaddr_storage client_addr;
                socklen_t client_socket_len = sizeof(client_addr);
                // TODO: test one client from two servers
                check((client_fd = accept(server_fd, (sockaddr *)&client_addr, &client_socket_len)) < 0, "accept");
                set_non_blocking(client_fd);
                // add client socket to epoll
                event.events = EPOLLIN | EPOLLOUT;
                event.data.fd = client_fd;
                check(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0, "epoll_ctl");
                // TODO: remove inettop after
                inet_ntop(AF_INET, (sockaddr_in *)&client_addr, address, INET_ADDRSTRLEN);
                debug("server: new connection from fd:(%d), address:(%s)\n", client_fd, address);
                if (client_fd > sockets_len)
                    sockets = realloc(sockets, (sockets_len *= 2) * sizeof(Socket));
                sockets[client_fd].last_activity = time(NULL);
                new_socket(&sockets[client_fd], client_fd);
            }
            else // EPOLLIN or EPOLLOUT
            {
                Socket *sock = &sockets[events[i].data.fd];
                if (events[i].events & EPOLLIN && sock->action == READ_)
                {
                    debug("ENTER EPOLLIN\n");
                    if (sock->transfer == CHUNKED_ && sock->full_len && sock->full_len < BUFF)
                        check((bytes = recv(sock->socket_fd, content, sock->full_len, 0)) < 0, "recv");
                    else
                        check((bytes = recv(sock->socket_fd, content, BUFF, 0)) < 0, "recv");
                    if (bytes > 0)
                    {
                        s = 0;
                        e = s;
                        debug("join (%d bytes):(%.*s) and (%d bytes):(%.*s)\n",
                              sock->curr_len, sock->curr_len, sock->content, bytes, bytes, content);
                        tmp = memejoin(sock->content, content, sock->curr_len, bytes);
                        free(sock->content);
                        sock->content = tmp;
                        sock->curr_len += bytes;
                        debug("%d:(%s)\n", sock->curr_len, sock->content);
                        // ENTER IF HEADER IS READY
                        if (!sock->method && (tmp = find(sock->content, HEADER_END, sock->curr_len, strlen(HEADER_END))))
                        {
                            size_t header_len = tmp - sock->content;
                            sock->curr_len = sock->curr_len - header_len - strlen(HEADER_END);

                            header = memdup(sock->content, header_len);                                    // HEADER PART
                            tmp = memdup(sock->content + header_len + strlen(HEADER_END), sock->curr_len); // REMAINING PART
                            free(sock->content);
                            sock->content = tmp;
                            debug("%sHEADER:%s (%.*s)\n", RED, RESET, header_len, header);
                            debug("%sBODY:%s (%.*s)\n", RED, RESET, sock->curr_len, sock->content);
                            /*
                                POST / HTTP/1.1
                                Host: localhost:17000
                                Content-Type: text/plain
                                Content-Length: 22
                            */
                            if (header_len > strlen("GET /") && memcmp(header, "GET /", strlen("GET /")) == 0)
                            {
                                sock->method = GET_;
                                tmp = header + strlen("GET /");
                                s = 0;
                                e = 0;
                                // TODO: check if no filename : GET / HTTP/1.0
                                while (!isspace(tmp[e]))
                                    e++;
                                // filename
                                if (e > s)
                                {
                                    char *filename = memdup(tmp + s, e - s);
                                    strcpy(sock->filename, filename);
                                }
                                else
                                    strcpy(sock->filename, "response/index.html"); // TODO: check config file
                                e++;

                                if (strncmp((char *)tmp + e, "HTTP/1.1\r\n", strlen("HTTP/1.1\r\n")) &&
                                    strncmp((char *)tmp + e, "HTTP/1.0\r\n", strlen("HTTP/1.0\r\n")))
                                    check(1, "Invalid HTTP version");
                            }
                            else if (header_len > strlen("POST /") && memcmp(header, "POST /", strlen("POST /")) == 0)
                            {
                                sock->method = POST_;
                                tmp = header + strlen("POST /");
                                s = 0;
                                while (!isspace(tmp[s]))
                                    s++;
                                // filename
                                tmp = tmp + s;
                                s = 0;
                                while (isspace(tmp[s]))
                                    s++;
                                tmp = tmp + s;
                                // debug("(%s)\n", header);
                                debug("(%s)\n", tmp);
                                if (strncmp((char *)tmp, "HTTP/1.1\r\n", strlen("HTTP/1.1\r\n")) &&
                                    strncmp((char *)tmp, "HTTP/1.0\r\n", strlen("HTTP/1.0\r\n")))
                                    check(1, "Invalid HTTP version");
                                tmp = find(header, "Content-Type: ", header_len, strlen("Content-Type: "));
                                check(tmp == NULL, "Invalid request (Content-type required)");
                                s = 0;
                                tmp += strlen("Content-Type: ");
                                debug("tmp: (%s)\n", tmp);
                                while (tmp[s] && !isspace(tmp[s]))
                                    s++;
                                int len = s;
                                while (extentions[s].type &&
                                       (len != strlen(extentions[s].type) ||
                                        strncmp((char *)tmp, extentions[s].type, strlen(extentions[s].type))))
                                    s++;
                                // debug("s: %zu\n", s);
                                if (!extentions[s].type)
                                    check(1, "Invalid memetype");

                                timeval current_time;
                                tm *time_info;
                                gettimeofday(&current_time, NULL);
                                time_info = localtime(&current_time.tv_sec);
                                sprintf(sock->filename, "upload/");
                                strftime(sock->filename + strlen("upload/"), sizeof(sock->filename) - strlen("upload/"), "%y-%m-%d_%H:%M:%S", time_info);
                                sprintf(sock->filename + strlen(sock->filename), ".%03ld", current_time.tv_usec / 1000);
                                sprintf(sock->filename + strlen(sock->filename), "%s", extentions[s].ext);
                                debug("(%s)\n", sock->filename);
                                tmp = find(header, "Content-Length: ", header_len, strlen("Content-Length: "));
                                if (tmp)
                                {
                                    tmp += strlen("Content-Length: ");
                                    sock->full_len = strtol((char *)tmp, &ptr, 10);
                                    check(ptr == NULL, "parsing number");
                                    debug("has length: %zu\n", sock->full_len);
                                }
                                // TODO: test repeated key (Transfer-Encoding multiple times in header)
                                tmp = find(header, "Transfer-Encoding: ", header_len, strlen("Transfer-Encoding: "));
                                if (tmp)
                                {
                                    tmp += strlen("Transfer-Encoding: ");
                                    if (strncmp((char *)tmp, "chunked\r\n", strlen("chunked\r\n")) == 0)
                                    {
                                        debug("curr length: %zu\n", sock->curr_len);
                                        debug("content    : (%s)\n", sock->content);
                                        sock->transfer = CHUNKED_;
                                        tmp = find(sock->content, END, sock->curr_len, strlen(END));
                                        check(tmp == NULL, "next END not found");
                                        char *tmp0 = (char *)memdup(sock->content, tmp - sock->content);
                                        sock->full_len = strtol(tmp0, &ptr, 16);
                                        check(ptr == NULL, "parsing number");
                                        free(tmp0);
                                        tmp += strlen(END);

                                        sock->curr_len = sock->curr_len - (tmp - sock->content);
                                        debug("curr length: %zu\n", sock->curr_len);
                                        sock->content = memdup(tmp, sock->curr_len);
                                        debug("content    : (%s)\n", sock->content);
                                        if (sock->full_len == 0)
                                        {
                                            debug("close connection\n");
                                            check(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) < 0, "epoll_ctl");
                                            close(events[i].data.fd);
                                            close(sock->fd);
                                            *sock = (Socket){0};
                                        }
                                        debug("(%zu)\n", sock->full_len);
                                    }
                                    else if (strncmp((char *)tmp, "b\r\n", strlen("b\r\n")) == 0)
                                    {
                                        // debug("curr length: %zu\n", sock->curr_len);
                                        // debug("content    : (%s)\n", sock->content);
                                        // sock->transfer = BOUNDARY_;
                                    }
                                    else
                                        check(1, "Invalid Transfer-encoding");
                                    debug("(%s)\n", sock->content);
                                }
                            }
                            else
                                check(1, "Invalid header");
                            free(header);
                        }
                        sock->last_activity = time(NULL);
                    }
                    if (sock->method == POST_ && sock->transfer == CHUNKED_ && sock->full_len == 0)
                    {
                        debug("content    : (%s)\n", sock->content);
                        sock->curr_len -= strlen(END);
                        tmp = memdup(sock->content + strlen(END), sock->curr_len);
                        free(sock->content);
                        sock->content = tmp;
                        tmp = find(sock->content, END, sock->curr_len, strlen(END));
                        if (tmp == NULL)
                        {
                            printf("Error");
                            exit(1);
                        }
                        char *tmp0 = (char *)memdup(sock->content, tmp - sock->content);
                        if (tmp0 == NULL)
                            sock->full_len = 0;
                        else
                            sock->full_len = strtol(tmp0, &ptr, 16);
                        free(tmp0);
                        check(ptr == NULL, "parsing number");
                        tmp += strlen(END);

                        sock->curr_len = sock->curr_len - (tmp - sock->content);
                        debug("curr length: %zu\n", sock->curr_len);
                        sock->content = memdup(tmp, sock->curr_len);
                        debug("content    : (%s)\n", sock->content);
                        if (sock->full_len == 0)
                        {
                            debug("close connection\n");
                            check(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) < 0, "epoll_ctl");
                            close(events[i].data.fd);
                            close(sock->fd);
                            *sock = (Socket){0};
                        }
                        debug("(%zu)\n", sock->full_len);
                        // exit(1);
                    }
                    if (sock->method)
                    {
                        sock->action = WRITE_;
                    }
                    debug("EXIT EPOLL IN\n");
                }
                else if (events[i].events & EPOLLOUT && sock->action == WRITE_)
                {
                    debug("ENTER EPOLLOUT\n");
                    if (sock->method == GET_)
                    {
                        if (sock->fd == 0)
                        {
                            currDir = "./"; // TODO: get it from config file
                            tmp = memejoin(currDir, sock->filename, strlen(currDir), strlen(sock->filename));
                            debug("open (%s)\n", sock->filename);
                            sock->fd = open((char *)tmp, O_RDONLY);
                            free(tmp);
                            if (sock->fd > 0)
                            {
                                // create header
                                memset(res_header, 0, sizeof(res_header));
                                sprintf(res_header, "HTTP/1.0 200 OK\r\n");
                                // TODO: content-type should depends on file extention
                                sprintf(res_header + strlen(res_header), "Content-Type: text/html\r\n\r\n");
                            }
                            else
                            {
                                sock->fd = open("errors/404.html", O_RDONLY);
                                // create header
                                memset(res_header, 0, sizeof(res_header));
                                sprintf(res_header, "HTTP/1.0 400 Not Found\r\n");
                                sprintf(res_header + strlen(res_header), "Content-Type: text/html\r\n");
                                sprintf(res_header + strlen(res_header), "Connection: close\r\n");
                            }
                        }
                        // header not sent
                        if (!sock->header_sent)
                        {
                            sock->header_sent = true;
                            check((bytes = send(sock->socket_fd, res_header, strlen(res_header), 0)) < 0, "send");
                            debug("send %d bytes: (%s)\n", bytes, res_header);
                            // TODO: see if you can remove it
                            sock->last_activity = time(NULL);
                        }
                        else
                        {
                            size_t size = (sock->curr_len > BUFF || sock->curr_len == 0) ? BUFF : sock->curr_len;
                            memset(content, 0, BUFF);
                            check((bytes = read(sock->fd, content, size)) < 0, "read");
                            debug("read %d bytes: (%.*s)\n", bytes, bytes, content);

                            if (bytes > 0)
                            {
                                check((bytes = send(sock->socket_fd, content, bytes, 0)) < 0, "sending");
                                debug("send %d bytes\n", bytes);
                                sock->last_activity = time(NULL);
                            }
                            else
                            {
                                debug("close connection\n");
                                check(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) < 0, "epoll_ctl");
                                close(events[i].data.fd);
                                close(sock->fd);
                                *sock = (Socket){0};
                            }
                        }
                    }
                    else if (sock->method == POST_ && sock->curr_len)
                    {
                        check(sock->curr_len < 0, "Internal error");
                        if (sock->fd == 0)
                        {
                            sock->fd = open(sock->filename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
                            if (sock->fd > 0)
                            {
                                // Do something
                            }
                            else
                            {
                                // Do something
                            }
                        }
                        size_t size = sock->curr_len > BUFF ? BUFF : sock->curr_len;
                        check((bytes = write(sock->fd, sock->content, size)) < 0, "write");
                        tmp = memdup(sock->content + bytes, sock->curr_len - bytes);
                        debug("write %d bytes: (%.*s)\n", bytes, bytes, sock->content);
                        free(sock->content);
                        sock->content = tmp;
                        sock->curr_len -= bytes;
                        sock->full_len -= bytes;

                        check(sock->curr_len < 0, "Internal error");
                        if (sock->curr_len == 0)
                            sock->action = READ_;
                        sock->last_activity = time(NULL);
                        if (sock->full_len == 0 && sock->transfer != CHUNKED_)
                        {
                            debug("close connection\n");
                            check(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) < 0, "epoll_ctl");
                            close(events[i].data.fd);
                            close(sock->fd);
                            *sock = (Socket){0};
                        }
                    }
                    debug("EXIT EPOLL OUT (%zu):(%s)\n", sock->curr_len, sock->action == READ_ ? "READ" : "WRITE");
                }
                if (sock->last_activity && (time(NULL) - sock->last_activity) >= CLIENT_TIMEOUT)
                {
                    printf("Connection %d timed out (%ds)\n", sock->socket_fd, CLIENT_TIMEOUT);
                    check(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) < 0, "epoll_ctl");
                    close(events[i].data.fd);
                    *sock = (Socket){0};
                }
            }
        }
    }
    debug("server: Closing socket...\n");
    close(server_fd);
    debug("server: Finished\n");
}

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
            printf(" (errno = %d) : %s, ", errno_save, strerror(errno));
            fflush(stdout);
        }
        printf("%s\n", RESET);
        va_end(ap);
        exit(1);
    }
}

void debug_(int line, char *fmt, ...)
{
#if 1
    va_list ap;

    va_start(ap, fmt);
    printf("%sline %d: %s", RED, line, RESET);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
#endif
}

int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void *find(void *ptr, void *to_find, size_t ptr_len, size_t to_find_len)
{
    if (ptr == NULL || to_find == NULL ||
        ptr_len == 0 || to_find_len == 0 ||
        to_find_len > ptr_len)
        return NULL;

    unsigned char *left = (unsigned char *)ptr;
    unsigned char *right = (unsigned char *)to_find;

    for (size_t i = 0; i <= ptr_len - to_find_len; ++i)
    {
        size_t j = 0;
        while (j < to_find_len && left[i + j] == right[j])
            j++;
        if (j == to_find_len)
            return left + i;
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
    if (!llen && !rlen)
    {
        debug("%smemjoin return NULL%s\n", RED, RESET);
        return NULL;
    }
    unsigned char *result = calloc(llen + rlen + 1, 1);
    left &&llen &&memcpy(result, left, llen);
    right &&rlen &&memcpy(result + llen, right, rlen);
    return result;
}

void *memdup(void *ptr, size_t size)
{
    if (size == 0)
    {
        debug("%smemdup return NULL%s\n", RED, RESET);
        return NULL;
    }
    void *res = calloc(size + 1, 1);
    memcpy(res, ptr, size);
    return res;
}

Socket *new_socket(Socket *new, int fd)
{
    new->action = READ_;
    new->socket_fd = fd;
    return new;
}

void set_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    check(flags == -1, "fcntl");
    flags |= O_NONBLOCK;
    check(fcntl(fd, F_SETFL, flags) < 0, "fcntl");
}
