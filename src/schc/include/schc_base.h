#pragma once

#include <stdint.h>
#include <stddef.h>

// To support MATS-LT
#define SCHC_MAX_PAYLOAD_SIZE 3

#ifndef SENSOR_LINK_ID
#define SENSOR_LINK_ID 0
#endif

#define COMPOSE_RULE_ID(id_base) ((uint8_t)(((SENSOR_LINK_ID & 0x0F) << 4) | ((id_base) & 0x0F)))


typedef uint8_t schc_rule_id_t;

typedef struct schc_message {
    schc_rule_id_t rule_id;
    uint16_t pl_size_bits;
    uint8_t payload[SCHC_MAX_PAYLOAD_SIZE];
} schc_message_t;

int extract_schc_message(const uint8_t* bytes, size_t len, schc_message_t* msg_out);

int schc_encode(const schc_message_t* msg_in, uint8_t* bytes, size_t max_len, size_t* len_out);