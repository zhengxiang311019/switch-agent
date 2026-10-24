# OT Analyzer C Module

Platform-neutral C11 library for Linux userspace and embedded integration.
Hardware-specific packet acquisition is supplied by an implementation of
`platform/hal_capture.h`; analyzer code must not call vendor SDK APIs.

Logical layers: `platform/`, `capture/`, `protocols/`, `analysis/`, `store/`,
`reporting/`, `notifications/`, and `presentation/`.

Build the currently implemented foundation with CMake from this directory.
The protocol decoder currently recognizes EtherNet/IP, PROFINET, Modbus TCP,
OPC UA TCP, and MQTT. Bounded per-direction TCP analysis and PDU reassembly are
available; mirror coverage must be declared by the host before gap confidence
can be elevated. TLS/OPC UA application payload decryption is not provided.
