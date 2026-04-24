#include "io.h"

#include <unistd.h>
#include <string.h>

int mem_read(reader_t *r, void *dst, size_t n)
{
    if (r->pos >= r->buffer->length)
        return 0; // EOF

    size_t remaining = r->buffer->length - r->pos;
    if (n > remaining)
        n = remaining;

    memcpy(dst, r->buffer->value + r->pos, n);
    r->pos += n;

    return (int) n;
}

int mem_write(writer_t *w, const buffer_t *src, size_t n)
{
    if (w->buffer == NULL) {
        w->pos += n;
        return (int) n;
    }

    if (w->pos >= w->buffer->length)
        return 0; // buffer full

    size_t remaining = w->buffer->length - w->pos;
    if (n > remaining)
        n = remaining;

    memcpy(w->buffer->value + w->pos, src->value, n);
    w->pos += n;

    return (int) n;
}

long io_read_until_ch(int fd, buffer_t* b, char target) {
    size_t total = 0;
    while (total < b->length) {
        char ch;
        ssize_t n = read(fd, &ch, 1);
        if (n < 0) {
            return -1;
        }
        if (n == 0) {
            break;
        }

        b->value[total++] = ch;
        if (ch == target) {
            return (ssize_t) total;
        }
    }

    return -1;
}