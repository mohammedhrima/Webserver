#include "header.h"

typedef struct sockaddr saddr;
typedef struct sockaddr_in saddr_in;

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
        printf(" (errno = %d) : %s\n", errno_save, strerror(errno));
        fflush(stdout);
    }
    va_end(ap);
    exit(1);
}
#define errput(args) errput_(__LINE__, args)

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
char *bin2hex(unsigned char *input, size_t len)
{
    char *result;
    char *base = "0123456789ABCDEF";
    if (input == NULL || len <= 0)
        return NULL;
    size_t reslen = (len * 3) + 1;
    result = calloc(reslen, sizeof(char));

    for (size_t i = 0; i < len; i++)
    {
        if (input[i] == '\n')
        {
            result[i * 3] = ' ';
            result[i * 3 + 1] = ' ';
            result[i * 3 + 2] = input[i];
        }
        else
        {
            result[i * 3] = base[input[i] >> 4];
            result[i * 3 + 1] = base[input[i] & 0x0F];
            result[i * 3 + 2] = ' '; // make it readable
        }
    }
    return result;
}

int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

int main(int argc, char **argv)
{
    saddr_in addr = (saddr_in){0};
    int serv_fd;
    int clie_fd;
    char *end = "\r\n";
    char *header_end = "\r\n\r\n";

    // TCP stream socket
    if ((serv_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        errput("creating socket");

    // address
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    int opt = 1;
    if (setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(int)) == -1)
        errput("seting socket");
    // listen and bind
    if (bind(serv_fd, (saddr *)&addr, sizeof(addr)) < 0)
        errput("binding");
    if (listen(serv_fd, 10) < 0)
        errput("listening");

    size_t reqlen = READBYTES;
    while (1)
    {
        saddr_in addr;
        socklen_t addr_len;
        char address[ADDRLEN];

        printf("server listening on port %d...\n", PORT);
#if 0
        // (sa*)NULL : accept connection from everyone
        clie_fd = accept(serv_fd, (sa *)NULL, NULL);
#else
        clie_fd = accept(serv_fd, (saddr *)&addr, &addr_len);
        // network to presentation
        inet_ntop(AF_INET, &addr, address, ADDRLEN);
        printf("client address: %s\n", address);
#endif
        char *client_request = NULL;
        // char
        char *content = NULL;
        char buff[READBYTES + 1];
        memset(buff, 0, sizeof(buff));
        size_t bytes;
        size_t sum = 0;
        // read header
        while (bytes = recv(clie_fd, &buff, READBYTES, 0))
        {
            sum += bytes;
            char *tmp = strjoin(client_request, buff);
            free(client_request);
            client_request = tmp;
            if (content = strstr(client_request, header_end))
                break;
            memset(buff, 0, sizeof(buff));
        }
        // TODO: split header to header and the remaining
        // add it to the content
        printf("server read %zu bytes, received: \n<%s>\n", sum, client_request);
        int s = 0;
        char header[MAXLEN];
        memset(header, 0, sizeof(header));
        char *method = NULL;
        while (client_request[s])
        {
            /*
                Response:
                    - prepare header:
                        - Content-Type
                        - Content-Length
                    - send file
            */
            if (strncmp("GET ", &client_request[s], strlen("GET ")) == 0)
            {
                method = "GET";
                s += strlen("GET ");
                while (isspace(client_request[s]))
                    s++;
                int e = s;
                while (client_request[e] && !isspace(client_request[e]))
                    e++;
                char *filename = calloc(e - s + 2, sizeof(char));
                /*
                    TODOS:
                        + this might be full path
                        + you will use the root file in config file

                */
                filename[0] = '.';
                strncpy(filename + 1, client_request + s, e - s);
                printf("server: GET file: <%s>\n", filename);

                // open file
                // Content-Type: text/html; charset=UTF-8
                // Content-Length:

                if (is_regular_file(filename))
                {
                    FILE *file = fopen(filename, "r");
                    if (file) // file found
                    {
                        fseek(file, 0, SEEK_END);
                        size_t size = ftell(file);
                        fseek(file, 0, SEEK_SET);
                        printf("file has length: %zu\n", size);

                        // create content
                        content = calloc(size + strlen(end) + 50, sizeof(char));
                        fread(content, size, sizeof(char), file);
                        sprintf(content + strlen(content), "%s", end);
                        fclose(file);
                    }
                    else // file not found
                        content = strdup("<div>Error 404: File Not Found</div>\r\n\r\n");
                }
                else
                    content = strdup("<div>Error 404: File Not Found or is a dir</div>\r\n\r\n");
                break;
            }
#if 1
            else if (strncmp("POST ", &client_request[s], strlen("POST ")) == 0)
            {
                // handle header
                method = "POST";
                s += strlen("POST ");
                while (isspace(client_request[s]))
                    s++;
                int e = s;
                while (client_request[e] && !isspace(client_request[e]))
                    e++;
                while (client_request[e] &&
                       strncmp("Content-Type: ", client_request + e, strlen("Content-Type: ")))
                    e++;
                e += strlen("Content-Type: ");
                s = e;
                while (client_request[e] && !isspace(client_request[e]))
                    e++;
                char *type = calloc(e - s + 1, sizeof(char));
                strncpy(type, client_request + s, e - s);
                printf("type: <%s>\n", type);
                int i = 0;
                while (extentions[i].type && strcmp(extentions[i].type, type))
                    i++;
                if (!extentions[i].ext)
                    errput("Invalid memtype");
                free(type);
                char *filename = calloc(strlen("post") + strlen(extentions[i].ext) + 1, sizeof(char));
                strcpy(filename, "post");
                strcpy(filename + strlen(filename), extentions[i].ext);
                /*
                    TODO:
                        + read first line after header
                        + represent the lenght of the the content
                        + read the content

                        + small file has one seperator
                        + big files had multiple siparators
                        + read buff find separator and remove it
                        + then write to file
                        + first time get separator
                        + get it's length
                        + read the length
                        + get separator and soo forth
                */
                // handle content
                int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0777);
                content = strdup(content + strlen(header_end));
                printf("0.content: <%s>\n", content);
                memset(buff, 0, sizeof(buff));
                char *line = NULL;
                while (bytes = recv(clie_fd, &buff, READBYTES, 0))
                {
                    sum += bytes;
                    char *tmp = strjoin(content, buff);
                    free(content);
                    content = tmp;
                    if (line = strstr(content, end))
                        break;
                    memset(buff, 0, sizeof(buff));
                }
                // if (line)
                // {
                printf("str-size: %zu - %zu = <%zu>\n", (uint64_t)line, (uint64_t)content, line - content);
                char *tmp = calloc(line - content + 1, sizeof(char));
                strncpy(tmp, content, line - content);
                line = tmp;
                tmp = strdup(content + strlen(line) + strlen(end));
                free(content);
                content = tmp;

                /*
                    TODO:
                        + each time read READBYTES
                        + check if you did read the last decimal_value size
                        + search for the separator
                        + ...
                        + check the case if you read two separators in same line !!!
                */
                char *endptr;
                long decimal_value;
                decimal_value = strtol(line, &endptr, 16);
                if (*endptr != '\0')
                    errput("in parsing hexadecimal");
                printf("Chunck-length: <%zu> from <%s>\n", decimal_value, line);
                printf("Content:       <%s>\n", content);

                sum = strlen(content);
                write(fd, content, sum);
                decimal_value -= sum;
                memset(buff, 0, sizeof(buff));
                while (bytes = recv(clie_fd, &buff, READBYTES, 0))
                {
                    // TODO: check recv and write failure
                    printf("server posted <%ld> remain <%ld>\n", sum, decimal_value);
                    sum += bytes;
                    if (bytes >= decimal_value)
                    {
                        write(fd, buff, decimal_value);
                        printf("server posted <%ld> \n", decimal_value);
                        free(content);
                        // TODO: protect ot from segvault
                        content = strdup(buff + decimal_value + strlen(end));
                        if (!(line = strstr(content, end)))
                        {
                            memset(buff, 0, sizeof(buff));
                            while (bytes = recv(clie_fd, &buff, READBYTES, 0))
                            {
                                sum += bytes;
                                tmp = strjoin(content, buff);
                                free(content);
                                content = tmp;
                                if (line = strstr(content, end))
                                    break;
                                memset(buff, 0, sizeof(buff));
                            }
                        }
                        printf("%d.Content <%s>\n", __LINE__, content);
                        // if (line)
                        // {
                        printf("str-size: %zu - %zu = <%zu>\n", (uint64_t)line, (uint64_t)content, line - content);
                        tmp = calloc(line - content + 1, sizeof(char));
                        strncpy(tmp, content, line - content);
                        line = tmp;
                        tmp = strdup(content + strlen(line) + strlen(end));
                        free(content);
                        content = tmp;

                        /*
                            TODO:
                                + each time read READBYTES
                                + check if you did read the last decimal_value size
                                + search for the separator
                                + ...
                                + check the case if you read two separators in same line !!!
                        */
                        decimal_value = strtol(line, &endptr, 16);
                        printf("Chunck-length: <%zu> from <%s>\n", decimal_value, line);
                        // printf("Content:       <%s>\n", content);
                        if (*endptr != '\0')
                            errput(("in parsing hexadecimal <%s>", line));
                        if (decimal_value == 0)
                        {
                            printf("reached the end\n");
                            // exit(1);
                            break;
                        }
                        sum = strlen(content);
                        write(fd, content, sum);
                        // decimal_value = 0;
                        // printf("found first terminator\n");
                        // exit(1);
                    }
                    else
                    {
                        write(fd, buff, bytes);
                        decimal_value -= bytes;
                    }
                    // else
                    //     ;
                    memset(buff, 0, sizeof(buff));
                }
                // bytes = recv(clie_fd, &buff, READBYTES, 0);
                // printf("server posted <%ld> <%s>\n", sum, buff);
                // sum += bytes;
                // write(fd, buff, bytes);
                exit(1);
                // }
                // else
                //     printf("special character not found\n");

                // TODO: check NULL
                // printf("content:<%s>\nline:<%s>\n", content, line);
                // line += strlen(end);
                // if (line) // file too long
                // {
                //     char *tmp1 = strstr(line, end);
                //     if (!tmp1)
                //         errput("not found");
                //     char *tmp2 = calloc(tmp1 - line + 1, sizeof(char));
                //     strncpy(tmp2, line, tmp1 - line);
                //     line = tmp1;
                //     long n = strtol(line, NULL, 16);

                //     printf("line: <%s><%ld>\n\n", line, n);
                // }
#if 0
                sum = strlen(content);
                write(fd, content, sum);
                while (bytes = recv(clie_fd, &buff, READBYTES, 0))
                {
                    // TODO: check recv and write failure
                    printf("server posted <%ld> <%s>\n", sum, buff);
                    sum += bytes;
                    if (sum >= n)
                    {
                        tmp0 = strrchr(buff, '0');
                        write(fd, buff, tmp0 - buff - 2);
                        break;
                    }
                    else
                        write(fd, buff, bytes);
                    memset(buff, 0, sizeof(buff));
                }
#endif
                free(content);
                free(line);
                free(filename);
                exit(1);
            }
#endif
            else
                s++;
        }

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
        if (send(clie_fd, header, strlen(header), 0) < 0)
            errput("sending header");

        printf("===============  content  ===============\n|%s|\n", content);
        if (send(clie_fd, content, strlen(content), 0) < 0)
            errput("sending content");

        free(content);
        close(clie_fd);
        free(client_request);
    }
}
