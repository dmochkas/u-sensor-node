#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#define BYTE_BITS 8
#define BYTE_MAX 255

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define BITS_TO_BYTES(bits) (((bits) + (BYTE_BITS - 1)) / BYTE_BITS)
#define BYTES_TO_BITS(bytes) ((bytes) * BYTE_BITS)

typedef enum param_type {
    TYPE_STRING, TYPE_INT, TYPE_UINT8
} param_type_t;

typedef struct param {
    param_type_t type;
    union {
        struct {
            size_t max_len;
            char** val_ptr;
        } str_val;
        int32_t* int_val;
        uint8_t* uint8_val;
    };
} param_t;

#define AS_STRING_PARAM(param_val, max_len) (param_t) {.type = TYPE_STRING, .str_val = { max_len, &(param_val) }}
#define AS_INT_PARAM(param_val) (param_t) {.type = TYPE_INT, .int_val = &param_val}
#define AS_UINT8_PARAM(param_val) (param_t) {.type = TYPE_UINT8, .uint8_val = &param_val}

int regex_match(const char* str, const char* pattern);

int re_pattern_extract_groups(const char* str, const char* pattern, ...);

int re_pattern_extract_groups_va(const char* str, const char* pattern, va_list arg_ptr);

int hex_string_to_bytes(const char *hex, uint8_t *out, size_t max_size, size_t *out_size);

int bytes_to_hex_string(const uint8_t *bytes, size_t bytes_size, char *hex, size_t max_size, size_t *out_size);