// arduino_compat.h - Compatibility functions for Arduino APIs
#pragma once

#include <chrono>
#include "arduino_compat/String.h"

// Provides a millis() function similar to Arduino's
// Returns the number of milliseconds since program start
inline unsigned long millis() {
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return static_cast<unsigned long>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count());
}
