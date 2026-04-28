#include "utils.h"

#include <regex.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int regex_match(const char* str, const char* pattern) {
    int ret = -1;
    regex_t re;
    if (regcomp(&re, pattern, REG_EXTENDED) != 0) {
        return -1;
    }
    if (regexec(&re, str, 0, NULL, 0) != 0) {
        goto finish;
    }

    ret = 0;

    finish:
    regfree(&re);
    return ret;
}

int re_pattern_extract_groups(const char* str, const char* pattern, ...) {
    va_list args;
    va_start(args, pattern);
    int ret = re_pattern_extract_groups_va(str, pattern, args);
    va_end(args);
    return ret;
}

int re_pattern_extract_groups_va(const char* str, const char* pattern, va_list arg_ptr) {
    int ret = -1;
    if (str == NULL || pattern == NULL) {
        return -1;
    }

    regex_t re;
    if (regcomp(&re, pattern, REG_EXTENDED) != 0) {
//        printf("regex compile failed\n"); TODO: Log error
        return -1;
    }
    size_t nmatch = re.re_nsub + 1;
    regmatch_t matches[nmatch];
    int rc = regexec(&re, str, nmatch, matches, 0);
    if (rc != 0) {
        goto finish;
    }

    size_t n_groups = (nmatch > 0) ? nmatch - 1 : 0;
    for (size_t i = 0; i < n_groups; ++i) {
        param_t *param = va_arg(arg_ptr, param_t*);
        size_t match_idx = i + 1;
        size_t match_size = (size_t) (matches[match_idx].rm_eo - matches[match_idx].rm_so);

        if (param->type == TYPE_UINT8) {
            if (param->uint8_val == NULL || match_size == 0) {
                goto finish;
            }

            char tmp[match_size + 1];
            memcpy(tmp, str + matches[match_idx].rm_so, match_size);
            tmp[match_size] = '\0';

            errno = 0;
            char *end = NULL;
            unsigned long value = strtoul(tmp, &end, 10);
            if (end == tmp || *end != '\0') {
                goto finish;
            }

            if (errno == ERANGE || value > UINT8_MAX) {
                value = UINT8_MAX;
            }

            *param->uint8_val = (uint8_t) value;
            continue;
        }

        if (param->type != TYPE_STRING) {
            continue;
        }


        if (param->str_val.val_ptr == NULL || *param->str_val.val_ptr == NULL) {
            goto finish;
        }
        if (param->str_val.max_len < match_size + 1) {
            goto finish;
        }

        memcpy(*param->str_val.val_ptr, str + matches[match_idx].rm_so, match_size);
        (*param->str_val.val_ptr)[match_size] = '\0';
    }

    ret = 0;

    finish:
    regfree(&re);
    return ret;
}

int hex_string_to_bytes(const char *hex, uint8_t *out, size_t max_size, size_t *out_size) {
    if (hex == NULL || out == NULL || out_size == NULL) {
        return -1;
    }

    size_t len = 0;
    while (hex[len] != '\0') len++;

    if ((len % 2) != 0 || len / 2 > max_size) {
        return -1;
    }

    for (size_t i = 0; i < len / 2; ++i) {
        char hi_c = hex[2 * i];
        char lo_c = hex[2 * i + 1];
        int hi = (hi_c >= '0' && hi_c <= '9') ? (hi_c - '0') :
                 (hi_c >= 'a' && hi_c <= 'f') ? (10 + (hi_c - 'a')) :
                 (hi_c >= 'A' && hi_c <= 'F') ? (10 + (hi_c - 'A')) : -1;
        int lo = (lo_c >= '0' && lo_c <= '9') ? (lo_c - '0') :
                 (lo_c >= 'a' && lo_c <= 'f') ? (10 + (lo_c - 'a')) :
                 (lo_c >= 'A' && lo_c <= 'F') ? (10 + (lo_c - 'A')) : -1;
        if (hi < 0 || lo < 0) {
            return -1;
        }
        out[i] = (uint8_t)((hi << 4) | lo);
    }

    *out_size = len / 2;
    return 0;
}

int bytes_to_hex_string(const uint8_t *bytes, size_t bytes_size, char *hex, size_t max_size, size_t *out_size) {
    static const char digits[] = "0123456789ABCDEF";
    if (hex == NULL || out_size == NULL) {
        return -1;
    }
    if (bytes_size > (SIZE_MAX - 1) / 2) {
        return -1;
    }

    size_t hex_size = bytes_size * 2;
    if (hex_size + 1 > max_size) {
        return -1;
    }

    if (bytes_size == 0) {
        hex[0] = '\0';
        *out_size = 0;
        return 0;
    }
    if (bytes == NULL) {
        return -1;
    }

    for (size_t i = 0; i < bytes_size; ++i) {
        uint8_t byte = bytes[i];
        hex[2 * i] = digits[(byte >> 4) & 0x0F];
        hex[2 * i + 1] = digits[byte & 0x0F];
    }
    hex[hex_size] = '\0';
    *out_size = hex_size;
    return 0;
}
