# Firewall Configuration System

## Overview

This project consists of a server and client program that manages a firewall configuration system. The server handles requests related to firewall rules and maintains a collection of these rules along with IP addresses and ports that match each rule. The client allows users to interact with the server to add, check, delete, and display firewall rules.

## Features

- **Server Program**:
  - Runs indefinitely, listening for client requests on a specified port.
  - Supports the following operations:
    - **Add Rule**: Adds a firewall rule if valid.
    - **Check Connection**: Verifies if an IP address and port are allowed according to the rules.
    - **Delete Rule**: Removes a specified rule if it exists.
    - **List Rules**: Returns all rules and associated IP addresses and ports.
  
- **Client Program**:
  - Sends requests to the server and displays responses.
  - Commands include:
    - `A <rule>`: Add a rule.
    - `C <IPAddress> <port>`: Check if a connection is allowed.
    - `D <rule>`: Delete a specified rule.
    - `L`: List current firewall rules.

## Firewall Rule Specification

A firewall rule must adhere to the following format:
<IPAddresses> <ports>

Where:
- `<IPAddresses>` can be:
  - A single IP address (e.g., `xxx.xxx.xxx.xxx`).
  - A range of IP addresses (e.g., `IPAddress1-IPAddress2`).
  
- `<ports>` can be:
  - A single port number (0-65535).
  - A range of ports (e.g., `port1-port2`).

### Examples of Valid Rules
- `147.188.193.0-147.188.194.255 21-22`
- `147.188.192.41 443`

## Setup and Usage

### Server

## To start the server program, use the following command:

./server <port>
Replace <port> with the port number you want the server to listen on.

Client
To use the client program, execute the following commands:

## To add a rule:

./client <serverHost> <serverPort> A <rule>

## To check an IP address and port:

./client <serverHost> <serverPort> C <IPAddress> <port>

## To delete a rule:

./client <serverHost> <serverPort> D <rule>

## To list all rules:

./client <serverHost> <serverPort> L

## Compilation

To compile the source code, navigate to the project directory and run:

make

This will generate the necessary binaries for the server and client programs.
