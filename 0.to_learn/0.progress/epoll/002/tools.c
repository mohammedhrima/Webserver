#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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

void *memejoin(void *left, void *right, size_t llen, size_t rlen)
{
    unsigned char *result = calloc(llen + rlen + 1, 1);
    memcpy(result, left, llen);
    memcpy(result + llen, right, rlen);
    return result;
}

int main()
{
    char *str0 = "abcdefgh";
    char *str1 = "a";
    char *res = (char *)find(str0, str1, strlen(str0), strlen(str1));
    printf("%s\n", res ? res : "(null)");

    int i = 1000000;
    int j = 0;
    while (i)
    {
        i /= 2;
        j++;
    }
    printf("<%d>\n", j);
    res = memejoin(str0, str1, strlen(str0), strlen(str1));
    printf("res: %s\n", res);
}