#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace mt
{

    constexpr size_t HEADER_SIZE = 16;
    constexpr size_t MAX_PAYLOAD = 1200; // keeps packets under a typical 1500-byte MTU
    constexpr uint8_t VERSION = 1;

    enum Flags : uint8_t
    {
        SYN = 0x01,
        ACK = 0x02,
        FIN = 0x04,
        RST = 0x08,
    };

    struct Packet
    {
        uint32_t seq = 0;
        uint32_t ack = 0;
        uint16_t window = 0;
        uint8_t flags = 0;
        std::vector<uint8_t> payload;

        bool has(uint8_t f) const { return (flags & f) != 0; }
    };

    // 16-bit one's complement Internet checksum.
    uint16_t checksum(const uint8_t *data, size_t len);

    // Packet -> wire bytes (header + payload, checksum filled in).
    std::vector<uint8_t> serialize(const Packet &p);

    // Wire bytes -> Packet. Returns nullopt if the bytes are malformed or corrupted.
    std::optional<Packet> parse(const uint8_t *data, size_t len);

    // Human-readable summary for logging, e.g. "[SYN|ACK] seq=1 ack=5 win=8 len=0"
    std::string describe(const Packet &p);

} // namespace mt