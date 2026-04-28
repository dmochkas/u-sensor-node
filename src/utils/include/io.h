#pragma once

#include <stddef.h>
#include <stdint.h>

#define DEFAULT_READ_SIZE 256
#define IO_MAX_DYNAMIC_BUF_SIZE 2048

typedef uint16_t buffer_len_t;

typedef struct buffer {
    buffer_len_t length;
    uint8_t* value;
} buffer_t;

typedef struct {
    const buffer_t *buffer;
    size_t chunk;
    size_t pos;
} reader_t;

typedef struct {
    buffer_t *buffer;
    size_t chunk;
    size_t pos;
} writer_t;

int mem_read(reader_t *r, void *dst, size_t n);

int mem_write(writer_t *w, const buffer_t *src, size_t n);

long io_read_until_ch(int fd, buffer_t* b, char target);

long io_read_until_match(int fd, buffer_t* b, char* regex);