# Protocol Decoders

Portable protocol decoders consume bounded frame records and do not depend on
the platform HAL implementation.
# Protocol decoder scope

The portable decoders classify visible EtherNet/IP, PROFINET, Modbus TCP,
OPC UA TCP, and MQTT traffic. They use bounded field parsing inspired by
Wireshark dissector behavior; they are independent implementations and do not
link Wireshark code. MQTT TLS payloads and OPC UA signed/encrypted application
content are not decrypted. The packet metadata parser handles up to four VLAN
tags, IPv4/IPv6 and bounded IPv6 extension chains. The TCP analyzer keeps
bounded per-direction sequence state and a limited out-of-order reassembly
window for supported application PDU framing. IP fragment reassembly,
unbounded TCP streams, and vendor-private schemas remain outside scope. Any
capture gap under incomplete mirror coverage remains uncertain. Application
payload bytes are transient in memory only and are excluded from diagnostic
snapshots and persistence.
