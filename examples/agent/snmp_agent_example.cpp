/**
 * SNMP-ASIO Agent Example
 * 
 * This example demonstrates how to create a simple SNMP Agent using the SNMP-ASIO library.
 * The agent implements a minimal MIB with system information and responds to SNMP GET, 
 * GETNEXT, and SET requests.
 */

#include <snmp.h>
#include <asio.hpp>
#include <iostream>
#include <string>
#include <memory>
#include <csignal>
#include <map>
#include <chrono>

// Global variables for signal handling
asio::io_context* g_io_context = nullptr;
std::atomic<bool> g_running{true};

// Signal handler for clean shutdown
void signalHandler(int signal) {
    std::cout << "Received signal " << signal << ", shutting down..." << std::endl;
    g_running = false;
    if (g_io_context) {
        g_io_context->stop();
    }
}

// OID constants for standard MIB-II system group
const char* SYSNAME_OID = "1.3.6.1.2.1.1.5.0";     // SNMPv2-MIB::sysName.0
const char* SYSDESCR_OID = "1.3.6.1.2.1.1.1.0";    // SNMPv2-MIB::sysDescr.0
const char* SYSUPTIME_OID = "1.3.6.1.2.1.1.3.0";   // SNMPv2-MIB::sysUpTime.0
const char* SYSCONTACT_OID = "1.3.6.1.2.1.1.4.0";  // SNMPv2-MIB::sysContact.0
const char* SYSLOCATION_OID = "1.3.6.1.2.1.1.6.0"; // SNMPv2-MIB::sysLocation.0

/**
 * Simple MIB implementation that stores values in memory
 */
class SimpleMIB {
public:
    SimpleMIB() {
        // Initialize with default values
        mib_values[SYSNAME_OID] = "SNMP-Asio Example Device";
        mib_values[SYSDESCR_OID] = "Example SNMP Agent using SNMP-ASIO library";
        mib_values[SYSCONTACT_OID] = "admin@example.com";
        mib_values[SYSLOCATION_OID] = "Server Room";
    }

    // Get a value for an OID
    std::unique_ptr<SNMP::BER> getValue(const std::string& oid) {
        auto it = mib_values.find(oid);
        if (it != mib_values.end()) {
            return std::make_unique<SNMP::OctetStringBER>(it->second.c_str());
        } else if (oid == SYSUPTIME_OID) {
            // Special case for sysUpTime which is dynamic
            return std::make_unique<SNMP::TimeTicksBER>(getUptime());
        }
        return nullptr;
    }

    // Set a value for an OID
    bool setValue(const std::string& oid, const std::string& value) {
        // Only allow setting certain OIDs
        if (oid == SYSNAME_OID || oid == SYSCONTACT_OID || oid == SYSLOCATION_OID) {
            mib_values[oid] = value;
            std::cout << "Set " << oid << " to '" << value << "'" << std::endl;
            return true;
        }
        return false;
    }

    // Get the next OID in lexicographical order
    std::string getNextOID(const std::string& oid) {
        // Find the next OID in lexicographical order
        for (const auto& entry : mib_values) {
            if (entry.first > oid) {
                return entry.first;
            }
        }
        if (oid < SYSUPTIME_OID && SYSUPTIME_OID > oid) {
            return SYSUPTIME_OID;
        }
        return ""; // No more OIDs
    }

private:
    std::map<std::string, std::string> mib_values;
    
    // Get current uptime in hundredths of a second
    uint32_t getUptime() const {
        auto now = std::chrono::steady_clock::now();
        auto uptime = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count() / 10;
        return static_cast<uint32_t>(uptime);
    }
};

/**
 * Main SNMP Agent application class
 */
class SNMPAgentApp {
public:
    SNMPAgentApp() : io_context_() {
        // Store global reference for signal handler
        g_io_context = &io_context_;
    }

    ~SNMPAgentApp() {
        g_io_context = nullptr;
    }

    bool run() {
        try {
            // Set up signal handling for graceful shutdown
            std::signal(SIGINT, signalHandler);
            std::signal(SIGTERM, signalHandler);
            
            std::cout << "Starting SNMP Agent on port " << SNMP::Port::SNMP << std::endl;
            
            // Create the SNMP agent with io_context
            agent_ = SNMP::Agent::create(io_context_);
            
            // Set up message handler
            agent_->onMessage(
                [this](const SNMP::Message* message, const IPAddress& remote, uint16_t port) {
                    this->handleMessage(message, remote, port);
                }
            );
            
            // Set up error handler
            agent_->onError(
                [](const asio::error_code& ec) {
                    std::cerr << "SNMP error: " << ec.message() << std::endl;
                }
            );
            
            // Initialize with local IP address (bind to all interfaces)
            IPAddress localIP(0, 0, 0, 0);
            if (!agent_->initialize(localIP, SNMP::Port::SNMP)) {
                std::cerr << "Failed to initialize SNMP agent" << std::endl;
                return false;
            }
            
            // Start asynchronous operations
            if (!agent_->start()) {
                std::cerr << "Failed to start SNMP agent" << std::endl;
                return false;
            }
            
            std::cout << "SNMP Agent running. Press Ctrl+C to stop." << std::endl;
            
            // Create work guard to keep io_context alive even when no handlers are ready
            asio::executor_work_guard<asio::io_context::executor_type> work_guard(
                io_context_.get_executor());
            
            // Set up a timer to check for stop signal
            asio::steady_timer stop_timer(io_context_, std::chrono::seconds(1));
            setupStopTimer(stop_timer);
            
            // Run the io_context - this will block until io_context.stop() is called
            io_context_.run();
            
            std::cout << "SNMP Agent stopped." << std::endl;
            return true;
        }
        catch (const std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
            return false;
        }
    }

private:
    // Set up a recurring timer to check for stop signal
    void setupStopTimer(asio::steady_timer& timer) {
        timer.async_wait([this, &timer](const asio::error_code& ec) {
            if (!ec) {
                if (!g_running) {
                    // Stop the agent and io_context
                    agent_->stop();
                    io_context_.stop();
                } else {
                    // Schedule next check
                    timer.expires_at(timer.expiry() + std::chrono::seconds(1));
                    setupStopTimer(timer);
                }
            }
        });
    }

    // Handle incoming SNMP messages
    void handleMessage(const SNMP::Message* message, const IPAddress& remote, uint16_t port) {
        std::cout << "Received SNMP message from " << remote.toString() << ":" << port << std::endl;
        
        auto type = message->getType();
        auto version = message->getVersion();
        auto community = message->getCommunity();
        
        std::cout << "  Type: " << (int)type << ", Version: " << (int)version 
                  << ", Community: " << community << std::endl;
        
        // Only process if community is "public"
        if (std::string(community) != "public") {
            std::cout << "  Invalid community string, ignoring" << std::endl;
            return;
        }
        
        // Get the variable binding list from the message
        auto varbindlist = message->getVarBindList();
        
        // Create an appropriate response based on the request type
        auto response = createResponse(message, type, version, community, varbindlist);
        
        // Send the response
        if (response) {
            agent_->send(std::move(response), remote, port);
        }
    }
    
    // Create appropriate SNMP response based on the request type
    std::unique_ptr<SNMP::Message> createResponse(
        const SNMP::Message* request,
        uint8_t type,
        uint8_t version,
        const char* community,
        SNMP::VarBindList* varbindlist
    ) {
        auto response = std::make_unique<SNMP::Message>(version, community, SNMP::Type::GetResponse);
        response->setRequestID(request->getRequestID());
        
        switch (type) {
            case SNMP::Type::GetRequest:
                handleGetRequest(response.get(), varbindlist);
                break;
                
            case SNMP::Type::GetNextRequest:
                handleGetNextRequest(response.get(), varbindlist);
                break;
                
            case SNMP::Type::SetRequest:
                handleSetRequest(response.get(), varbindlist);
                break;
                
            default:
                std::cout << "  Unsupported message type" << std::endl;
                return nullptr;
        }
        
        return response;
    }
    
    // Handle GetRequest PDU
    void handleGetRequest(SNMP::Message* response, SNMP::VarBindList* varbindlist) {
        bool error = false;
        uint8_t error_index = 0;
        
        for (uint8_t i = 0; i < varbindlist->count(); ++i) {
            auto varbind = (*varbindlist)[i];
            auto name = varbind->getName();
            
            std::cout << "  GET " << name << std::endl;
            
            auto value = mib_.getValue(name);
            if (value) {
                response->add(name, value.release());
            } else {
                error = true;
                error_index = i + 1;
                response->add(name, new SNMP::NoSuchObjectBER());
            }
        }
        
        if (error) {
            response->setError(SNMP::Error::NoSuchName, error_index);
        }
    }
    
    // Handle GetNextRequest PDU
    void handleGetNextRequest(SNMP::Message* response, SNMP::VarBindList* varbindlist) {
        bool error = false;
        uint8_t error_index = 0;
        
        for (uint8_t i = 0; i < varbindlist->count(); ++i) {
            auto varbind = (*varbindlist)[i];
            auto name = varbind->getName();
            
            std::cout << "  GETNEXT " << name << std::endl;
            
            auto next_oid = mib_.getNextOID(name);
            if (!next_oid.empty()) {
                auto value = mib_.getValue(next_oid);
                if (value) {
                    response->add(next_oid.c_str(), value.release());
                    continue;
                }
            }
            
            // No more OIDs or error
            error = true;
            error_index = i + 1;
            response->add(name, new SNMP::EndOfMIBViewBER());
        }
        
        if (error) {
            response->setError(SNMP::Error::NoSuchName, error_index);
        }
    }
    
    // Handle SetRequest PDU
    void handleSetRequest(SNMP::Message* response, SNMP::VarBindList* varbindlist) {
        bool error = false;
        uint8_t error_index = 0;
        
        for (uint8_t i = 0; i < varbindlist->count(); ++i) {
            auto varbind = (*varbindlist)[i];
            auto name = varbind->getName();
            auto value_ber = varbind->getValue();
            
            std::cout << "  SET " << name << std::endl;
            
            // Only handle OctetString values for simplicity
            if (value_ber->getType() == SNMP::Type::OctetString) {
                auto string_value = static_cast<SNMP::OctetStringBER*>(value_ber)->getValue();
                
                if (mib_.setValue(name, string_value)) {
                    // Add the same value to the response
                    response->add(name, new SNMP::OctetStringBER(string_value));
                } else {
                    error = true;
                    error_index = i + 1;
                    response->add(name, new SNMP::NoSuchObjectBER());
                }
            } else {
                error = true;
                error_index = i + 1;
                // Using NoSuchObjectBER as WrongTypeBER isn't defined in the current implementation
                response->add(name, new SNMP::NoSuchObjectBER());
            }
        }
        
        if (error) {
            response->setError(SNMP::Error::BadValue, error_index);
        }
    }

    // Member variables
    asio::io_context io_context_;
    std::shared_ptr<SNMP::Agent> agent_;
    SimpleMIB mib_;
};

/**
 * Main function
 */
int main() {
    std::cout << "SNMP-ASIO Agent Example" << std::endl;
    std::cout << "=======================" << std::endl;
    
    SNMPAgentApp app;
    if (!app.run()) {
        std::cerr << "SNMP Agent failed to run" << std::endl;
        return 1;
    }
    
    return 0;
}
