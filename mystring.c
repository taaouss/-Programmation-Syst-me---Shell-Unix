#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include "mystring.h"

struct string *string_new(size_t capacity)
{
    struct string *str_n = malloc(sizeof(struct string));
    str_n->capacity = capacity;
    str_n->length = 0;
    str_n->data = malloc(capacity * sizeof(char));
    memset(str_n->data, '\0', capacity);
    return str_n;
}

void string_delete(struct string *str)
{
    free(str->data);
    free(str);
}

int string_append(struct string *dest, char *src)
{
    size_t src_length = strlen(src);

    if (!(dest->length + src_length < dest->capacity))
    {
        return 0;
    }

    memmove(dest->data + dest->length, src, src_length);

    dest->length += src_length;

    *(dest->data + dest->length) = '\0';

    return 1;
}

void string_cpy(struct string *dest, struct string *src)
{
    if (src->length + 1 > dest->capacity)
    {
        if (realloc(dest->data, sizeof(src->data)) == NULL)
        {
            write(STDERR_FILENO, "error : realloc", strlen("error : realloc"));
            return;
        }
        dest->capacity = src->capacity;
    }
    memmove(dest->data, src->data, src->length);
    dest->length = src->length;
}
