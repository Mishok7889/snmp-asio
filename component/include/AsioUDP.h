// AsioUDP.h - ASIO-based implementation of UDP for SNMP-ASIO library
#pragma once

#include "arduino_compat/UDP.h"
#include <asio.hpp>
#include <memory>
#include <vector>
#include <queue>
#include <functional>

// Forward declaration for callback type
using PacketReceivedCallback = std::function<void(const uint8_t*, size_t, const IPAddress&, uint16_t)>;
using ErrorCallback = std::function<void(const asio::error_code&)>;

// AsioUDP - Concrete implementation of UDP using ASIO with event-driven model
class AsioUDP : public UDP, public std::enable_shared_from_this<AsioUDP> {
public:
    // Constructor accepts external io_context
    AsioUDP(asio::io_context& io_context);
    virtual ~AsioUDP();
    
    // UDP implementation
    uint8_t begin(uint16_t port) override;
    uint8_t beginMulticast(const IPAddress& addr, uint16_t port) override;
    void stop() override;
    int beginPacket(const IPAddress& ip, uint16_t port) override;
    int beginPacket(const char* host, uint16_t port) override;
    int endPacket() override;
    int parsePacket() override;
    IPAddress remoteIP() override;
    uint16_t remotePort() override;
    
    // Stream implementation
    int available() override;
    int read() override;
    int peek() override;
    size_t write(uint8_t) override;
    size_t write(const uint8_t* buffer, size_t size) override;
    void flush() override;
    
    // New event-driven methods
    void setPacketCallback(PacketReceivedCallback callback);
    void setErrorCallback(ErrorCallback callback);
    bool startReceiving();
    bool stopReceiving();
    
protected:
    // Implementation of the Stream::millis() method
    unsigned long millis() const override;
    
private:
    // ASIO-specific members
    asio::io_context& io_context_;
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint remote_endpoint_;
    std::vector<uint8_t> rx_buffer_;  // Receive buffer
    std::vector<uint8_t> tx_buffer_;  // Transmit buffer
    
    // Buffer management
    size_t rx_pos_ = 0;        // Current read position in rx_buffer_
    size_t rx_available_ = 0;  // Number of bytes available to read
    
    // Destination for outgoing packets
    asio::ip::udp::endpoint tx_endpoint_;
    
    // Callbacks for received packets and errors
    PacketReceivedCallback packet_callback_;
    ErrorCallback error_callback_;
    
    // Flag to track if we're receiving
    bool receiving_ = false;
    
    // Start an asynchronous receive
    void startReceive();
    
    // Handle completion of an asynchronous receive
    void handleReceive(const asio::error_code& error, size_t bytes_transferred);
};
