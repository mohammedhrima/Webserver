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

int main()
{
    printf("server: Configuring local address...\n");
    struct addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *addr;
    check(getaddrinfo(NULL, SPORT, &hints, &addr) != 0, "getaddrinfo failed");

    // create socket
    printf("server: Creating socket...\n");
    int sock_fd;
    check((sock_fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) < 0, "socket creation");

    // bind socket to address and port
    printf("server: Binding...\n");
    check(bind(sock_fd, addr->ai_addr, addr->ai_addrlen) != 0, "bind failed");
    freeaddrinfo(addr);

    // listen for connectons
    printf("server: Listening...\n");
    check(listen(sock_fd, SOMAXCONN) != 0, "listen failed");

    // create epoll instance
    int epoll_fd = epoll_create1(0);
    check(epoll_fd < 0, "epoll_create1 failed");

    // add server socket to epoll
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = sock_fd;
    check(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &event) < 0, "epoll_ctl failed");


    struct epoll_event *events = calloc(MAX_EVENTS, sizeof(event));
    check(events == NULL, "calloc failed");

    printf("server: Waiting for connections...\n");
    while (1)
    {
        // wait for an event
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        check(num_events < 0, "epoll_wait failed");

        for (int i = 0; i < num_events; ++i)
        {
            if (events[i].data.fd == sock_fd)
            {
                // accept client connection
                struct sockaddr_storage client_addr;
                socklen_t clen = sizeof(client_addr);
                int client_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &clen);
                check(client_fd < 0, "accept failed");

                // add client socket to epoll
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_fd;
                check(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0, "epoll_ctl failed");

                // internet address to presentation
                char addr_buff[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(((struct sockaddr_in *)&client_addr)->sin_addr), addr_buff, INET_ADDRSTRLEN);
                printf("server: new connection from %s\n", addr_buff);
            }
            else
            {

                char read[BUFF];
                memset(read, 0, sizeof(read));
                int bytes = recv(events[i].data.fd, read, BUFF, 0);
                if (bytes <= 0)
                {
                    if (bytes == 0)
                        printf("server: Connection closed by client\n");
                    else
                        perror("recv");

                    close(events[i].data.fd);
                    // delete client
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                }
                else
                {
                    // this because I have a chat_server
                    printf("server: received (%d bytes): '%.*s'\n", bytes, bytes, read);
                    for (int j = 0; j < num_events; ++j)
                    {
                        if (events[j].data.fd != sock_fd && events[j].data.fd != events[i].data.fd)
                        {
                            char message[1000] = {0};
                            strcpy(message, "client");
                            message[strlen(message)] = '0' + events[j].data.fd;
                            strcpy(message + strlen(message), ": ");
                            strncpy(message + strlen(message), read, bytes);
                            send(events[j].data.fd, message, strlen(message), 0);
                        }
                    }
                }
            }
        }
    }

    free(events);
    printf("server: Closing socket...\n");
    close(sock_fd);
    printf("server: Finished\n");

    return 0;
}
