// Stream.h - Minimal Stream implementation for SNMP-ASIO library
#pragma once

#include <cstdint>
#include <cstring>
#include <string>

// Forward declaration
class Print;

// Minimal Print class required for Stream
class Print {
public:
    virtual ~Print() = default;
    
    // Write a single byte - pure virtual function that derived classes must implement
    virtual size_t write(uint8_t) = 0;
    
    // Write buffer of bytes
    virtual size_t write(const uint8_t* buffer, size_t size);
    
    // Convenience methods
    size_t write(const char* str) {
        if (str == nullptr) return 0;
        return write((const uint8_t*)str, strlen(str));
    }
    
    size_t write(const char* buffer, size_t size) {
        return write((const uint8_t*)buffer, size);
    }
    
    // Default implementation of how many bytes can be written at once
    virtual int availableForWrite() { return 0; }
    
    // Flush the output - nothing by default
    virtual void flush() { }
    
protected:
    void setWriteError(int err = 1) { write_error = err; }
    
private:
    int write_error = 0;
};

// Stream class - base class for character and binary streams
class Stream : public Print {
public:
    Stream() : _timeout(1000), _startMillis(0) {}
    virtual ~Stream() = default;
    
    // These are pure virtual functions that derived classes must implement
    virtual int available() = 0;      // Returns number of bytes available to read
    virtual int read() = 0;           // Read a single byte, returns -1 if no data available
    virtual int peek() = 0;           // Returns next byte without consuming it
    
    // Set timeout for read operations
    void setTimeout(unsigned long timeout) { _timeout = timeout; }
    unsigned long getTimeout() const { return _timeout; }
    
    // Read multiple bytes into buffer, returns number of bytes read
    virtual size_t readBytes(uint8_t* buffer, size_t length);
    
    // Convenience method to read into char buffer
    size_t readBytes(char* buffer, size_t length) {
        return readBytes(reinterpret_cast<uint8_t*>(buffer), length);
    }
    
protected:
    // Utility methods for derived classes
    int timedRead();      // Read a byte with timeout
    int timedPeek();      // Peek at a byte with timeout
    
    unsigned long _timeout;      // Timeout in milliseconds
    unsigned long _startMillis;  // Used for timeout tracking
    
    // Time function - to be implemented by the actual class
    virtual unsigned long millis() const = 0;
};
