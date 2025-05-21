# SNMP Agent example
cmake_minimum_required(VERSION 3.10)
project(snmp_agent)

# Add the component library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../component ${CMAKE_BINARY_DIR}/component)

# Create the main agent executable
add_executable(snmp_agent snmp_agent_example.cpp)
target_link_libraries(snmp_agent PRIVATE snmp_asio)

# Find Asio (standalone version or part of Boost)
find_package(asio CONFIG REQUIRED)
