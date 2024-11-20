#include "header.h"

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
    printf("%sline %d: %s", RED, line, RESET);
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
    left &&llen &&memcpy(result, left, llen);
    right &&rlen &&memcpy(result + llen, right, rlen);
    return result;
}

void *memdup(void *ptr, size_t size)
{
    void *res = calloc(size + 1, 1);
    memcpy(res, ptr, size);
    return res;
}

Socket *new_socket(Socket *new, int fd)
{
    new->action = READ_;
    new->socket_fd = fd;
    // TODO: clen = 0 may cause problem
    new->clen = 0;
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
    char address[ADDRLEN], content[BUFF], response_header[MAXLEN], *currDir, *ctype = NULL;
    unsigned char *tmp0 = NULL, *tmp1 = NULL, *header = NULL;
    // char *type, *len_str; //, *filename;
    size_t clen = 0;
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
    int fd = open("test.log", O_RDWR | O_CREAT | O_TRUNC);
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
                socklen_t slen = sizeof(client_addr);
                check((client_fd = accept(server_fd, (sockaddr *)&client_addr, &slen)) < 0, "accept failed");
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
                if (events[i].events & EPOLLIN && sock->action == READ_) // if (sock->action == READ_)
                {
                    debug("Enter EPOLL IN\n");

                    check((bytes = recv(sock->socket_fd, content, BUFF, 0)) < 0, "recv failed");
                    if (bytes > 0)
                    {
                        s = 0;
                        e = s;
                        debug("memejoin %d:<%.*s> and %d:<%.*s>\n", sock->clen, sock->clen,
                              sock->content, bytes, bytes, content);

                        tmp0 = memejoin(sock->content, content, sock->clen, bytes);
                        free(sock->content);
                        sock->content = tmp0;
                        sock->clen += bytes;
                        // Enter only if the full-header is ready
                        if (!sock->method && (header = find(sock->content, HEADER_END, sock->clen, strlen(HEADER_END))))
                        {
                            debug("%sBefore:%s <%s>\n", RED, RESET, sock->content);
                            size_t header_len = header - sock->content;
                            sock->clen -= header_len;

                            header = memdup(sock->content, header_len);            // HEADER PART
                            tmp0 = memdup(sock->content + header_len, sock->clen); // REMAINING PART
                            free(sock->content);
                            sock->content = tmp0;

                            debug("%sheader:%s <%.*s>\n", RED, RESET, header_len, header);
                            debug("%sbody:%s <%.*s>\n", RED, RESET, sock->clen, sock->content);
                            /*
                                POST / HTTP/1.1
                                Host: localhost:17000
                                Content-Type: text/plain
                                Content-Length: 22
                            */
                            if (header_len > strlen("POST /") && memcmp(header, "POST /", strlen("POST /")) == 0)
                            {
                                sock->method = POST_;
                                // FILE PATH
                                // GET /index.html
                                // FILE NAME
                                /*
                                    TODO:
                                        + test two folowed lines that have same Key (Content-Length for example)
                                */
                                tmp0 = find(header, "Content-Length: ", header_len, strlen("Content-Length: "));
                                check(tmp0 == NULL, "Content-Length is required");
                                char *ptr;
                                sock->hlen = strtol((char *)tmp0, &ptr, 10);
                                check(ptr == NULL, "parsing number");

                                tmp0 = find(header, "Content-Type: ", header_len, strlen("Content-Type: "));
                                check(tmp0 == NULL, "Content-Type is required");
                                int i = 0;
                                while (!isspace(tmp0[i]))
                                    i++;
                                ctype = memdup(tmp0, i);
                                i = 0;
                                while (extentions[i].type && strcmp(extentions[i].type, ctype))
                                    i++;
                                sock->filename = strjoin("post", extentions[i].ext);

                                tmp0 = find(header, "Transfer-Encoding: ", header_len, strlen("Transfer-Encoding: "));
                                if (tmp0)
                                {
                                    i = 0;
                                    while (!isspace(tmp0[i]))
                                        i++;
                                    if (strncmp((char *)tmp0, "chunked", i) == 0) // TODO: it may segvault if i > strlen("chunked")
                                    {
                                        sock->transfer = chunked_;
                                        check(1, "found 'chunked'\n");
                                    }
                                    else
                                        check(1, "Invalid transfer encoding");
                                }
                                debug("content-length: <%zu>\n", sock->hlen);
                                debug("content-type:   <%s>\n", ctype);
                                debug("remaining len:  <%zu>\n", sock->clen);
                                debug("filename:       <%s>\n", sock->filename);

                                sock->action = WRITE_;
                            }
                            else if (header_len > strlen("GET /") && memcmp(header, "GET /", strlen("GET /")) == 0)
                            {
                                check(1, "handle this case");
                            }
                            else
                                check(1, "Invalid request");
                            free(header);
                            // exit(1);
                        }
                        if (sock->method)
                        {
                            if (sock->transfer == chunked_)
                            {
                                debug("content: <%s>\n", sock->content);
                                tmp0 = find(sock->content, END, sock->clen, strlen(END));
                                tmp1 = tmp0 ? memdup(sock->content, tmp0 - sock->content - strlen(END)) : NULL;
                                debug("tmp0: <%s>\n", tmp0);
                                debug("tmp1: <%s>\n", tmp1);
                            }
                            else
                                sock->action = WRITE_;
                        }

#if 0
                        if (0 && !sock->method && tmp0)
                        {
                            if (sock->clen > strlen("POST /") && memcmp(sock->content, "POST /", strlen("POST /")) == 0)
                            {
                                sock->method = POST_;
                                // FILE TYPE
                                tmp0 = find(sock->content, "Content-Type: ", sock->clen, strlen("Content-Type: "));
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
                                tmp0 = find(sock->content, "Content-Length: ", sock->clen, strlen("Content-Length: "));
                                check(!tmp0, "Content Length is required");

                                s = 0;
                                while (tmp0[s] && !isspace(tmp0[s]))
                                    s++;
                                len_str = calloc(s + 1, sizeof(char));
                                strncpy(len_str, (char *)tmp0, s);
                                debug("len: <%s> bytes\n", len_str);
                                char *ptr;
                                sock->hlen = strtol(len_str, &ptr, 10);
                                check(ptr == NULL, "parsing number");
                                // FILE NAME
                                sock->filename = strjoin("post", extentions[i].ext);
                                debug("filename <%s>\n", sock->filename);
                                // POST FILE
                                tmp0 = find(sock->content, HEADER_END, sock->clen, strlen(HEADER_END));
                                sock->clen = sock->clen - (tmp0 - sock->content);
                                debug("clen is : %zu\n", sock->clen);
                                sock->content = tmp0;
                                // debug("sock->content: <%s>\n", sock->content);
                                sock->action = WRITE_;
                                // exit(1);
                            }
                            else if (sock->clen > strlen("GET /") && memcmp(sock->content, "GET /", strlen("GET /")) == 0)
                            {
                                sock->method = GET_;
                                // FILEPATH
                                s = strlen("GET /");
                                e = s;
                                while (e < sock->clen && !isspace(sock->content[e]))
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
                                /*
                                TODOS:
                                    + to be checked
                                    + substraction may cause problem
                                    + length might be useless in GET request
                                */
                                sock->clen -= (tmp0 - sock->content);
                                sock->content = tmp0;
                                debug("remaing:'%.*s'\n", sock->clen, sock->content);
                            }
                        }
                        else
                        {
                            sock->action = WRITE_;
                            debug("%s==============================================================%s\n", RED, RESET);
                            // check(1, "Invalid request");
                        }
#endif
                    }
                    else if (bytes == 0)
                    {
                        // TODOS: check if all feiled are full, then change stat to EPOLLOUT
                        debug("%sfinish reading from socket%s\n", GREEN, RESET);
                    }
                    if (sock->method == GET_ && sock->filename)
                    {
                        debug("change event\n");
                    }
                    debug("Exit EPOLL IN\n");
                }
                else if (events[i].events & EPOLLOUT && !sock->action)
                {
                    debug("Enter EPOLL OUT Without SOCKET\n");
                    debug("Exit EPOLL OUT Without SOCKET\n");
                }
                else if (events[i].events & EPOLLOUT && sock->action == WRITE_) // if (sock->action == WRITE_)
                {
                    debug("Enter EPOLL OUT\n");

                    if (sock->method == POST_)
                    {
                        if (!sock->fileptr)
                        {
                            currDir = "./";
                            tmp0 = (unsigned char *)strjoin(currDir, sock->filename);
                            debug("open '%s'\n", sock->filename);
                            sock->fileptr = fopen((char *)tmp0, "w");
                            check(!sock->fileptr, "opening file");
                        }

                        // size_t bytes = sock->clen > BUFF ? BUFF : sock->clen;
                        bytes = sock->clen;
                        if (sock->clen > BUFF)
                        {
                            // do something
                        }
                        // bytes = strlen((char*)sock->content);
                        debug("I have '%zu'\n", strlen((char *)sock->content));
                        debug("write '%s' (%zu bytes) to '%s'\n", sock->content, bytes, sock->filename);
                        fwrite(sock->content, 1, bytes, sock->fileptr);
                        // exit(1);
                        free(sock->content);
                        sock->content = NULL;
                        sock->clen -= bytes;
                        sock->wlen += bytes;

                        if (sock->wlen >= sock->hlen && sock->transfer != chunked_)
                        {
                            check(sock->socket_fd != events[i].data.fd, "Error");
                            debug("close connection\n");
                            check(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) < 0, "epoll_ctl");
                            close(events[i].data.fd);
                            fclose(sock->fileptr);
                            *sock = (Socket){0};
                        }
                        if (sock->clen <= 0)
                            sock->action = READ_;
                        // exit(1);
                    }
                    else if (sock->method == GET_)
                    {
                        // keep it, beacause I might send file chunk by chunk
                        if (!sock->fileptr)
                        {
                            currDir = "./";
                            tmp0 = (unsigned char *)strjoin(currDir, sock->filename);
                            debug("open '%s'\n", sock->filename);
                            sock->fileptr = fopen((char *)tmp0, "r");
                            if (sock->fileptr) // file found
                            {
                                fseek(sock->fileptr, 0, SEEK_END);
                                sock->clen = ftell(sock->fileptr);
                                fseek(sock->fileptr, 0, SEEK_SET);
                                debug("file has length: %zu\n", sock->clen);

                                // create header
                                memset(response_header, 0, sizeof(response_header));
                                sprintf(response_header, "HTTP/1.0 200 OK\r\n");
                                sprintf(response_header + strlen(response_header), "Content-Type: text/html\r\n");
                                sprintf(response_header + strlen(response_header), "Content-Length: %zu\r\n\r\n", sock->clen);
                            }
                            else
                            {
                                sock->fileptr = fopen("404.html", "r");
                                fseek(sock->fileptr, 0, SEEK_END);
                                sock->clen = ftell(sock->fileptr);
                                fseek(sock->fileptr, 0, SEEK_SET);
                                debug("file has length: %zu\n", sock->clen);

                                // create response_header
                                memset(response_header, 0, sizeof(response_header));
                                sprintf(response_header, "HTTP/1.0 400 NOK\r\n");
                                sprintf(response_header + strlen(response_header), "Content-Type: text/html\r\n");
                                sprintf(response_header + strlen(response_header), "Content-Length: %zu\r\n\r\n", sock->clen);
                            }
                        }
                        // response_header not sent
                        if (!sock->header_sent)
                        {
                            sock->header_sent = true; // TODO: check len
                            check((bytes = send(sock->socket_fd, response_header, strlen(response_header), 0)) < 0, "sending");
                            debug("send: '%s'", response_header);
                            debug("============ send %d bytes from header ============\n", bytes);
                            event.events = EPOLLIN | EPOLLOUT;
                            event.data.fd = client_fd;
                            check(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, events[i].data.fd, &event) < 0, "epoll_ctl");
                            debug("change event\n");
                        }
                        else
                        {
                            size_t size = sock->clen > BUFF ? BUFF : sock->clen;
                            memset(content, 0, BUFF);
                            check((bytes = fread(content, size, 1, sock->fileptr)) < 0, "reading from '%s'", sock->filename);
                            debug("did read from file '%s'\n", content);
                            check((bytes = send(sock->socket_fd, content, size, 0)) < 0, "sending");
                            sock->clen -= size;
                            debug("============ send %d bytes from body remaining %zu ============\n", bytes, sock->clen);
                            if (sock->clen <= 0)
                            {
                                check(sock->socket_fd != events[i].data.fd, "Error");
                                debug("close connection\n");
                                check(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) < 0, "epoll_ctl");
                                close(events[i].data.fd);
                                *sock = (Socket){0};
                            }
                        }
                    }
                    debug("Exit EPOLL OUT\n");
                    // exit(1);
                }
                // else
                //     check(1, "Invalid event");
            }
        }
        evpos++;
    }
    free(events);
    debug("server: Closing socket...\n");
    close(server_fd);
    debug("server: Finished\n");
}