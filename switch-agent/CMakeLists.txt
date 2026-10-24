cmake_minimum_required(VERSION 3.16)
project(ot_analyzer C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(ot_analyzer STATIC
    platform/hal_capture.c
    capture/health.c
    capture/packet_queue.c
    protocols/ethernet_ip.c
    protocols/packet_view.c
    protocols/profinet.c
    protocols/modbus_tcp.c
    protocols/opcua.c
    protocols/mqtt.c
    platform/timebase.c
    analysis/flow_catalog.c
    analysis/traffic_aggregate.c
    analysis/observation_health.c
    analysis/engine.c
    presentation/monitoring_status.c
    store/retention.c
    analysis/quality.c
    analysis/tcp_analysis.c
    module_api.c
)
target_include_directories(ot_analyzer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(ot_analyzer PRIVATE -Wall -Wextra -Wpedantic)
