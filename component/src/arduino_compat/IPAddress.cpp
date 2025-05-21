#include "arduino_compat/IPAddress.h"
#include <sstream>

#if defined(_MSC_VER)
    // MSVC-specific secure functions
    #define SSCANF sscanf_s
#else
    // Standard C library functions for other platforms
    #define SSCANF sscanf
#endif

// Default constructor
IPAddress::IPAddress() : bytes{0, 0, 0, 0} {}

// Constructor from 4 octets
IPAddress::IPAddress(uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet) 
    : bytes{first_octet, second_octet, third_octet, fourth_octet} {}

// Constructor from uint32_t
IPAddress::IPAddress(uint32_t address) {
    bytes[0] = (address >> 24) & 0xFF;
    bytes[1] = (address >> 16) & 0xFF;
    bytes[2] = (address >> 8) & 0xFF;
    bytes[3] = address & 0xFF;
}

// Constructor from string
IPAddress::IPAddress(const char* address) {
    fromString(address);
}

// Constructor from std::string
IPAddress::IPAddress(const std::string& address) {
    fromString(address.c_str());
}

// Constructor from raw byte array
IPAddress::IPAddress(const uint8_t* address) {
    if (address) {
        bytes[0] = address[0];
        bytes[1] = address[1];
        bytes[2] = address[2];
        bytes[3] = address[3];
    }
}

// Assignment operator for uint32_t
IPAddress& IPAddress::operator=(uint32_t address) {
    bytes[0] = (address >> 24) & 0xFF;
    bytes[1] = (address >> 16) & 0xFF;
    bytes[2] = (address >> 8) & 0xFF;
    bytes[3] = address & 0xFF;
    return *this;
}

// Assignment operator for raw byte array
IPAddress& IPAddress::operator=(const uint8_t* address) {
    if (address) {
        bytes[0] = address[0];
        bytes[1] = address[1];
        bytes[2] = address[2];
        bytes[3] = address[3];
    }
    return *this;
}

// Convert to uint32_t
IPAddress::operator uint32_t() const {
    return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

// Check if address is set
bool IPAddress::isSet() const {
    return *this != IPAddress();
}

// Boolean conversion
IPAddress::operator bool() const {
    return isSet();
}

// Access individual octets
uint8_t IPAddress::operator[](int index) const {
    if (index >= 0 && index < 4) {
        return bytes[index];
    }
    return 0;
}

uint8_t& IPAddress::operator[](int index) {
    return bytes[index];
}

// Equality operators
bool IPAddress::operator==(const IPAddress& addr) const {
    return bytes == addr.bytes;
}

bool IPAddress::operator!=(const IPAddress& addr) const {
    return bytes != addr.bytes;
}

bool IPAddress::operator==(uint32_t addr) const {
    return static_cast<uint32_t>(*this) == addr;
}

bool IPAddress::operator!=(uint32_t addr) const {
    return static_cast<uint32_t>(*this) != addr;
}

// Convert to string
std::string IPAddress::toString() const {
    std::stringstream ss;
    ss << static_cast<int>(bytes[0]) << "." 
       << static_cast<int>(bytes[1]) << "." 
       << static_cast<int>(bytes[2]) << "." 
       << static_cast<int>(bytes[3]);
    return ss.str();
}

// Parse from string
bool IPAddress::fromString(const char* address) {
    unsigned int a, b, c, d;
    int result = SSCANF(address, "%u.%u.%u.%u", &a, &b, &c, &d);
    if (result == 4 && a < 256 && b < 256 && c < 256 && d < 256) {
        bytes[0] = a;
        bytes[1] = b;
        bytes[2] = c;
        bytes[3] = d;
        return true;
    }
    return false;
}

bool IPAddress::fromString(const std::string& address) {
    return fromString(address.c_str());
}

// Static validation method
bool IPAddress::isValid(const char* address) {
    IPAddress test;
    return test.fromString(address);
}

bool IPAddress::isValid(const std::string& address) {
    return isValid(address.c_str());
}

// Clear the address
void IPAddress::clear() {
    bytes = {0, 0, 0, 0};
}

// Standard pre-defined IP addresses
const IPAddress INADDR_ANY(0, 0, 0, 0);
const IPAddress INADDR_NONE(255, 255, 255, 255);
