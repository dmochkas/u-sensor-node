#pragma once

#include <stdint.h>
#include <stddef.h>

int state_sense(uint8_t* bytes, size_t max_len, size_t* len_out);