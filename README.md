## Acknowledgements

This library is a port of the [SNMP Manager For Arduino](https://github.com/patricklaf/SNMP) authored by Patrick Lafarguette, which itself was inspired by Martin Rowan's work. The original Arduino library has been tested on several boards including ESP32 and STM32 architectures.## Original Library Features

The SNMP-ASIO library is based on the [Arduino SNMP library by Patrick Lafarguette](https://github.com/patricklaf/SNMP) and supports:

### SNMP Protocol Versions
- SNMPv1
- SNMPv2c

### SNMP Message Types
- GETREQUEST
- GETNEXTREQUEST
- GETRESPONSE
- SETREQUEST
- TRAP
- GETBULKREQUEST
- INFORMREQUEST
- SNMPV2TRAP

### SNMP Object Types
- Boolean
- Integer
- OctetString
- Null
- ObjectIdentifier
- Sequence
- IPAddress
- Counter32
- Gauge32
- TimeTicks
- Opaque
- Counter64
- Float
- OpaqueFloat# SNMP-ASIO

SNMP implementation ported from Arduino to ASIO with modern C++ support. This library provides a cross-platform SNMP agent implementation that uses ASIO for network communication.

## Overview

This library is a port of the [Arduino SNMP library by Patrick Lafarguette](https://github.com/patricklaf/SNMP) to ASIO networking, allowing it to be used in desktop applications and embedded systems that support C++17/20. The code has been designed with the following features:

- No exceptions (suitable for embedded applications)
- Modern C++ (C++17/20)
- Asynchronous I/O using ASIO
- Support for SNMPv1 and SNMPv2c
- GET, GETNEXT, and SET operations

## Dependencies

- ASIO (standalone, header-only version)
- C++17 or C++20 compiler

## ESP-IDF Component Configuration

To use this library with ESP-IDF, add the following to your project's `idf_component.yml`:

```yaml
dependencies:
  espressif/asio: "1.14.1~3"
  snmp-asio:
    git: https://github.com/Mishok7889/snmp-asio.git
    version: "main"
    path: component

  # If you need to compile with specific options
  build:
    cmake_options:
      - "-DCMAKE_CXX_STANDARD=20"
```

## Basic Usage

```cpp
#include <snmp.h>
#include <asio.hpp>

void setupSNMPAgent() {
    asio::io_context io_context;
    
    // Create the SNMP agent
    auto agent = SNMP::Agent::create(io_context);
    
    // Initialize (bind to all interfaces on the standard SNMP port)
    IPAddress bindAddress(0, 0, 0, 0);
    agent->initialize(bindAddress, SNMP::Port::SNMP);
    
    // Set message handler
    agent->onMessage([](const SNMP::Message* message, const IPAddress& remote, uint16_t port) {
        // Handle SNMP requests here
    });
    
    // Start the agent
    agent->start();
    
    // Run the io_context in a separate thread
    std::thread io_thread([&io_context]() {
        io_context.run();
    });
    
    // ... application code ...
    
    // Clean shutdown
    agent->stop();
    io_context.stop();
    io_thread.join();
}
```