cmake_minimum_required(VERSION 3.15)
project(disk_throughput_and_latency_test C)


set(CMAKE_C_STANDARD 11)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_MACOSX_RPATH 1)


find_package(LMDB REQUIRED)

INCLUDE_DIRECTORIES(${LMDB_INCLUDE_DIRS})



add_executable(disk_throughput_and_latency_test main.c)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE ${LMDB_LIBRARIES})
