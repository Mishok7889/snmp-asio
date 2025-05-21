// UDP.h - Minimal UDP implementation for SNMP-ASIO library
#pragma once

#include "Stream.h"
#include "IPAddress.h"

// UDP class - Abstract class for UDP communication
class UDP : public Stream {
public:
    virtual ~UDP() = default;
    
    // Initialize, start listening on specified port
    // Returns 1 if successful, 0 if there are no sockets available
    virtual uint8_t begin(uint16_t port) = 0;
    
    // Optional: Initialize multicast support
    virtual uint8_t beginMulticast(const IPAddress& addr, uint16_t port) { return 0; }
    
    // Close the socket
    virtual void stop() = 0;
    
    // Start building up a packet to send to the remote host specific in ip and port
    // Returns 1 if successful, 0 if there was a problem
    virtual int beginPacket(const IPAddress& ip, uint16_t port) = 0;
    
    // Start building up a packet to send to the remote host specific in host and port
    // Returns 1 if successful, 0 if there was a problem resolving the hostname
    virtual int beginPacket(const char* host, uint16_t port) = 0;
    
    // Finish off this packet and send it
    // Returns 1 if the packet was sent successfully, 0 if there was an error
    virtual int endPacket() = 0;
    
    // Start processing the next available incoming packet
    // Returns the size of the packet in bytes, or 0 if no packets are available
    virtual int parsePacket() = 0;
    
    // Return the IP address of the host who sent the current incoming packet
    virtual IPAddress remoteIP() = 0;
    
    // Return the port of the host who sent the current incoming packet
    virtual uint16_t remotePort() = 0;
    
protected:
    // Helper for derived classes
    const uint8_t* rawIPAddress(const IPAddress& addr) const {
        return reinterpret_cast<const uint8_t*>(&addr);
    }
};
