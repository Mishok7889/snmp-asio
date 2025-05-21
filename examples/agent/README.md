# SNMP-ASIO Library
This directory contains the examples for the SNMP-ASIO library.

## SNMP Agent Example

A simple SNMP agent implementation that demonstrates the library's capabilities. 
The agent responds to SNMP GET, GETNEXT, and SET requests for a small set of MIB objects.

### Building the Example

```bash
mkdir build
cd build
cmake ..
make
```

### Running the Example

```bash
./snmp_agent
```

This will start the SNMP agent listening on port 161 (the standard SNMP port).
Press Ctrl+C to stop the agent.

### Testing the Agent

You can test the agent using any standard SNMP client, such as snmpget, snmpwalk, or snmpset:

```bash
# Get system description
snmpget -v 2c -c public localhost 1.3.6.1.2.1.1.1.0

# Walk the system MIB
snmpwalk -v 2c -c public localhost 1.3.6.1.2.1.1

# Set system contact
snmpset -v 2c -c public localhost 1.3.6.1.2.1.1.4.0 s "new_contact@example.com"
```
