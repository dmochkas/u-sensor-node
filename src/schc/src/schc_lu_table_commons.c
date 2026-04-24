#include "schc_table.h"

#include <stddef.h>

#include "utils.h"

extern schc_lu_table_t lu_table;

int schc_accept_message(const schc_message_t* msg_in, const schc_message_t** msg_out) {
    for (int i = 0; i < lu_table.n_entries; ++i) {
        schc_lu_entry_t* e = &lu_table.entries[i];
        if (e->req_msg.rule_id == msg_in->rule_id) {
            if (e->get_payload != NULL) {
                size_t pl_size;
                int res = e->get_payload(e->resp_msg.payload, SCHC_MAX_PAYLOAD_SIZE, &pl_size);
                if (res < 0) {
                    break;
                }
                e->resp_msg.pl_size_bits = BYTES_TO_BITS(pl_size);
            }
            *msg_out = &e->resp_msg;
            return 1;
        }
        if (e->resp_msg.rule_id == msg_in->rule_id) {
            if (e->validate_payload != NULL) {
                if (e->validate_payload(msg_in->payload, msg_in->pl_size_bits) < 0) {
                    break;
                }
            }
            *msg_out = NULL;
            return 2;
        }
    }

    *msg_out = NULL;
    return -1;
}

void schc_get_message(const schc_rule_id_t* rule_id, const schc_message_t** msg_out) {
    for (int i = 0; i < lu_table.n_entries; ++i) {
        schc_lu_entry_t* e = &lu_table.entries[i];
        if (e->resp_msg.rule_id == *rule_id) {
            if (e->get_payload != NULL) {
                size_t pl_size;
                int res = e->get_payload(e->resp_msg.payload, SCHC_MAX_PAYLOAD_SIZE, &pl_size);
                if (res < 0) {
                    break;
                }
                e->resp_msg.pl_size_bits = BYTES_TO_BITS(pl_size);
            }
            *msg_out = &e->resp_msg;
            return;
        }
    }

    *msg_out = NULL;
}