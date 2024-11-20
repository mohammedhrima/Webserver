#include "header.h"

void errput_(int line, char *fmt, ...)
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
        printf("(errno = %d) : %s\n", errno_save, strerror(errno));
        fflush(stdout);
    }
    va_end(ap);
    exit(1);
}
#define errput(args) errput_(__LINE__, args)

int set_non_blocking(int fd)
{
    int flags;
    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl");
        return -1;
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1)
    {
        perror("fcntl");
        return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    char *address = "127.1.1.0";
    int clie_fd;
    if ((clie_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        errput("creating socket");

    // Set the socket to non-blocking mode
    if (set_non_blocking(clie_fd) == -1)
        errput("set_non_blocking");

    // connect to an address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(CPORT);
    if (inet_aton(address, (struct in_addr *)&addr.sin_addr.s_addr) == 0)
        errput("inet_aton");

    // Create epoll instance
    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0)
        errput("epoll_create1 failed");

    // Add the client socket to the epoll event list
    struct epoll_event event;
    event.events = EPOLLOUT | EPOLLET; // Edge-triggered mode
    event.data.fd = clie_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clie_fd, &event) < 0)
        errput("epoll_ctl failed");

    // Connect the client socket
    if (connect(clie_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0 && errno != EINPROGRESS)
        errput("connection");

    char *request;
    FILE *file = fopen("request.txt", "r");
    if (file == NULL)
        errput("openning file");
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *end = "\r\n\r\n";
    request = calloc(size + strlen(end) + 1, sizeof(char));
    fread(request, size, sizeof(char), file);
    strcpy(request + strlen(request), end);
    fclose(file);

    printf("client send request:\n'%s'\n", request);

    // Event loop
    struct epoll_event events[10];
    int nfds;
    while (1)
    {
        nfds = epoll_wait(epoll_fd, events, 10, -1);
        if (nfds == -1)
            errput("epoll_wait failed");

        for (int i = 0; i < nfds; i++)
        {
            if (events[i].events & EPOLLOUT)
            {
                // Socket is ready for writing
                ssize_t n = send(clie_fd, request, strlen(request), 0);
                if (n < 0)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        // The socket buffer is full, wait for the next EPOLLOUT event
                        continue;
                    }
                    else
                    {
                        errput("send failed");
                    }
                }
                else
                {
                    // Request sent successfully
                    printf("client finished sending\n");
                    // Modify event to wait for reading
                    event.events = EPOLLIN | EPOLLET;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, clie_fd, &event) < 0)
                        errput("epoll_ctl failed");
                }
            }
            else if (events[i].events & EPOLLIN)
            {
                // Socket is ready for reading
                char buffer[READBYTES + 1];
                ssize_t n = recv(clie_fd, buffer, READBYTES, 0);
                if (n < 0)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        // No more data to read, wait for the next EPOLLIN event
                        continue;
                    }
                    else
                    {
                        errput("recv failed");
                    }
                }
                else if (n == 0)
                {
                    // Connection closed by the server
                    printf("Connection closed by the server\n");
                    close(clie_fd);
                    exit(EXIT_FAILURE);
                }
                else
                {
                    // Received some data
                    buffer[n] = '\0'; // Null-terminate the buffer
                    printf("Received %zd bytes: %s\n", n, buffer);
                    close(clie_fd); // Close the connection
                    exit(EXIT_SUCCESS);
                }
            }
        }
    }

    close(clie_fd);
    free(request);
    return 0;
}