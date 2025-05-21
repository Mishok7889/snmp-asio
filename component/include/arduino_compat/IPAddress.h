// IPAddress.h - Minimal implementation of IPAddress for SNMP-ASIO
#pragma once

#include <cstdint>
#include <string>
#include <array>

class IPAddress {
private:
    std::array<uint8_t, 4> bytes;

public:
    // Default constructor
    IPAddress();
    
    // Constructor from 4 octets
    IPAddress(uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet);
    
    // Constructor from uint32_t
    IPAddress(uint32_t address);
    
    // Constructor from string
    IPAddress(const char* address);
    
    // Constructor from std::string
    IPAddress(const std::string& address);
    
    // Constructor from raw byte array
    IPAddress(const uint8_t* address);
    
    // Copy constructor
    IPAddress(const IPAddress& other) = default;
    
    // Move constructor
    IPAddress(IPAddress&& other) = default;
    
    // Assignment operators
    IPAddress& operator=(const IPAddress& other) = default;
    IPAddress& operator=(uint32_t address);
    IPAddress& operator=(const uint8_t* address);
    
    // Convert to uint32_t
    operator uint32_t() const;
    
    // Check if address is set
    bool isSet() const;
    
    // Boolean conversion
    operator bool() const;
    
    // Access individual octets
    uint8_t operator[](int index) const;
    uint8_t& operator[](int index);
    
    // Equality operators
    bool operator==(const IPAddress& addr) const;
    bool operator!=(const IPAddress& addr) const;
    bool operator==(uint32_t addr) const;
    bool operator!=(uint32_t addr) const;
    
    // Convert to string
    std::string toString() const;
    
    // Parse from string
    bool fromString(const char* address);
    bool fromString(const std::string& address);
    
    // Static validation method
    static bool isValid(const char* address);
    static bool isValid(const std::string& address);
    
    // Clear the address
    void clear();
    
    // Raw access to the address bytes
    uint8_t* raw_address() { return bytes.data(); }
    const uint8_t* raw_address() const { return bytes.data(); }
};

// Standard pre-defined IP addresses
extern const IPAddress INADDR_ANY;
extern const IPAddress INADDR_NONE;
