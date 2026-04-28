#include "schc_table.h"

#include <stddef.h>
#include <string.h>

#include "utils.h"

extern schc_lu_table_t lu_table;

int schc_accept_message(const schc_message_t* msg_in, schc_message_t* msg_out) {
#ifdef N_INSTANCES_ENABLED
    const uint8_t instance_id = (uint8_t)(msg_in->rule_id >> MSB_SIZE);
    const uint8_t target_mask = (uint8_t)(UINT8_C(0xFF) >> MSB_SIZE);
    const uint8_t target_id = (uint8_t)(msg_in->rule_id & target_mask);
#endif
#ifndef N_INSTANCES_ENABLED
    const uint8_t instance_id = 0;
    const uint8_t target_mask = UINT8_C(0xFF);
    const uint8_t target_id = msg_in->rule_id;
#endif
    int ret = -1;
    for (int i = 0; i < lu_table.n_entries; ++i) {
        schc_lu_entry_t* e = &lu_table.entries[i];
        schc_message_t* target = NULL;
        if ((e->req_msg.rule_id & target_mask) == target_id) {
            if (e->get_payload != NULL) {
                size_t pl_size;
                int res = e->get_payload(e->resp_msg.payload, SCHC_MAX_PAYLOAD_SIZE, &pl_size);
                if (res < 0) {
                    break;
                }
                e->resp_msg.pl_size_bits = BYTES_TO_BITS(pl_size);
            }
            target = &e->resp_msg;
            ret = 1;
        } else if ((e->resp_msg.rule_id & target_mask) == target_id) {
            if (e->validate_payload != NULL) {
                if (e->validate_payload(msg_in->payload, msg_in->pl_size_bits) < 0) {
                    break;
                }
            }
            target = &e->req_msg;
            ret = 2;
        } else {
            continue;
        }

        msg_out->rule_id = (instance_id << MSB_SIZE) | (target->rule_id & target_mask);
        msg_out->pl_size_bits = target->pl_size_bits;
        memcpy(msg_out->payload, target->payload, sizeof(msg_out->payload));
        return ret;
    }

    return ret;
}

int schc_get_message(const schc_rule_id_t* rule_id, schc_message_t* msg_out) {
#ifdef N_INSTANCES_ENABLED
    const uint8_t instance_id = (uint8_t)(*rule_id >> MSB_SIZE);
    const uint8_t target_mask = (uint8_t)(UINT8_C(0xFF) >> MSB_SIZE);
    const uint8_t target_id = (uint8_t)(*rule_id & target_mask);
#endif
#ifndef N_INSTANCES_ENABLED
    const uint8_t instance_id = 0;
    const uint8_t target_mask = UINT8_C(0xFF);
    const uint8_t target_id = *rule_id;
#endif
    for (int i = 0; i < lu_table.n_entries; ++i) {
        schc_lu_entry_t* e = &lu_table.entries[i];
        schc_message_t* target = NULL;
        if ((e->resp_msg.rule_id & target_mask) == target_id) {
            target = &e->resp_msg;
        } else if ((e->req_msg.rule_id & target_mask) == target_id) {
            target = &e->req_msg;
        } else {
            continue;
        }

        if (e->get_payload != NULL) {
            size_t pl_size;
            int res = e->get_payload(target->payload, SCHC_MAX_PAYLOAD_SIZE, &pl_size);
            if (res < 0) {
                break;
            }
            target->pl_size_bits = BYTES_TO_BITS(pl_size);
        }
        msg_out->rule_id = (instance_id << MSB_SIZE) | (target->rule_id & target_mask);
        msg_out->pl_size_bits = target->pl_size_bits;
        memcpy(msg_out->payload, target->payload, sizeof(msg_out->payload));
        return 1;
    }

    return -1;
}