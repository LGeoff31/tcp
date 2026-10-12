# MiniTCP

A reliable, connection-oriented transport protocol built on top of UDP in C++. MiniTCP implements the core ideas of TCP from scratch (sequence numbers, acknowledgements, retransmission, and sliding-window flow control) using only POSIX sockets.

> Built to understand how reliable delivery works beneath the sockets API: what TCP actually does when packets are lost, duplicated, delayed, or reordered.

## Features

- **Reliable, in-order delivery** over an unreliable datagram channel
- **Three-way handshake** and graceful connection teardown
- **Sequence numbers and cumulative ACKs** with out-of-order buffering at the receiver
- **Adaptive retransmission timeout (RTO)** using smoothed RTT estimation (RFC 6298 style)
- **Fast retransmit** after 3 duplicate ACKs
- **Sliding-window flow control** driven by the receiver's advertised window
- **Checksum validation** to drop corrupted packets
- **Loss simulator** to inject configurable packet loss, delay, and reordering for testing

## Packet format

Every MiniTCP segment starts with a fixed 16-byte header:

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+---------------------------------------------------------------+
|                        Sequence Number                        |
+---------------------------------------------------------------+
|                    Acknowledgment Number                      |
+-------------------------------+-------------------------------+
|        Window Size            |           Checksum            |
+-------------------------------+-------------------------------+
|     Flags (SYN/ACK/FIN)       |        Payload Length         |
+-------------------------------+-------------------------------+
|                         Payload (0..N)                        |
+---------------------------------------------------------------+
```

| Field | Size | Purpose |
|-------|------|---------|
| Sequence Number | 32 bits | Byte offset of the first payload byte |
| Acknowledgment Number | 32 bits | Next byte the receiver expects |
| Window Size | 16 bits | Receiver's remaining buffer space (flow control) |
| Checksum | 16 bits | Detects corrupted segments |
| Flags | 16 bits | `SYN`, `ACK`, `FIN` |
| Payload Length | 16 bits | Bytes of data following the header |

## How it works

**Connection setup.** The client sends `SYN`, the server replies `SYN+ACK`, and the client completes with `ACK`. Initial sequence numbers are randomized.

**Sending.** The sender keeps a window of unacknowledged segments, bounded by the receiver's advertised window. Each segment has a timer. If its ACK does not arrive within the RTO, the segment is retransmitted.

**Receiving.** The receiver delivers data in order, buffers out-of-order segments, and sends cumulative ACKs. A duplicate ACK signals a gap, and three in a row trigger fast retransmit at the sender before the timer fires.

**Timeouts.** RTO is computed from a smoothed RTT and RTT variance, with exponential backoff on repeated timeouts.

**Teardown.** Either side sends `FIN`, the peer ACKs it, and both sides close after all outstanding data is delivered.

## Project structure

```
minitcp/
├── include/
│   ├── packet.h          # header layout, serialize/parse, checksum
│   ├── connection.h      # connection state machine
│   └── loss_sim.h        # packet loss / delay / reorder injection
├── src/
│   ├── packet.cpp
│   ├── connection.cpp
│   ├── sender.cpp        # window, timers, retransmission
│   ├── receiver.cpp      # reordering buffer, ACK generation
│   ├── server.cpp
│   └── client.cpp
├── tests/
│   ├── test_packet.cpp
│   ├── test_reliability.cpp
│   └── bench.cpp
├── step1/                # early TCP/UDP echo experiments
├── CMakeLists.txt
└── README.md
```

## Build

Requires a C++17 compiler and CMake 3.15+. Tested on macOS and Linux.

```bash
git clone https://github.com/lgeoff31/minitcp.git
cd minitcp
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Usage

Start the server, then send a file from the client:

```bash
# Terminal 1
./build/server 9001 --out received.bin

# Terminal 2
./build/client 127.0.0.1 9001 --file input.bin
```

Simulate a lossy network by passing a loss rate (and optional delay in ms):

```bash
./build/client 127.0.0.1 9001 --file input.bin --loss 0.10 --delay 20
```

## Testing

```bash
ctest --test-dir build --output-on-failure
```

The reliability tests transfer files across a simulated lossy link and verify the received bytes match the sent bytes exactly, under:

- random packet loss (0% to 30%)
- packet duplication
- packet reordering
- corrupted checksums
- delayed ACKs

## Results

Measured on loopback with a 10 MB transfer, 20 ms simulated one-way delay:

| Packet loss | Throughput | Retransmissions | Correct delivery |
|-------------|-----------|-----------------|------------------|
| 0% | [X] MB/s | [X] | Yes |
| 5% | [X] MB/s | [X] | Yes |
| 10% | [X] MB/s | [X] | Yes |
| 20% | [X] MB/s | [X] | Yes |

## Design decisions

- **Cumulative ACKs over selective ACKs.** Simpler to reason about and matches classic TCP. SACK is listed under future work.
- **Receiver-side reordering buffer.** Avoids retransmitting segments that already arrived out of order.
- **Fixed segment size.** Keeps the header small and avoids implementing path MTU discovery.
- **Loss simulator inside the stack.** Makes failures reproducible in tests, instead of relying on the network.

## Limitations and future work

- No congestion control (slow start, AIMD). The sender is limited only by the receiver's window.
- No selective acknowledgements (SACK).
- Single connection per socket pair, with no multiplexing.
- No encryption.

## What I learned

- Why TCP needs both a retransmission timer and fast retransmit
- How RTT estimation affects throughput under loss
- How flow control differs from congestion control
- Debugging distributed state with deterministic packet-loss tests

## License

MIT
