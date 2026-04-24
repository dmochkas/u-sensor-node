#pragma once

#include <stdint.h>
#include <stddef.h>

#define TEMP_PAYLOAD_SIZE 2

int temp_sense(uint8_t* bytes, size_t max_len, size_t* len_out);