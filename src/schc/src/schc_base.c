#include "schc_base.h"

#include <string.h>

#include "utils.h"

// Warning: assumes rule id of 1 byte
int extract_schc_message(const uint8_t* bytes, size_t len, schc_message_t* msg_out) {
    if (len < 1 || len > SCHC_MAX_PAYLOAD_SIZE + 1) {
        return -1;
    }
    msg_out->rule_id = bytes[0];
    msg_out->pl_size_bits = BYTES_TO_BITS(len - 1);
    memcpy(msg_out->payload, bytes + 1, len -1);

    return 1;
}

int schc_encode(const schc_message_t* msg_in, uint8_t* bytes, size_t max_len, size_t* len_out_bits) {
    int ret = -1;

    const size_t payload_size_bytes = BITS_TO_BYTES(msg_in->pl_size_bits);
    *len_out_bits = sizeof(schc_rule_id_t) * BYTE_BITS + msg_in->pl_size_bits;

    if (max_len < BITS_TO_BYTES(*len_out_bits)) {
        goto finish;
    }
    bytes[0] = msg_in->rule_id; // Warning: assumes rule id of 1 byte
    memcpy(bytes + 1, msg_in->payload, payload_size_bytes);

    ret = 1;

    finish:
    return ret;
}