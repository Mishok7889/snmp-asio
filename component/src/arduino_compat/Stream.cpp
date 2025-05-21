// Stream.cpp - Minimal Stream implementation for SNMP-ASIO library
#include "arduino_compat/Stream.h"

// Print class implementation
size_t Print::write(const uint8_t* buffer, size_t size) {
    size_t n = 0;
    while (size--) {
        size_t ret = write(*buffer++);
        if (ret == 0) break;
        n += ret;
    }
    return n;
}

// Stream class implementation

// Read a byte with timeout
int Stream::timedRead() {
    _startMillis = millis();
    do {
        int c = read();
        if (c >= 0) return c;
        
        // Handle timeout
        if (_timeout == 0) return -1;
        
        // Yield to other tasks if needed - can be a no-op in a single-threaded context
    } while (millis() - _startMillis < _timeout);
    
    return -1; // Timeout
}

// Peek at a byte with timeout
int Stream::timedPeek() {
    _startMillis = millis();
    do {
        int c = peek();
        if (c >= 0) return c;
        
        // Handle timeout
        if (_timeout == 0) return -1;
        
        // Yield to other tasks if needed
    } while (millis() - _startMillis < _timeout);
    
    return -1; // Timeout
}

// Read multiple bytes into buffer with timeout
size_t Stream::readBytes(uint8_t* buffer, size_t length) {
    size_t count = 0;
    while (count < length) {
        int c = timedRead();
        if (c < 0) break;
        *buffer++ = (uint8_t)c;
        count++;
    }
    return count;
}
