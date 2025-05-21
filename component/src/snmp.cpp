#include "snmp.h"
#include "AsioUDP.h"

namespace SNMP {

// SNMP base class constructor
SNMP::SNMP(asio::io_context& io_context, const uint16_t defaultPort)
    : _io_context(io_context), _defaultPort(defaultPort)
{
}

// Destructor
SNMP::~SNMP() {
    stop();
}

// Initialize network
bool SNMP::initialize(const IPAddress& bindAddress, uint16_t port) {
    if (port == 0) {
        port = _defaultPort;
    }
    
    // Create UDP interface
    _udp = std::make_shared<AsioUDP>(_io_context);
    
    // Set packet handler
    _udp->setPacketCallback(
        [self = shared_from_this()](const uint8_t* data, size_t length, const IPAddress& remote, uint16_t port) {
            self->handlePacket(data, length, remote, port);
        }
    );
    
    // Set error handler
    _udp->setErrorCallback(
        [self = shared_from_this()](const asio::error_code& ec) {
            if (self->_onError) {
                self->_onError(ec);
            }
        }
    );
    
    // Bind to address and port
    return _udp->begin(port);
}

// Start asynchronous operation
bool SNMP::start() {
    if (!_udp) {
        return false;
    }
    
    return _udp->startReceiving();
}

// Stop asynchronous operation
bool SNMP::stop() {
    if (!_udp) {
        return false;
    }
    
    return _udp->stopReceiving();
}

// Send an SNMP message (raw pointer)
bool SNMP::send(Message* message, const IPAddress ip, const uint16_t port) {
    if (!_udp) {
        return false;
    }
    
#if SNMP_STREAM
    _udp->beginPacket(ip, port);
    message->build(*_udp);
    return _udp->endPacket();
#else
    uint32_t length = message->getSize(true);
    std::vector<uint8_t> buffer(length);
    message->build(buffer.data());
    _udp->beginPacket(ip, port);
    _udp->write(buffer.data(), length);
    return _udp->endPacket();
#endif
}

// Send an SNMP message (unique_ptr)
bool SNMP::send(std::unique_ptr<Message> message, const IPAddress ip, const uint16_t port) {
    return send(message.get(), ip, port);
}

// Set message handler
void SNMP::onMessage(MessageHandler handler) {
    _onMessage = handler;
}

// Set error handler
void SNMP::onError(ErrorHandler handler) {
    _onError = handler;
}

// Handle received packet
void SNMP::handlePacket(const uint8_t* data, size_t length, const IPAddress& remote, uint16_t port) {
    // Parse as SNMP message
    Message* message = new Message();
    
    // Copy data to writable buffer (as parse takes a non-const buffer)
    std::vector<uint8_t> buffer(data, data + length);
    
#if SNMP_STREAM
    // SNMP_STREAM is not supported in this context
    delete message;
    if (_onError) {
        _onError(asio::error::operation_not_supported);
    }
    return;
#else
    // Parse from buffer
    message->parse(buffer.data());
    
    // Call user handler if set
    if (_onMessage) {
        _onMessage(message, remote, port);
    }
    
    delete message;
#endif
}

// Agent constructor
Agent::Agent(asio::io_context& io_context)
    : SNMP(io_context, Port::SNMP)
{
}

// Agent factory method
std::shared_ptr<Agent> Agent::create(asio::io_context& io_context) {
    return std::make_shared<Agent>(io_context);
}

// Manager constructor
Manager::Manager(asio::io_context& io_context)
    : SNMP(io_context, Port::Trap)
{
}

// Manager factory method
std::shared_ptr<Manager> Manager::create(asio::io_context& io_context) {
    return std::make_shared<Manager>(io_context);
}

} // namespace SNMP
