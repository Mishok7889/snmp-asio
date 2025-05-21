// String.h - Arduino-compatible String wrapper for std::string
#pragma once

#include <string>
#include <cstring>
#include <algorithm>

class String {
private:
    std::string _str;  // Underlying std::string

public:
    // Default constructor
    String() = default;

    // Constructor with const char* - safely handles nullptr
    String(const char* cstr) {
        if (cstr) _str = cstr;
        // If nullptr, _str remains empty
    }

    // Constructor from std::string
    String(const std::string& stdStr) {
        _str = stdStr;
    }

    // Copy constructor
    String(const String& other) = default;

    // Move constructor
    String(String&& other) = default;

    // Assignment from const char* with nullptr safety
    String& operator=(const char* cstr) {
        if (cstr) _str = cstr;
        else _str.clear();  // Empty the string if nullptr
        return *this;
    }

    // Assignment from std::string
    String& operator=(const std::string& stdStr) {
        _str = stdStr;
        return *this;
    }

    // Assignment from String
    String& operator=(const String& other) = default;

    // Move assignment
    String& operator=(String&& other) = default;

    // C-string access with nullptr safety
    const char* c_str() const {
        return _str.c_str();  // std::string::c_str() is never nullptr
    }

    // Length
    size_t length() const {
        return _str.length();
    }

    // Check if empty
    bool isEmpty() const {
        return _str.empty();
    }

    // Concatenation with std::string
    bool concat(const std::string& stdStr) {
        _str += stdStr;
        return true;
    }

    // Concatenation
    bool concat(const char* cstr) {
        if (cstr) {
            _str += cstr;
            return true;
        }
        return false;
    }

    // Concatenation with String
    bool concat(const String& other) {
        _str += other._str;
        return true;
    }

    // Concatenation with integers (for uint32_t support)
    bool concat(int num) {
        _str += std::to_string(num);
        return true;
    }

    bool concat(unsigned int num) {
        _str += std::to_string(num);
        return true;
    }

    bool concat(long num) {
        _str += std::to_string(num);
        return true;
    }

    bool concat(unsigned long num) {
        _str += std::to_string(num);
        return true;
    }

    // Comparison with nullptr safety
    int compareTo(const char* cstr) const {
        if (!cstr) {
            // Arduino-style: if this string is empty, equal; 
            // if not empty, this string is greater
            return _str.empty() ? 0 : 1;
        }
        return _str.compare(cstr);
    }

    // Equality check with nullptr safety
    bool equals(const char* cstr) const {
        if (!cstr) return _str.empty();
        return _str == cstr;
    }

    // Utility: convert from std::string to String
    static String fromStdString(const std::string& stdStr) {
        String result;
        result._str = stdStr;
        return result;
    }

    // Utility: convert to std::string
    std::string toStdString() const {
        return _str;
    }

    // Indexing operators for compatibility
    char operator[](size_t index) const {
        if (index >= _str.length()) return '\0';
        return _str[index];
    }
    
    // Automatically convert to std::string when needed
    operator std::string() const {
        return _str;
    }
    
    // Allow conversions to String from string literals
    // This makes "string" + String work properly
    friend String operator+(const char* lhs, const String& rhs);
    
    // Implicit conversion to bool (empty check)
    operator bool() const {
        return !_str.empty();
    }
    
    // String addition operators
    String& operator+=(const String& rhs) {
        concat(rhs);
        return *this;
    }
    
    String& operator+=(const std::string& rhs) {
        concat(rhs);
        return *this;
    }
    
    String& operator+=(const char* rhs) {
        concat(rhs);
        return *this;
    }
    
    String& operator+=(char c) {
        _str += c;
        return *this;
    }
    
    String& operator+=(unsigned long num) {
        concat(num);
        return *this;
    }
    
    String& operator+=(long num) {
        concat(num);
        return *this;
    }
    
    String& operator+=(unsigned int num) {
        concat(num);
        return *this;
    }
    
    String& operator+=(int num) {
        concat(num);
        return *this;
    }
};

// String concatenation operators
inline String operator+(const String& lhs, const String& rhs) {
    String result(lhs);
    result.concat(rhs);
    return result;
}

inline String operator+(const String& lhs, const char* rhs) {
    String result(lhs);
    result.concat(rhs);
    return result;
}

inline String operator+(const char* lhs, const String& rhs) {
    String result(lhs);
    result.concat(rhs);
    return result;
}

inline String operator+(const String& lhs, const std::string& rhs) {
    String result(lhs);
    result.concat(rhs);
    return result;
}

inline String operator+(const std::string& lhs, const String& rhs) {
    String result(lhs);
    result.concat(rhs);
    return result;
}
