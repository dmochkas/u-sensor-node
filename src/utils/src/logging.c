#include "logging.h"

#include <stdio.h>

zlog_category_t* stat_cat = NULL;
zlog_category_t* tx_cat = NULL;
zlog_category_t* rx_cat = NULL;
zlog_category_t* ok_cat = NULL;
zlog_category_t* error_cat = NULL;

logger_status logger_init() {
    const int rc = zlog_init(LOG_CONFIG_FILE);
    if (rc) {
        fprintf(stderr, "Config file %s is corrupt\n", LOG_CONFIG_FILE);
        return LOGGER_INIT_KO;
    }

    tx_cat = zlog_get_category("tx");
    if (!tx_cat) {
        fprintf(stderr, "Tx category init failed\n");
        zlog_fini();
        return LOGGER_INIT_KO;
    }

    rx_cat = zlog_get_category("rx");
    if (!rx_cat) {
        fprintf(stderr, "Rx category init failed\n");
        zlog_fini();
        return LOGGER_INIT_KO;
    }

    stat_cat = zlog_get_category("stat");
    if (!stat_cat) {
        fprintf(stderr, "Stat category init failed\n");
        zlog_fini();
        return LOGGER_INIT_KO;
    }

    ok_cat = zlog_get_category("ok");
    if (!ok_cat) {
        fprintf(stderr, "OK category init failed\n");
        zlog_fini();
        return LOGGER_INIT_KO;
    }

    error_cat = zlog_get_category("error");
    if (!error_cat) {
        fprintf(stderr, "Error category init failed\n");
        zlog_fini();
        return LOGGER_INIT_KO;
    }
    setvbuf(stdout, NULL, _IOLBF, 0);   // line buffering
    setvbuf(stderr, NULL, _IOLBF, 0);   // line buffering

    return LOGGER_INIT_OK;
}

logger_status logger_deinit() {
    zlog_fini();
    return LOGGER_INIT_OK;
}