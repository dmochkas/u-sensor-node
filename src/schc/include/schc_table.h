#pragma once

#include <stdint.h>

#include "schc_base.h"

#define SCHC_TABLE_MAX_ENTRIES 20

typedef int (*payload_producer_t) (uint8_t*, size_t, size_t*);
typedef int (*payload_validator_t) (const uint8_t*, size_t); // size in bits

//typedef enum schc_entry_type {
//    PULL_ENTRY, PUSH_ENTRY
//} schc_entry_type_t;

typedef struct schc_lu_entry {
//    schc_entry_type_t type;
    schc_message_t req_msg;
    schc_message_t resp_msg;
    payload_producer_t get_payload;
    payload_validator_t validate_payload;
} schc_lu_entry_t;

typedef struct schc_lu_table {
    size_t n_entries;
    schc_lu_entry_t entries[SCHC_TABLE_MAX_ENTRIES];
} schc_lu_table_t;

int schc_lu_table_init();

int schc_accept_message(const schc_message_t* msg_in, const schc_message_t** msg_out);

int schc_get_message(const schc_rule_id_t* rule_id, schc_message_t* msg_out);