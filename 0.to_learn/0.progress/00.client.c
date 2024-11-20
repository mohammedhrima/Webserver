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

int main(int argc, char **argv)
{
    char *address = "127.1.1.0";
    int clie_fd;
    if ((clie_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        errput("creating socket");
    // connect to an address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    if (inet_aton(address, (struct in_addr *)&addr.sin_addr.s_addr) == 0)
        errput("inet_aton");
    if (connect(clie_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        errput("connection");
#if 1
    FILE *file = fopen("request.txt", "r");
    if (file == NULL)
        errput("openning file");
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *end = "\r\n\r\n";
    char *request = calloc(size + strlen(end) + 1, sizeof(char));
    fread(request, size, sizeof(char), file);
    strcpy(request + strlen(request), end);
    fclose(file);
#else
    char *request;
    request = "GET / HTTP/1.0\nHost: example.com\neol\n";
#endif
    printf("client send request:\n'%s'\n", request);
    if (send(clie_fd, request, strlen(request), 0) < 0)
        errput("sending request");
    free(request);
    printf("client finished sending\n");

    // recevi header
    char header[MAXLEN];
    memset(header, 0, sizeof(header));
    char *body = NULL;
    char buff[READBYTES + 1];
    memset(buff, 0, sizeof(buff));
    size_t bytes;
    size_t sum = 0;
    // get header
    while (bytes = recv(clie_fd, &buff, READBYTES, 0))
    {
        sum += bytes;
        strcpy(header + strlen(header), buff);
        if (body = strstr(header, "\r\n\r\n"))
            break;
        memset(buff, 0, sizeof(buff));
    }
    char *tmp = strdup(body + strlen("\r\n\r\n"));
    memset(body, 0, strlen(body));
    body = tmp;
    printf("client received:\n");
    printf("=============== header (%zu) ===============\n|%s|\n", sum, header);
    // parse header;
    // TODO: check if Content-length
    // if not throw error
    char *ContentLength = strstr(header, "Content-Length:");
    if (!ContentLength)
        errput("Content-Lentgh is required");
    ContentLength += strlen("Content-Length:");
    long length = atol(ContentLength);
    printf("client parsed header:\n");
    printf("    Content-Length: str(%s) long(%ld)\n", ContentLength, length);
#if 1
    body = realloc(body, length + 1);
    // get body
    sum = 0;
    // TODO: add a timeout in case body
    // didn't completely be receuved
    while (bytes = recv(clie_fd, &buff, READBYTES, 0))
    {
        sum += bytes;
        strcpy(body + strlen(body), buff);
        if (sum >= length)
            break;
        memset(buff, 0, sizeof(buff));
    }
    printf("===============  body  ===============\n|%s|\n", body);
#endif
    close(clie_fd);
    free(body);
}
