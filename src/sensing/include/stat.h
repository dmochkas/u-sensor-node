#pragma once

#include <stdint.h>
#include <stddef.h>

int stat_get(uint8_t* bytes, size_t max_len, size_t* len_out);

int stat_get_sent(uint8_t* bytes, size_t max_len, size_t* len_out);

void stat_inc_recv();

void stat_inc_send();