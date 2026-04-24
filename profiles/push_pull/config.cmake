set(SERIAL_PORT "/dev/ttyUSB0" CACHE STRING "Port to connect to")
set(NODE_ID 0x01 CACHE STRING "Node id")
set(GATEWAY_ID 0x07 CACHE STRING "Gateway id")
set(SENSOR_LINK_ID 1 CACHE STRING "Link local address of the sensor")
set(PUSH_INTERVAL_S 10 CACHE STRING "Interval of sending a push message")
set(RESP_TIMEOUT 1 CACHE STRING "Timeout for response")

set(SCHC_LU_TABLE "schc_push_pull" CACHE STRING "Look up table name")