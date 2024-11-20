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

// TODO: avoid strings X'D
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
                debug("chunk-len: str:<%s> len:<%ld> \ncontent: <%s>\n", chunck_len_str, len, content);

#if 1
                // TODO: while the chunck len
                if (content)
                    len -= write(fd, content, strlen(content)); // write the first part
                content = NULL;
                int rd = READBYTES;
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
#if 1
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
                            debug("chunk-len: str:<%s> len:<%ld> \ncontent: <%s>\n", chunck_len_str, len, posted_file);
                            if (posted_file)
                                len -= write(fd, posted_file, strlen(posted_file));
                        }
                    }

#endif
                    // TODO: ther might be cases where thersi no complete \r\n
                    // if (buff[len])
                    // {
                    //     // len -= write(fd, content, strlen(content));
                    //     // free(content);
                    //     content = strdup(buff + len + strlen(end));
                    //     tmp = strstr(content, end);
                    //     if (tmp)
                    //     {
                    //         tmp += strlen(end);
                    //         free(chunck_len_str);
                    //         chunck_len_str = calloc(tmp - content + 1, sizeof(char));
                    //         strncpy(chunck_len_str, content, tmp - content - strlen(end));
                    //         len = strtol(chunck_len_str, &ptr, 16);
                    //         content += strlen(chunck_len_str) + strlen(end);
                    //     }
                    //     else
                    //         len = 0;
                    //     check(!ptr, "parsing len");
                    //     debug("chunk-len: str:<%s> len:<%ld> \ncontent: <%s>\n", chunck_len_str, len, content);
                    // }
                    // else
                    //     content = NULL;
                }
#elif 1
                while (len)
                {
                    if (content)
                        len -= write(fd, content, strlen(content));
                    memset(buff, 0, sizeof(buff));
                    while ((bytes = recv(client_fd, &buff, READBYTES, 0)) > 0)
                    {
                        if (len < READBYTES)
                        {
                            write(fd, buff, len);
                            break;
                        }
                        else
                            len -= write(fd, buff, bytes);
                        memset(buff, 0, sizeof(buff));
                    }
                    if (buff[len])
                    {
#if 1
                        content = strdup(buff + len + strlen(end));
                        tmp = strstr(content, end);
                        if (tmp)
                        {
                            tmp += strlen(end);
                            free(chunck_len_str);
                            chunck_len_str = calloc(tmp - content + 1, sizeof(char));
                            strncpy(chunck_len_str, content, tmp - content - strlen(end));
                            len = strtol(chunck_len_str, &ptr, 16);
                            content += strlen(chunck_len_str) + strlen(end);
                        }
                        else
                        {
                            // content = NULL;
                            // len = 0;
                            // exit(1);
                        }
                        check(!ptr, "parsing len");
#else
                        content = strdup(buff + len + strlen(end));
                        tmp = strstr(content, end);
                        if (tmp)
                        {
                            tmp += strlen(end);
                            free(chunck_len_str);
                            chunck_len_str = calloc(tmp - content + 1, sizeof(char));
                            strncpy(chunck_len_str, content, tmp - content - strlen(end));
                            len = strtol(chunck_len_str, &ptr, 16);
                            content += strlen(chunck_len_str) + strlen(end);
                        }
                        else
                            len = 0;
                        check(!ptr, "parsing len");
                        debug("chunk-len: str:<%s> len:<%ld> \ncontent: <%s>\n", chunck_len_str, len, content);
#endif
                    }
                    else
                        content = NULL;
                }

#else
                char *line = NULL;
                memset(buff, 0, sizeof(buff));
                if (!content || !(line = strstr(content, end)))
                {
                    while ((bytes = recv(client_fd, &buff, READBYTES, 0)))
                    {
                        sum += bytes;
                        char *tmp = strjoin(content, buff);
                        free(content);
                        content = tmp;
                        if ((line = strstr(content, end)))
                            break;
                        memset(buff, 0, sizeof(buff));
                    }
                }
                // TODO: check if line != NULL
                debug("hex-size: %zu - %zu = <%zu>\n", (uint64_t)line, (uint64_t)content, line - content);
                // get chunck-size
                tmp = calloc(line - content + 1, sizeof(char));
                strncpy(tmp, content, line - content);
                line = tmp;
                tmp = strdup(content + strlen(line) + strlen(end));
                free(content);
                content = tmp;

                long decimal = strtol(line, &tmp, 16);
                debug("Chunck-length: <%zu> from <%s>\n", decimal, line);

                check(*tmp != '\0', "in parsing hexadecimal <%s>", line);
                if (decimal == 0)
                {
                    debug("reached the end of file\n");
                    // exit(1);
                    break;
                }
                sum = strlen(content);
                write(fd, content, sum);
                int rd = READBYTES;
                if (decimal < rd)
                    rd = decimal;
                while ((bytes = recv(client_fd, &buff, rd, 0)))
                {
                    sum += bytes;
                    debug("post %zu\n", sum);
                    if (sum >= decimal)
                    {
                        write(fd, buff, sum - decimal);
                        exit(1);
                    }
                    else
                        write(fd, buff, bytes);
                    // TODO: this is not perfect
                    // line = strstr(buff, end); // skip first \r\n
                    // if (line)
                    // {
                    //     line += strlen(end);
                    //     tmp = strstr(line + strlen(end), end); // search for the second \r\n
                    //     char *tmp1 = calloc(tmp - line + 1, sizeof(char));
                    //     strncpy(tmp1, line, tmp - line);
                    //     line = tmp1;
                    //     decimal = strtol(line, &tmp, 16);
                    //     debug("Chunck-length: <%zu> from <%s>\n", decimal, line);
                    //     sum = 0;
                    // }
                    // else if(sum > decimal)
                    //     break;
                    // char *s = "\n==================\n";
                    // write(fd, s, strlen(s));
                    // exit(1);
                    // break;
                    memset(buff, 0, sizeof(buff));
                }
                exit(1);
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
