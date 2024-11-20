#include "header.h"

void errput_(int line, int cond, char *fmt, ...)
{
    if (cond)
    {
        int errno_save;
        va_list ap;
        errno_save = errno;

        va_start(ap, fmt);
        printf("%d: Error: ", line);
        vfprintf(stdout, fmt, ap);
        fflush(stdout);
        if (errno_save != 0)
        {
            printf(" (errno = %d) : %s\n", errno_save, strerror(errno));
            fflush(stdout);
        }
        va_end(ap);
        exit(1);
    }
}

// TODO: to be checked
void set_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    check(flags == -1, "fcntl");
    flags |= O_NONBLOCK;
    check(fcntl(fd, F_SETFL, flags) < 0, "fcntl");
}

int main()
{
    char *ip_addr = "127.1.1.0";
    int client = socket(AF_INET, SOCK_STREAM, 0);
    check(client < 0, "socket creation");
    // set socket to non-blocking mode
    set_non_blocking(client);

    // connect to an address
    sockaddr_in addr = (sockaddr_in){0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(CPORT);
    check(inet_aton(ip_addr, (in_addr *)&addr.sin_addr.s_addr) == 0, "inet_aton");

    // create epoll instance
    int epoll_fd = epoll_create1(0);
    check(epoll_fd < 0, "epoll_create1");

    epoll_event event = (epoll_event){0};
    event.events = EPOLLOUT | EPOLLET; // edge-triggered mode
    event.data.fd = client;
    check(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client, &event) < 0, "epoll_ctl");
    // TODO: why errno != EINPROGRESS ?
    check(connect(client, (sockaddr *)&addr, sizeof(addr)) < 0 && errno != EINPROGRESS, "connection");

    FILE *file = fopen("request.txt", "r");
    check(file == NULL, "openning file");
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *end = "\r\n\r\n";
    char *request = calloc(size + strlen(end) + 1, sizeof(char));
    fread(request, size, sizeof(char), file);
    strcpy(request + strlen(request), end);
    fclose(file);

    printf("client send '%s'\n", request);

    epoll_event events[10];
    int fds;
    while (1)
    {
        // TODO: seach about epoll_wait
        check((fds = epoll_wait(epoll_fd, events, sizeof(events) / sizeof(*events), -1)) < 0, "epoll_wait failed");
        for (int i = 0; i < fds; i++)
        {
            if (events[i].events & EPOLLOUT)
            {
                // socket is ready for writing
                size_t bytes = send(client, request, strlen(request), 0);
                if (bytes < 0)
                {
                    // socket buffer is full
                    // wait till next epllout event
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                        continue;
                    else
                        printf("send failed\n");
                }
                else
                {
                    // request send succefully
                    printf("client: finished sending\n");
                    // sent event to wait for reading
                    event.events = EPOLLIN | EPOLLET;
                    check(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client, &event) < 0, "epoll_ctl failed");
                }
            }
            else if (events[i].events & EPOLLIN)
            {
                // socket is ready to be read
                char buff[READBYTES];
                memset(buff, 0, sizeof(buff));
                size_t bytes = recv(client, buff, READBYTES, 0);
                if (bytes < 0)
                {
                    // No more data to read
                    // wait till next EPOLLIN event
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                        continue;
                    else
                        printf("recv failed");
                }
                else if (bytes == 0)
                {
                    // Connectin closed by the server
                    printf("Connectin closed by the server\n");
                    close(client);
                }
                else
                {
                    // receive data
                    printf("receive %zu bytes: '%s'\n", bytes, buff);
                    close(client);
                }
            }
        }
    }
    close(client);
    free(request);
}
