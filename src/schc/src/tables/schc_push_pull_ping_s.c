#include "schc_table.h"
#include "temp_sensing.h"
#include "state_sensing.h"
#include "stat.h"

#include <string.h>

schc_lu_table_t lu_table = {0};

int schc_lu_table_init() {
    memset(&lu_table, 0, sizeof(lu_table));

    /* Push messages */

    lu_table.entries[0] = (schc_lu_entry_t) {
            .req_msg = (schc_message_t) {
                    .rule_id = COMPOSE_RULE_ID(0x01),
            },
            .get_payload = temp_sense
    };
    lu_table.n_entries++;

    lu_table.entries[1] = (schc_lu_entry_t) {
            .req_msg = (schc_message_t) {
                    .rule_id = COMPOSE_RULE_ID(0x02),
            },
            .get_payload = state_sense
    };
    lu_table.n_entries++;

    /* Pull messages */

    lu_table.entries[2].req_msg = (schc_message_t) {
            .rule_id = COMPOSE_RULE_ID(0x0A),
    };
    lu_table.entries[2].resp_msg = (schc_message_t) {
            .rule_id = COMPOSE_RULE_ID(0x03),
    };
    lu_table.entries[2].get_payload = stat_get;
    lu_table.n_entries++;

    lu_table.entries[3].req_msg = (schc_message_t) {
            .rule_id = COMPOSE_RULE_ID(0x0B),
    };
    lu_table.entries[3].resp_msg = (schc_message_t) {
            .rule_id = COMPOSE_RULE_ID(0x04),
    };
    lu_table.entries[3].get_payload = temp_sense;
    lu_table.n_entries++;

    lu_table.entries[4].req_msg = (schc_message_t) {
            .rule_id = COMPOSE_RULE_ID(0x0C),
    };
    lu_table.entries[4].resp_msg = (schc_message_t) {
            .rule_id = COMPOSE_RULE_ID(0x05),
    };
    lu_table.entries[4].get_payload = state_sense;
    lu_table.n_entries++;

    return 1;
}