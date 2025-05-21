// AsioUDP.cpp - ASIO-based implementation of UDP for SNMP-ASIO library
#include "AsioUDP.h"
#include <chrono>

// Constructor
AsioUDP::AsioUDP(asio::io_context& io_context)
    : io_context_(io_context),
      socket_(io_context),
      rx_buffer_(1500), // Default MTU size
      tx_buffer_(1500)  // Default MTU size
{
}

// Destructor
AsioUDP::~AsioUDP() {
    stop();
}

// Begin listening on specified port
uint8_t AsioUDP::begin(uint16_t port) {
    // Close socket if already open
    asio::error_code ec;
    if (socket_.is_open()) {
        socket_.close(ec);
        if (ec) {
            return 0; // Failure
        }
    }
    
    // Open and bind the socket
    socket_.open(asio::ip::udp::v4(), ec);
    if (ec) {
        if (error_callback_) {
            error_callback_(ec);
        }
        return 0; // Failure
    }
    
    socket_.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), port), ec);
    if (ec) {
        if (error_callback_) {
            error_callback_(ec);
        }
        return 0; // Failure
    }
    
    // Start receiving packets if callback is set
    if (packet_callback_) {
        startReceiving();
    }
    
    return 1; // Success
}

// Begin multicast listening
uint8_t AsioUDP::beginMulticast(const IPAddress& addr, uint16_t port) {
    // Close socket if already open
    asio::error_code ec;
    if (socket_.is_open()) {
        socket_.close(ec);
        if (ec) {
            return 0; // Failure
        }
    }
    
    // Open and bind the socket
    socket_.open(asio::ip::udp::v4(), ec);
    if (ec) {
        if (error_callback_) {
            error_callback_(ec);
        }
        return 0; // Failure
    }
    
    socket_.set_option(asio::ip::udp::socket::reuse_address(true), ec);
    if (ec) {
        if (error_callback_) {
            error_callback_(ec);
        }
        return 0; // Failure
    }
    
    socket_.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), port), ec);
    if (ec) {
        if (error_callback_) {
            error_callback_(ec);
        }
        return 0; // Failure
    }
    
    // Join multicast group
    asio::ip::address_v4 multicast_addr(static_cast<uint32_t>(addr));
    socket_.set_option(asio::ip::multicast::join_group(multicast_addr), ec);
    if (ec) {
        if (error_callback_) {
            error_callback_(ec);
        }
        return 0; // Failure
    }
    
    // Start receiving packets if callback is set
    if (packet_callback_) {
        startReceiving();
    }
    
    return 1; // Success
}

// Stop/close the socket
void AsioUDP::stop() {
    stopReceiving();
    
    if (socket_.is_open()) {
        asio::error_code ec;
        socket_.close(ec);
        // Ignore any errors on closing
    }
}

// Begin packet to IP address
int AsioUDP::beginPacket(const IPAddress& ip, uint16_t port) {
    tx_endpoint_ = asio::ip::udp::endpoint(
        asio::ip::address_v4(static_cast<uint32_t>(ip)),
        port
    );
    tx_buffer_.clear();
    return 1; // Success
}

// Begin packet to hostname
int AsioUDP::beginPacket(const char* host, uint16_t port) {
    asio::error_code ec;
    asio::ip::udp::resolver resolver(io_context_);
    auto endpoints = resolver.resolve(host, std::to_string(port), ec);
    if (ec) {
        if (error_callback_) {
            error_callback_(ec);
        }
        return 0; // Failure
    }
    
    tx_endpoint_ = *endpoints.begin();
    tx_buffer_.clear();
    return 1; // Success
}

// End packet and send it
int AsioUDP::endPacket() {
    if (!socket_.is_open()) {
        return 0;
    }
    
    // Send the packet
    asio::error_code ec;
    auto bytes_sent = socket_.send_to(
        asio::buffer(tx_buffer_.data(), tx_buffer_.size()),
        tx_endpoint_,
        0, // flags
        ec
    );
    
    if (ec) {
        if (error_callback_) {
            error_callback_(ec);
        }
        return 0; // Failure
    }
    
    return (bytes_sent == tx_buffer_.size()) ? 1 : 0;
}

// Parse next available packet - still available for backward compatibility
int AsioUDP::parsePacket() {
    // This is now a non-blocking check 
    return rx_available_;
}

// Get remote IP address
IPAddress AsioUDP::remoteIP() {
    return IPAddress(remote_endpoint_.address().to_v4().to_uint());
}

// Get remote port
uint16_t AsioUDP::remotePort() {
    return remote_endpoint_.port();
}

// Stream implementation - available
int AsioUDP::available() {
    return rx_available_;
}

// Stream implementation - read
int AsioUDP::read() {
    if (rx_available_ > 0) {
        uint8_t byte = rx_buffer_[rx_pos_++];
        rx_available_--;
        return byte;
    }
    return -1; // No data available
}

// Stream implementation - peek
int AsioUDP::peek() {
    if (rx_available_ > 0) {
        return rx_buffer_[rx_pos_];
    }
    return -1; // No data available
}

// Stream implementation - write single byte
size_t AsioUDP::write(uint8_t byte) {
    tx_buffer_.push_back(byte);
    return 1;
}

// Stream implementation - write buffer
size_t AsioUDP::write(const uint8_t* buffer, size_t size) {
    tx_buffer_.insert(tx_buffer_.end(), buffer, buffer + size);
    return size;
}

// Stream implementation - flush
void AsioUDP::flush() {
    // Nothing to do for UDP
}

// Implementation of millis()
unsigned long AsioUDP::millis() const {
    // Get current time since epoch
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch());
    return static_cast<unsigned long>(duration.count());
}

// Set callback for packet reception
void AsioUDP::setPacketCallback(PacketReceivedCallback callback) {
    packet_callback_ = callback;
    
    // If socket is already open, start receiving
    if (packet_callback_ && socket_.is_open() && !receiving_) {
        startReceiving();
    }
}

// Set callback for error handling
void AsioUDP::setErrorCallback(ErrorCallback callback) {
    error_callback_ = callback;
}

// Start receiving packets
bool AsioUDP::startReceiving() {
    if (!socket_.is_open() || !packet_callback_) {
        return false;
    }
    
    if (!receiving_) {
        receiving_ = true;
        startReceive();
    }
    
    return true;
}

// Stop receiving packets
bool AsioUDP::stopReceiving() {
    receiving_ = false;
    return true;
}

// Start asynchronous receive
void AsioUDP::startReceive() {
    if (!socket_.is_open() || !receiving_) {
        return;
    }
    
    auto self = shared_from_this();
    socket_.async_receive_from(
        asio::buffer(rx_buffer_),
        remote_endpoint_,
        [self](const asio::error_code& error, size_t bytes_transferred) {
            self->handleReceive(error, bytes_transferred);
        }
    );
}

// Handle completion of an asynchronous receive
void AsioUDP::handleReceive(const asio::error_code& error, size_t bytes_transferred) {
    if (!error && bytes_transferred > 0) {
        // Reset buffer positions
        rx_pos_ = 0;
        rx_available_ = bytes_transferred;
        
        // Call user callback if set
        if (packet_callback_) {
            packet_callback_(
                rx_buffer_.data(),
                bytes_transferred,
                IPAddress(remote_endpoint_.address().to_v4().to_uint()),
                remote_endpoint_.port()
            );
        }
    }
    else if (error && error != asio::error::operation_aborted) {
        // Call error callback if set
        if (error_callback_) {
            error_callback_(error);
        }
    }
    
    // Queue another receive if still receiving
    if (receiving_) {
        startReceive();
    }
}
