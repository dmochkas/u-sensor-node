#include "stat.h"

typedef struct stat_data {
    uint8_t n_recv;
    uint8_t n_send;
} stat_data_t;

static stat_data_t stat_data = {0};

int stat_get(uint8_t* bytes, size_t max_len, size_t* len_out) {
    if (max_len < 2) {
        return -1;
    }
    bytes[0] = stat_data.n_recv;
    bytes[1] = stat_data.n_send;
    *len_out = 2;
    return 1;
}

void stat_inc_recv() {
    stat_data.n_recv++;
}

void stat_inc_send() {
    stat_data.n_send++;
}