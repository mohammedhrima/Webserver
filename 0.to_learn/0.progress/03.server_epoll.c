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

    printf("server: Creating socket...\n");
    int sock_fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    check(sock_fd < 0, "socket creation");

    printf("server: Binding...\n");
    check(bind(sock_fd, addr->ai_addr, addr->ai_addrlen) != 0, "bind failed");
    freeaddrinfo(addr);

    printf("server: Listening...\n");
    check(listen(sock_fd, SOMAXCONN) != 0, "listen failed");

    printf("server: create epoll instance\n");
    int epoll_fd = epoll_create1(0);
    check(epoll_fd < 0, "epoll_create1 failed");

    printf("server: adding server socket to epoll\n");
    epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = sock_fd;
    check(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &event) < 0, "epoll_ctl failed");

    printf("server: allocate MAX_EVENTS\n");
    epoll_event *events = calloc(MAX_EVENTS, sizeof(event));
    check(events == NULL, "calloc failed");

    printf("server: waiting for connection...\n");
    while (1)
    {
        // TODO: add epoll timeout
        printf("server: waiting for event\n");
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        check(num_events < 0, "epoll_wait failed");

        for (int i = 0; i < num_events; ++i)
        {
            if (events[i].data.fd == sock_fd)
            {
                // accept client connection
                sockaddr_storage client_addr;
                socklen_t clen = sizeof(client_addr);
                int client_fd = accept(sock_fd, (sockaddr *)&client_addr, &clen);
                check(client_fd < 0, "accept failed");

                // add client socket to epoll
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_fd;
                check(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0, "epoll_ctl failed");

                // address presentation
                char address[ADDRLEN];
#if 1
                inet_ntop(AF_INET, &(((struct sockaddr_in *)&client_addr)->sin_addr), address, INET_ADDRSTRLEN);
#else
                inet_ntop(AF_INET, (sockaddr_in *)&client_addr, address, INET_ADDRSTRLEN);
#endif
                printf("server: new connection from %s\n", address);
            }
            else
            {
                char reads[BUFF];
                memset(read, 0, sizeof(reads));
                int bytes;
                /*
                TODO:
                    + when recv fail:
                        + close events[i].data.fd
                        + delete it from events
                */
                check((bytes = recv(events[i].data.fd, reads, BUFF, 0)) < 0, "recv failed");
                if (bytes)
                {
                    printf("server: received (%d bytes): '%.*s'\n", bytes, bytes, reads);
                    for (int j = 0; j < num_events; ++j)
                    {
                        if (events[j].data.fd != sock_fd && events[j].data.fd != events[i].data.fd)
                        {
                            char message[1000] = {0};
                            strcpy(message, "client");
                            message[strlen(message)] = '0' + events[j].data.fd;
                            strcpy(message + strlen(message), ": ");
                            strncpy(message + strlen(message), reads, bytes);
                            send(events[j].data.fd, message, strlen(message), 0);
                        }
                    }
                }
                else
                {
                    // close connection and delete client
                    printf("server: Connection closed by client\n");
                    close(events[i].data.fd);
                    check(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) < 0, "epoll_ctl failed");
                }
            }
        }
    }
    free(events);
    printf("server: Closing socket...\n");
    close(sock_fd);
    printf("server: Finished\n");
}