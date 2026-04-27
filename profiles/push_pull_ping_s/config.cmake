set(SERIAL_PORT "/dev/ttyUSB0" CACHE STRING "Port to connect to")
set(NODE_ID 0x01 CACHE STRING "Node id")
set(GATEWAY_ID 0x07 CACHE STRING "Gateway id")
set(SENSOR_LINK_ID 0 CACHE STRING "Link local address of the sensor")
set(PUSH_INTERVAL_S 10 CACHE STRING "Interval of sending a push message")
set(RESP_TIMEOUT 1 CACHE STRING "Timeout for response")
set(LOG_CONFIG_FILE "${CMAKE_CURRENT_LIST_DIR}/log.config" CACHE STRING "Log config file")

set(SCHC_LU_TABLE "schc_push_pull_ping_s" CACHE STRING "Look up table name")
# Means 4 MSB to encode instance ID, then create 2 instances of the SCHC lookup table.
set(SCHC_RULE_GEN_STRATEGY "4-2-instances" CACHE STRING "SCHC rule generation strategy")