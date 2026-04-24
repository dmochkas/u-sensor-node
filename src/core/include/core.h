#pragma once

#include <stdint.h>
#include <stddef.h>

#define MATS_LT_SET_COMMAND_FMT "*SET,%s,%s,"
#define MATS_LT_GET_COMMAND_FMT "*GET,%s,"
#define MATS_LT_RESTART_COMMAND "*RESTART\r"
#define MATS_LT_RESET_COMMAND "*RESET\r"
#define MATS_LT_UPGRADE_COMMAND "*UPGRADE\r"
#define MATS_LT_OK_RESPONSE "#OK\r"

#define MATS_LT_ACK_RESPONSE "#ACK\r"
#define MATS_LT_SENT_RESPONSE "#SENT\r"
// ACOUSTIC_MODEM V1.0.11 TDMA,AUV
// ACOUSTIC_MODEM V1.0.11 SRC-DEST
#define MATS_LT_RESTART_RESPONSE_REGEX "ACOUSTIC_MODEM (V[^,]+) ([[:alnum:],-]+)\r"
#define MATS_LT_GET_RESPONSE_REGEX "#=([^\r\n]+)\r"

#define MATS_LT_SEND_SRC_DST_PKT_FMT "*TRA,%d,%d,%d,%s,"
#define MATS_LT_RECV_SRC_DST_PKT_REGEX "\\*RCV,([0-9]+),([0-9]+),([0-9]+),([[:xdigit:]]+),!([[:xdigit:]]{4})\r"

#define MATS_LT_PROTOCOL_PARAM_ADDR "0"
#define MATS_LT_BAUD_PARAM_ADDR "1"
#define MATS_LT_TIMEOUT_PARAM_ADDR "2"

#define MATS_LT_PROTOCOL_TDMA "TDMA"
#define MATS_LT_PROTOCOL_SRC_DEST "SRC-DEST"

#ifndef MATS_LT_RESTART_SLEEP_TIME_MS
#define MATS_LT_RESTART_SLEEP_TIME_MS 1000
#endif

#define MATS_LT_MAX_RESP_SIZE 127
#define MATS_LT_MAX_PL_SIZE 4
#define MATS_LT_CRC_SIZE 4

#define RESP_SLEEP_TIME 1000

//// A and R flags are incompatible
//typedef enum ahoi_packet_flags {
//    AHOI_NO_FLAGS = 0x00,
//    AHOI_A_FLAG = 0x01,
//    AHOI_R_FLAG = 0x02,
//    AHOI_E_FLAG = 0x04,
//    AHOI_AE_FLAGS = 0x05,
//    AHOI_RE_FLAGS = 0x06,
//} ahoi_packet_flags_t;
//
//typedef struct {
//    uint8_t power;
//    uint8_t rssi;
//    uint8_t biterrors;
//    uint8_t agcMean;
//    uint8_t agcMin;
//    uint8_t agcMax;
//} ahoi_footer_t;
//
//typedef struct {
//    uint8_t src;
//    uint8_t dst;
//    uint8_t type;
//    uint8_t flags;
//    uint8_t seq;
//    uint8_t pl_size;
//    uint8_t payload[AHOI_MAX_PAYLOAD_SIZE];
//    ahoi_footer_t footer;
//} ahoi_packet_t;

typedef struct mats_lt_src_dst_packet {
    uint8_t src;
    uint8_t dst;
    uint8_t pl_size_bits;
    uint8_t payload[MATS_LT_MAX_PL_SIZE];
} mats_lt_src_dst_packet_t;

typedef enum mats_lt_protocol_cfg {
    TDMA, SRC_DEST
} mats_lt_protocol_cfg_t;

typedef void (*msg_handler_t) (mats_lt_src_dst_packet_t* p);

int mats_lt_connect(const char *port, int baudrate);

int set_ahoi_id(int fd, uint8_t id);

void set_msg_handler(msg_handler_t h);

/**
 * Should be called in a loop
 * @return
 */
void handle_receive(int fd);

void handle_receive_silent(int fd);

int trigger_send(int fd, const mats_lt_src_dst_packet_t* p);

void print_packet(const mats_lt_src_dst_packet_t *pkt);
