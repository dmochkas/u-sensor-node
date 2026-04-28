#define _GNU_SOURCE

#include "core.h"

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include "utils.h"
#include "io.h"

#define RECV_BUF_SIZE MATS_LT_MAX_RESP_SIZE
#define SEND_BUF_SIZE MATS_LT_MAX_RESP_SIZE

#define ENOUGH_STRING_SIZE 100

static uint8_t recv_mem[RECV_BUF_SIZE] = {0};
static buffer_t recv_buf = {
        RECV_BUF_SIZE,
        recv_mem
};
static uint8_t send_mem[SEND_BUF_SIZE] = {0};
static buffer_t send_buf = {
        SEND_BUF_SIZE,
        send_mem
};
static mats_lt_src_dst_packet_t staging_packet = {0};

static msg_handler_t msg_handler = NULL;

static int mats_lt_compute_crc(const char* cmd, char* crc_out) {
    int ret = -1;
    uint16_t crc = 0xFFFF;

    if (cmd[0] != '*') {
        goto finish;
    }
    const char* cmd_body = cmd + 1;

    for (const unsigned char* p = (const unsigned char*)cmd_body; *p != '\0' && *p != '!'; ++p) {
        crc ^= (uint16_t)(*p) << 8;
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x8000) {
                crc = (uint16_t)((crc << 1) ^ 0x1021);
            } else {
                crc <<= 1;
            }
        }
    }

    if (snprintf(crc_out, 5, "%04X", crc) != 4) {
        goto finish;
    }

    ret = 0;

    finish:
    return ret;
}

static int mats_lt_add_crc(char* cmd) {
    int ret = -1;
    size_t cmd_len = strlen(cmd);
    if (cmd_len == 0) {
        goto finish;
    }
    if (mats_lt_compute_crc(cmd, cmd + cmd_len + 1) < 0) {
        goto finish;
    }
    cmd[cmd_len] = '!';
    strcat(cmd, "\r");
    ret = 1;

    finish:
    return ret;
}

static int mats_lt_send_cmd(int fd, const char* cmd, const char* expected_regex, ...) {
    const size_t cmd_len = strlen(cmd);
    if (write(fd, cmd, cmd_len) != cmd_len) {
        return -1;
    }
    tcdrain(fd);
    if (expected_regex == NULL) {
        return 1;
    }

    usleep(100 * 1000);
    char resp[MATS_LT_MAX_RESP_SIZE];
    ssize_t n = io_read_until_match(fd, &(buffer_t){MATS_LT_MAX_RESP_SIZE - 1, (uint8_t*) resp}, "[\r\n]");
    if (n < 0) {
        return -1;
    }
    resp[n] = '\0';

    va_list args;
    va_start(args, expected_regex);
    if (re_pattern_extract_groups_va(resp, expected_regex, args) < 0) {
        va_end(args);
        return -1;
    }

    va_end(args);
    return 1;
}

static int mats_lt_restart(int fd) {
    int ret = -1;

    if (mats_lt_send_cmd(fd, MATS_LT_RESTART_COMMAND, NULL) < 0) {
        goto finish;
    }

    usleep(MATS_LT_RESTART_SLEEP_TIME_MS * 1000);

    char resp[MATS_LT_MAX_RESP_SIZE];
    if (read(fd, (uint8_t[1]){}, 1) != 1) {
        goto finish;
    }
    ssize_t n = io_read_until_match(fd, &(buffer_t){MATS_LT_MAX_RESP_SIZE - 1, (uint8_t*) resp}, "[\r\n]");
    if (n < 0) {
        goto finish;
    }
    resp[n] = '\0';
    if (regex_match(resp, MATS_LT_RESTART_RESPONSE_REGEX) != 0) {
        // TODO: Error invalid response
        goto finish;
    }
    // TODO: Log response
    ret = 1;

    finish:
    return ret;
}


static int mats_lt_set_protocol(int fd, mats_lt_protocol_cfg_t p) {
    int ret = -1;
    int n = -1;
    const char* protocol_val = p == TDMA ? MATS_LT_PROTOCOL_TDMA : MATS_LT_PROTOCOL_SRC_DEST;

    {
        char get_protocol_cmd[ENOUGH_STRING_SIZE];
        n = sprintf(get_protocol_cmd, MATS_LT_GET_COMMAND_FMT, MATS_LT_PROTOCOL_PARAM_ADDR);
        if (n < 0) {
            goto finish;
        }
        if (mats_lt_add_crc(get_protocol_cmd) < 0) {
            goto finish;
        }
        char this_protocol[ENOUGH_STRING_SIZE];
        char* this_protocol_ptr = this_protocol;
        param_t protocol_param = AS_STRING_PARAM(this_protocol_ptr, ENOUGH_STRING_SIZE);
        if (mats_lt_send_cmd(fd, get_protocol_cmd, MATS_LT_GET_RESPONSE_REGEX, &protocol_param) < 0) {
            goto finish;
        }
        if (strcmp(this_protocol, protocol_val) == 0) {
            ret = 0;
            goto finish;
        }
    }

    char set_protocol_cmd[ENOUGH_STRING_SIZE];
    n = snprintf(set_protocol_cmd, sizeof(set_protocol_cmd),
                 MATS_LT_SET_COMMAND_FMT,
                 MATS_LT_PROTOCOL_PARAM_ADDR,
                 protocol_val);
    if (n < 0 || (size_t)n >= sizeof(set_protocol_cmd)) {
        goto finish;
    }

    if (mats_lt_add_crc(set_protocol_cmd) < 0) {
        goto finish;
    }

    if (mats_lt_send_cmd(fd, set_protocol_cmd, MATS_LT_OK_RESPONSE_REGEX) < 0) {
        goto finish;
    }

    ret = 1;

    finish:
    return ret;
}

static int open_serial_port(const char *port, int baudrate) {
    int fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd == -1) {
        return -1;
    }

    struct termios options;
    tcgetattr(fd, &options);

    cfsetispeed(&options, baudrate);
    cfsetospeed(&options, baudrate);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_oflag &= ~OPOST;

    tcsetattr(fd, TCSANOW, &options);
    return fd;
}

static int mats_lt_src_dst_encode(const mats_lt_src_dst_packet_t* pkt, writer_t* w) {
    char* cmd_start = (char*) w->buffer->value + w->pos;
    uint8_t pl_mode;
    switch (pkt->pl_size_bits) {
        case 8:
            pl_mode = 0;
            break;
        case 12:
            pl_mode = 1;
            break;
        case 16:
            pl_mode = 2;
            break;
        case 32:
            pl_mode = 3;
            break;
        default:
            return -1;
    }
    size_t n;
    char payload_hex[ENOUGH_STRING_SIZE];
    if (bytes_to_hex_string(pkt->payload, BITS_TO_BYTES(pkt->pl_size_bits), payload_hex, ENOUGH_STRING_SIZE, &n) < 0) {
        return -1;
    }

    if (snprintf(cmd_start, w->buffer->length - w->pos, MATS_LT_SEND_SRC_DST_PKT_FMT,
                 pkt->src,
                 pkt->dst,
                 pl_mode,
                 payload_hex) < 0) {
        return -1;
    }
    if (mats_lt_add_crc(cmd_start) < 0) {
        return -1;
    }
    n = strlen(cmd_start);
    w->pos += n;

    return 1;
}

static int mats_lt_src_dst_decode(reader_t* r, mats_lt_src_dst_packet_t* pkt) {
    if (r == NULL || pkt == NULL) {
        return -1;
    }

    uint8_t* start = r->buffer->value + r->pos;
    uint8_t* end = memchr(start, '\r', r->buffer->length - r->pos);
    if (end == NULL) {
        return 0;
    }
    end += 1;
    size_t msg_len = (size_t) (end - start);
    char msg_str[msg_len + 1];
    memcpy(msg_str, r->buffer->value + r->pos, msg_len);
    msg_str[msg_len] = '\0';
    param_t src_param = AS_UINT8_PARAM(pkt->src);
    param_t dst_param = AS_UINT8_PARAM(pkt->dst);
    uint8_t pl_size = 0;
    param_t pl_size_param = AS_UINT8_PARAM(pl_size);
    char pl_hex[ENOUGH_STRING_SIZE];
    char* pl_ptr = pl_hex;
    param_t pl_param = AS_STRING_PARAM(pl_ptr, ENOUGH_STRING_SIZE);
    char crc[MATS_LT_CRC_SIZE + 1];
    char* crc_ptr = crc;
    param_t crc_param = AS_STRING_PARAM(crc_ptr, MATS_LT_CRC_SIZE + 1);
    if (re_pattern_extract_groups(msg_str, MATS_LT_RECV_SRC_DST_PKT_REGEX,
                                  &src_param,
                                  &dst_param,
                                  &pl_size_param,
                                  &pl_param,
                                  &crc_param) < 0) {
        return -1;
    }
    char actual_crc[MATS_LT_CRC_SIZE + 1];
    if (mats_lt_compute_crc(msg_str, actual_crc) < 0 || strcmp(crc, actual_crc) != 0) {
        return -1;
    }
    switch (pl_size) {
        case 0:
            pkt->pl_size_bits = 8;
            break;
        case 1:
            pkt->pl_size_bits = 12;
            break;
        case 2:
            pkt->pl_size_bits = 16;
            break;
        // Not supported in our version
        case 3:
            pkt->pl_size_bits = 32;
            break;
        default:
            return -1;
    }
    size_t pl_size_bytes;
    if (hex_string_to_bytes(pl_hex, pkt->payload, MATS_LT_MAX_PL_SIZE, &pl_size_bytes) < 0) {
        return -1;
    }
    if (pl_size_bytes != BITS_TO_BYTES(pkt->pl_size_bits)) {
        return -1;
    }

    r->pos += msg_len;

    return 1;
}

static int mats_lt_handle_ack(int fd) {
    int ret = -1;

    static const size_t ack_len = 6;
    char send_ack[ack_len + 1];
    ssize_t n = read(fd, send_ack, ack_len);
    if (n != ack_len) {
        ret = -1;
        goto finish;
    }
    send_ack[ack_len] = '\0';

    if (regex_match(send_ack, MATS_LT_SENT_RESPONSE_REGEX) != 0) {
        ret = -1;
        goto finish;
    }
    ret = 1;

    finish:
    return ret;
}

int mats_lt_connect(const char *port, int baudrate) {
    int res = -1;
    int fd = open_serial_port(port, baudrate);
    if (fd < 0) {
        goto finish;
    }

    tcflush(fd, TCIOFLUSH);

    // TODO: Set init config params
    int set_st = 0;
    if ((set_st = mats_lt_set_protocol(fd, SRC_DEST)) < 0) {
        goto finish;
    }

    if (set_st != 0 && mats_lt_restart(fd) < 0) {
        res = -1;
        goto finish;
    }
    res = 1;

    finish:
    if (res < 0 && fd >= 0) {
        close(fd);
        fd = -1;
    }

    return fd;
}

void set_msg_handler(msg_handler_t h) {
    msg_handler = h;
}

void handle_receive_silent(int fd) {
    ssize_t n = read(fd, recv_buf.value, recv_buf.length);
    if (n > 0) {
        reader_t r = {
                &recv_buf,
                0
        };
        if (mats_lt_src_dst_decode(&r, &staging_packet) < 0) {
            // TODO: Log error
            return;
        }

        if (msg_handler != NULL) {
            msg_handler(&staging_packet);
        }
    }
}

int trigger_send(int fd, const mats_lt_src_dst_packet_t * p) {
    int ret = -1;

    writer_t w = {
        &send_buf,
        0
    };
    if (mats_lt_src_dst_encode(p, &w) < 0) {
        goto finish;
    }

    printf("Wrote %zu bytes\n", w.pos);

    if (write(fd, send_buf.value, w.pos) < 0) {
        // TODO: error
        goto finish;
    }

    sleep(1);
    if (mats_lt_handle_ack(fd) < 0) {
        // TODO: Error
        goto finish;
    }
    ret = 1;



    finish:
    return ret;
}
