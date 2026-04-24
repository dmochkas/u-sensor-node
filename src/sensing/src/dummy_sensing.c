#include "state_sensing.h"
#include "temp_sensing.h"

#include <stdint.h>

#define BATTERY_MIN_PERCENT 40u
#define BATTERY_MAX_PERCENT 100u

static uint32_t prng_next(void) {
    // xorshift32: tiny deterministic PRNG suitable for lightweight dummy data generation
    static uint32_t state = 0xA341316Cu;
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

int state_sense(uint8_t* bytes, size_t max_len, size_t* len_out) {
    if (bytes == NULL || len_out == NULL || max_len < 2u) {
        return -1;
    }

    // Byte 0: battery level (%) with slow variation and tiny noise.
    uint8_t battery = (uint8_t)(BATTERY_MIN_PERCENT + (prng_next() % (BATTERY_MAX_PERCENT - BATTERY_MIN_PERCENT + 1u)));
    // Byte 1: processor temperature in degC (realistic 35..65 range).
    uint8_t proc_temp = (uint8_t)(35u + (prng_next() % 31u));

    bytes[0] = battery;
    bytes[1] = proc_temp;
    *len_out = 2u;
    return 1;
}

int temp_sense(uint8_t* bytes, size_t max_len, size_t* len_out) {
    if (bytes == NULL || len_out == NULL || max_len < 1u) {
        return -1;
    }

    // 1-byte ambient temperature in degC around room conditions with small randomness.
    bytes[0] = (uint8_t)(22u + (prng_next() % 7u)); // 22..28 degC
    *len_out = 1u;
    return 1;
}