#include "header.h"

typedef struct sockaddr saddr;
typedef struct sockaddr_in saddr_in;

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
#define check(cond, ...) errput_(__LINE__, (cond), __VA_ARGS__)

void debug_(int line, char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    printf("line %d: ", line);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
}
#define debug(...) debug_(__LINE__, __VA_ARGS__)

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

int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

int main()
{
    saddr_in addr = (saddr_in){0};
    int server_fd;
    int client_fd;
    char *header_end = "\r\n\r\n";
    char *end = "\r\n";

    // TCP stream socket
    check((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0, "creating socket");
    // address
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);
    int opt = 1;
    check(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(int)) == -1, "seting socket");
    check(bind(server_fd, (saddr *)&addr, sizeof(addr)) < 0, "binding");
    check(listen(server_fd, 10) < 0, "listening");
    while (1)
    {
        saddr_in addr;
        socklen_t addr_len;
        char address[ADDRLEN];
        memset(address, 0, sizeof(address));
        debug("server listening on port %d...\n", PORT);
#if 0
        // (sa*)NULL : accept connection from everyone
        client_fd = accept(server_fd, (sa *)NULL, NULL);
#else
        client_fd = accept(server_fd, (saddr *)&addr, &addr_len);
        check(client_fd < 0, "accepting");
        // network to presentation
        inet_ntop(AF_INET, &addr, address, ADDRLEN);
        debug("client address: %s\n", address);
#endif
        char *request = NULL;
        char *content = NULL;
        char *tmp;
        char buff[READBYTES + 1];
        memset(buff, 0, sizeof(buff));
        size_t bytes;
        size_t sum = 0;
        // receive header
        while ((bytes = recv(client_fd, &buff, READBYTES, 0)))
        {
            sum += bytes;
            tmp = strjoin(request, buff);
            free(request);
            request = tmp;
            if ((content = strstr(request, header_end)))
                break;
            memset(buff, 0, sizeof(buff));
        }
        // split header and the body remaining
        debug("server read %zu bytes, received: \n<%s>\n", sum, request);
        char header[MAXLEN];
        memset(header, 0, sizeof(header));
        char *method = NULL;
        int s = 0;
        while (request[s])
        {
            if (strncmp("GET ", &request[s], strlen("GET ")) == 0)
            {
            }
            else if (strncmp("POST ", &request[s], strlen("POST ")) == 0)
            {
                // PARSE HEADER
                method = "POST";
                // TODO: throw error if content length dosn't exists
                char *tmp = strstr(&request[s], "Content-Type: ");
                tmp += strlen("Content-Type: ");
                s = 0;
                while (tmp[s] && !isspace(tmp[s]))
                    s++;
                char *type = calloc(s + 1, sizeof(char));
                strncpy(type, tmp, s);
                debug("type: <%s>\n", type);
                int i = 0;
                while (extentions[i].type && strcmp(extentions[i].type, type))
                    i++;
                check(!extentions[i].ext, "Insupported memtype");
                free(type);
                char *filename = calloc(strlen("post") + strlen(extentions[i].ext) + 1, sizeof(char));
                strcpy(filename, "post");
                strcpy(filename + strlen(filename), extentions[i].ext);
                int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0777);
                check(fd < 0, "Error openning %s", filename);
                // skip the first \r\n
                content = content ? strdup(content + strlen(header_end)) : content;
                tmp = strstr(content, end);

                char *chunck_len_str = calloc(tmp - content + 1, sizeof(char));
                strncpy(chunck_len_str, content, tmp - content);
                content = strstr(content, end) + strlen(end);
                char *ptr;
                long len = strtol(chunck_len_str, &ptr, 16);
                check(!ptr, "parsing len");
                debug("chunk-len: str:<%s> len:<%ld>\n", chunck_len_str, len, content);
                // debug("content: <%s>\n", chunck_len_str, len, content);
#if 1
                // TODO: ther might be cases where there is no complete \r\n
                // TODO: while the chunck len
                if (content)
                    len -= write(fd, content, strlen(content)); // write the first part
                content = NULL;
                int rd = READBYTES;
#if 0
                while (len > 0)
                {
                    if (rd > len)
                        rd = len;
                    memset(buff, 0, sizeof(buff));
                    while ((bytes = recv(client_fd, &buff, rd, 0)))
                    {
                        len -= write(fd, buff, bytes);
                        memset(buff, 0, sizeof(buff));
                        if (rd > len)
                            rd = len;
                    }
                    memset(buff, 0, sizeof(buff));
                    rd = READBYTES;
                    char *posted_file = NULL;
                    while (bytes = recv(client_fd, &buff, rd, 0))
                    {
                        tmp = strjoin(posted_file, buff);
                        free(posted_file);
                        posted_file = tmp;
                        if ((tmp = strstr(posted_file, end)))
                            break;
                        memset(buff, 0, sizeof(buff));
                    }
                    // content = posted_file;
                    posted_file = posted_file ? strdup(posted_file + strlen(end)) : posted_file;
                    if (posted_file)
                    {
                        tmp = strstr(posted_file, end);
                        if (tmp)
                        {
                            tmp += strlen(end);
                            // write(fd, tmp, strlen(tmp));
                            free(chunck_len_str);
                            chunck_len_str = calloc(tmp - posted_file + 1, sizeof(char));
                            strncpy(chunck_len_str, posted_file, tmp - posted_file);
                            posted_file = strstr(posted_file, end) + strlen(end);
                            len = strtol(chunck_len_str, &ptr, 16);
                            check(!ptr, "parsing len");
                            debug("chunk-len: str:<%s> len:<%ld>\n", chunck_len_str, len, posted_file);
                            // debug("content: <%s>\n", chunck_len_str, len, posted_file);
                            if (posted_file)
                                len -= write(fd, posted_file, strlen(posted_file));
                        }
                        else
                            check(1, "something went wrong <%s>", posted_file);
                    }
                    else
                        check(1, "something went wrong");

                }
#else
                memset(buff, 0, sizeof(buff));
                char *to_write;
                size_t size;
                while ((bytes = recv(client_fd, &buff, rd, 0)))
                {
                    /*
                        TODO:
                            check \r:
                                check \r\n:
                                    check \r\n0:
                                        check \r\n0\r\n:
                    */
                    char *tmp;
                    // if((tmp = strchr(buff, '\n')))
                    if ((tmp = strstr(buff, "\r\n0\r\n")))
                    {
                        write(fd, buff, bytes);
                        // write(fd, buff, buff - tmp);
                        exit(1);
                        break;
                    }
                    else if ((tmp = strchr(buff, '\r')))
                    {
                        printf("found\n");
                        printf("first: <%s>\n", buff);
                        to_write = strdup(buff);
                        memset(buff, 0, sizeof(buff));
                        tmp = calloc(bytes + recv(client_fd, &buff, rd, 0) + 1, sizeof(char));
                        printf("second: <%s>\n", buff);
                        strcpy(tmp, to_write);
                        strcpy(tmp + strlen(tmp), buff);
                        write(fd, tmp, strlen(tmp));
                    }
                    else
                    {
                        to_write = buff;
                        size = bytes;
                    }
                    write(fd, to_write, size);
                    memset(buff, 0, sizeof(buff));
                }
#endif

#endif
            }
            else
                s++;
        }

        // server response
        memset(header, 0, sizeof(header));
        if (!method)
            content = strdup("<div>Error 404: method not implemented</div>\r\n\r\n");
        else if (strcmp(method, "GET") == 0)
            sprintf(header + strlen(header), "%s\n", "HTTP/1.0 200 OK");
        else if (strcmp(method, "POST") == 0)
        {
            // check if it created succefully
            sprintf(header + strlen(header), "%s", "HTTP/1.1 201 Created\n");
            content = strdup("<div>Hello POST</div>\r\n\r\n");
        }

        // create header for response
        sprintf(header + strlen(header), "%s\n", "Content-Type: text/html; charset=UTF-8");
        sprintf(header + strlen(header), "%s%zu\n", "Content-Length: ", strlen(content));
        sprintf(header + strlen(header), "%s", end);
        printf("server send :\n");
        printf("=============== header ===============\n|%s|\n", header);
        check(send(client_fd, header, strlen(header), 0) < 0, "sending header");
        printf("===============  content  ===============\n|%s|\n", content);
        check(send(client_fd, content, strlen(content), 0) < 0, "sending content");
        free(content);
        close(client_fd);
        free(request);
    }
}
