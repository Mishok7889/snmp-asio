cmake_minimum_required(VERSION 3.10)
project(snmp_asio VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define source directories
set(SNMP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SNMP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Library sources - use absolute paths
set(SNMP_SOURCES
    ${SNMP_SOURCE_DIR}/ber.cpp
    ${SNMP_SOURCE_DIR}/AsioUDP.cpp
    ${SNMP_SOURCE_DIR}/snmp.cpp
    ${SNMP_SOURCE_DIR}/arduino_compat/IPAddress.cpp
    ${SNMP_SOURCE_DIR}/arduino_compat/Stream.cpp
)

# Library headers - use absolute paths
set(SNMP_HEADERS
    ${SNMP_INCLUDE_DIR}/ber.h
    ${SNMP_INCLUDE_DIR}/AsioUDP.h
    ${SNMP_INCLUDE_DIR}/arduino_compat.h
    ${SNMP_INCLUDE_DIR}/snmp.h
    ${SNMP_INCLUDE_DIR}/snmp_message.h
    ${SNMP_INCLUDE_DIR}/arduino_compat/IPAddress.h
    ${SNMP_INCLUDE_DIR}/arduino_compat/Stream.h
    ${SNMP_INCLUDE_DIR}/arduino_compat/UDP.h
    ${SNMP_INCLUDE_DIR}/arduino_compat/String.h
)

# Create the library
add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
  PRIVATE
    ${SNMP_SOURCES}
  PUBLIC
    ${SNMP_HEADERS}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${SNMP_INCLUDE_DIR}
)

find_package(asio CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC asio::asio)

# Define SNMP_STREAM to 0 to use buffer-based implementation instead of Stream
target_compile_definitions(${PROJECT_NAME} PUBLIC SNMP_STREAM=0)
