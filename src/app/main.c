#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "logging.h"
#include "core.h"
#include "schc_base.h"
#include "schc_table.h"
#include "stat.h"
#include "utils.h"

#ifndef PUSH_INTERVAL_S
#define PUSH_INTERVAL_S 30
#endif

static int g_fd = -1;

static uint8_t seq = 0;
static size_t push_rr_idx = 0;

static long long monotonic_ms(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0) {
        return 0;
    }

    return ((long long)ts.tv_sec * 1000LL) + ((long long)ts.tv_nsec / 1000000LL);
}

static int send_schc_payload(uint8_t src, uint8_t dst, const schc_message_t *schc_msg)
{
    mats_lt_src_dst_packet_t pkt = {0};
    pkt.src = src;
    pkt.dst = dst;

    size_t encode_size = 0;
    int encode_st = schc_encode(schc_msg, pkt.payload, MATS_LT_MAX_PL_SIZE, &encode_size);
    if (encode_st < 0) {
        return -1;
    }
    pkt.pl_size_bits = encode_size;

    if (trigger_send(g_fd, &pkt) < 0) {
        return -1;
    }

    stat_inc_send();
    seq++;
    return 1;
}

static void send_push_message(void)
{
    // TODO: For now hardcoded, no more than 2 bytes
    static const schc_rule_id_t push_rule_bases[] = {0x01, 0x1D};

    schc_rule_id_t rule_id = push_rule_bases[push_rr_idx];
    schc_message_t schc_push = {0};
    if (schc_get_message(&rule_id, &schc_push) < 0) {
        zlog_warn(error_cat, "Lookup table entry not found");
        return;
    }
    (void)send_schc_payload(NODE_ID, GATEWAY_ID, &schc_push);

    char hex[16] = {0};
    size_t n;
    if (bytes_to_hex_string(schc_push.payload, BITS_TO_BYTES(schc_push.pl_size_bits), hex, sizeof(hex), &n) < 0) {
        zlog_error(error_cat, "bytes_to_hex_string internal error");
        return;
    }
    zlog_info(ok_cat, "SCHC push sent: rule_id=0x%02x, payload=0x%s", rule_id, hex);

    push_rr_idx = (push_rr_idx + 1u) % (sizeof(push_rule_bases) / sizeof(push_rule_bases[0]));
}

static void try_response(const mats_lt_src_dst_packet_t *request)
{
    schc_message_t schc_msg = {0};
    if (extract_schc_message(request->payload, BITS_TO_BYTES(request->pl_size_bits), &schc_msg) < 0) {
        return;
    }

    schc_message_t schc_peer = {0};

    int st = schc_accept_message(&schc_msg, &schc_peer);
    switch (st) {
        // Incoming message is a request
        case 1:
            (void)send_schc_payload(request->dst, request->src, &schc_peer);
            break;
        // Incoming message is a response
        case 2: {
            char hex[16] = {0};
            size_t n;
            if (bytes_to_hex_string(schc_msg.payload, BITS_TO_BYTES(schc_msg.pl_size_bits), hex, sizeof(hex), &n) < 0) {
                zlog_error(error_cat, "bytes_to_hex_string internal error");
                return;
            }
            zlog_info(ok_cat, "SCHC response received: req_rule_id=0x%02x, resp_rule_id=0x%02x, payload=0x%s",
                      schc_peer.rule_id, schc_msg.rule_id, hex);
            break;
        }
        default:
            zlog_error(error_cat, "Lookup table internal error");
            break;
    }
}

static void on_packet_received(mats_lt_src_dst_packet_t *pkt) {
    if (pkt->dst != NODE_ID) {
        return;
    }

    stat_inc_recv();
    try_response(pkt);
}

int main(void) {
    if (logger_init() != LOGGER_INIT_OK) {
        perror("logger_init");
        return -1;
    }

    struct pollfd fds = {
        .fd = -1,
        .events = POLLIN,
        .revents = 0,
    };

    g_fd = mats_lt_connect(SERIAL_PORT, B115200);
    if (g_fd < 0) {
        zlog_error(error_cat, "Open serial port error");
        return -1;
    }

    fds.fd = g_fd;
    set_msg_handler(on_packet_received);

    if (schc_lu_table_init() < 0) {
        zlog_error(error_cat, "SCHC lookup table init error");
        close(g_fd);
        return -1;
    }

    uint64_t push_interval_ms = PUSH_INTERVAL_S * 1000LL;
    uint64_t next_push_ms = monotonic_ms() + push_interval_ms;

    zlog_info(ok_cat, "Sensor init ok. Entering event loop...");

    while (1) {
        int timeout_ms = (int) (next_push_ms - monotonic_ms());
        if (timeout_ms > push_interval_ms) {
            timeout_ms = (int) push_interval_ms;
        }

        int ret = poll(&fds, 1, timeout_ms);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }

            zlog_error(error_cat, "Poll internal error");
            break;
        }

        if (fds.revents & POLLIN) {
            handle_receive_silent(g_fd);
        }

        if (fds.revents & (POLLERR | POLLHUP | POLLNVAL)) {
            zlog_error(error_cat, "Serial port error");
            break;
        }

        uint64_t now_ms = monotonic_ms();
        if (now_ms >= next_push_ms) {
            send_push_message();
            do {
                next_push_ms += push_interval_ms;
            } while (next_push_ms <= now_ms);
        }
    }

    close(g_fd);
    return -1;
}