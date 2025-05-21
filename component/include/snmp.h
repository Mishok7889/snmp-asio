#pragma once

#include "snmp_message.h"
#include <asio.hpp>
#include <functional>
#include <memory>
#include "arduino_compat/IPAddress.h"

// Forward declaration
class AsioUDP;

/**
 * @namespace SNMP
 * @brief %SNMP library namespace.
 */
namespace SNMP {

/**
 * @struct Port
 * @brief Helper struct to handle UDP ports.
 *
 * %SNMP uses 2 well-known UDP ports.
 *
 * - Manager uses port 161 to communicate with an agent.
 * - Agent uses port 162 to send trap messages to the manager.
 */
struct Port {
    static constexpr uint16_t SNMP = 161; /**< SNMP default UDP port. */
    static constexpr uint16_t Trap = 162; /**< SNMP default UDP port for TRAP, INFORMREQUEST and SNMPV2TRAP messages. */
};

/**
 * Forward declarations
 */
class SNMP;
class Agent;
class Manager;

/**
 * @class SNMP
 * @brief Base class for Agent and Manager.
 */
class SNMP : public std::enable_shared_from_this<SNMP> {
public:
    /**
     * @brief On message event user handler type.
     *
     * The application must define an event handler function to process incoming message.
     *
     * Example
     *
     * ```cpp
     * void onMessage(const SNMP::Message *message, const IPAddress remote, const uint16_t port) {
     *     // User code here...
     * }
     * ```
     *
     * @param message %SNMP message to process.
     * @param remote IP address of the sender.
     * @param port UDP port of the sender.
     */
    using MessageHandler = std::function<void(const Message*, const IPAddress, const uint16_t)>;
    
    /**
     * @brief Error handler type for network errors.
     */
    using ErrorHandler = std::function<void(const asio::error_code&)>;

    /**
     * @brief Destructor.
     */
    virtual ~SNMP();

    /**
     * @brief Initializes network.
     *
     * @param bindAddress Local IP address to bind to.
     * @param port UDP port to listen on.
     * @return true if success, false if failure.
     */
    bool initialize(const IPAddress& bindAddress, uint16_t port = 0);

    /**
     * @brief Starts asynchronous operation.
     * 
     * @return true if success, false if failure.
     */
    bool start();
    
    /**
     * @brief Stops asynchronous operation.
     * 
     * @return true if success, false if failure.
     */
    bool stop();

    /**
     * @brief Network write operation
     *
     * Builds message and write outgoing packet.
     *
     * @param message %SNMP message to send.
     * @param ip IP address to send to.
     * @param port UDP port to send to
     * @return true if success, false if failure.
     */
    bool send(Message* message, const IPAddress ip, const uint16_t port);
    
    /**
     * @brief Send a message using a unique_ptr
     * 
     * Builds message and write outgoing packet.
     * 
     * @param message unique_ptr to the SNMP message to send
     * @param ip IP address to send to
     * @param port UDP port to send to
     * @return true if success, false if failure
     */
    bool send(std::unique_ptr<Message> message, const IPAddress ip, const uint16_t port);

    /**
     * @brief Sets on message event user handler.
     *
     * @param handler Message handler function.
     */
    void onMessage(MessageHandler handler);
    
    /**
     * @brief Sets error handler.
     * 
     * @param handler Error handler function.
     */
    void onError(ErrorHandler handler);

protected:
    /**
     * @brief Creates an SNMP object.
     *
     * @param io_context ASIO io_context for network operations.
     * @param defaultPort Default UDP port to use if not specified in initialize().
     */
    SNMP(asio::io_context& io_context, const uint16_t defaultPort);
    
    /**
     * @brief Handle packet received from UDP.
     * 
     * Called by the UDP layer when a packet is received.
     * 
     * @param data Packet data.
     * @param length Packet length.
     * @param remote Remote IP address.
     * @param port Remote port.
     */
    void handlePacket(const uint8_t* data, size_t length, const IPAddress& remote, uint16_t port);

    /** Default UDP port. */
    uint16_t _defaultPort = Port::SNMP;
    /** ASIO io_context reference. */
    asio::io_context& _io_context;
    /** UDP client. */
    std::shared_ptr<AsioUDP> _udp;
    /** On message event user handler. */
    MessageHandler _onMessage = nullptr;
    /** Error handler. */
    ErrorHandler _onError = nullptr;
};

/**
 * @class Agent
 * @brief %SNMP agent.
 *
 * An %SNMP agent listen to UDP port Port::SNMP.
 */
class Agent: public SNMP {
public:
    /**
     * @brief Creates an %SNMP agent.
     * 
     * @param io_context ASIO io_context for network operations.
     */
    Agent(asio::io_context& io_context);
    
    /**
     * @brief Creates a shared pointer to an SNMP agent.
     * 
     * @param io_context ASIO io_context for network operations.
     * @return Shared pointer to SNMP agent.
     */
    static std::shared_ptr<Agent> create(asio::io_context& io_context);
};

/**
 * @class Manager
 * @brief %SNMP manager.
 *
 * An %SNMP manager listen to UDP port Port::Trap.
 */
class Manager: public SNMP {
public:
    /**
     * @brief Creates an %SNMP manager.
     * 
     * @param io_context ASIO io_context for network operations.
     */
    Manager(asio::io_context& io_context);
    
    /**
     * @brief Creates a shared pointer to an SNMP manager.
     * 
     * @param io_context ASIO io_context for network operations.
     * @return Shared pointer to SNMP manager.
     */
    static std::shared_ptr<Manager> create(asio::io_context& io_context);
};

} // namespace SNMP
