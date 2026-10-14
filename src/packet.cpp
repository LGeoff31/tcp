#include "packet.h"

namespace mt
{

    namespace
    {

        void put16(std::vector<uint8_t> &b, size_t off, uint16_t v)
        {
            b[off] = v >> 8;
            b[off + 1] = v & 0xFF;
        }

        void put32(std::vector<uint8_t> &b, size_t off, uint32_t v)
        {
            b[off] = v >> 24;
            b[off + 1] = (v >> 16) & 0xFF;
            b[off + 2] = (v >> 8) & 0xFF;
            b[off + 3] = v & 0xFF;
        }

        uint16_t get16(const uint8_t *d, size_t off)
        {
            return (uint16_t(d[off]) << 8) | d[off + 1];
        }

        uint32_t get32(const uint8_t *d, size_t off)
        {
            return (uint32_t(d[off]) << 24) | (uint32_t(d[off + 1]) << 16) |
                   (uint32_t(d[off + 2]) << 8) | uint32_t(d[off + 3]);
        }

    } // namespace

    uint16_t checksum(const uint8_t *data, size_t len)
    {
        uint32_t sum = 0;
        for (size_t i = 0; i + 1 < len; i += 2)
            sum += (uint32_t(data[i]) << 8) | data[i + 1];
        if (len % 2 == 1)
            sum += uint32_t(data[len - 1]) << 8; // pad odd byte with a zero
        while (sum >> 16)
            sum = (sum & 0xFFFF) + (sum >> 16); // fold carries back in
        return static_cast<uint16_t>(~sum);
    }

    std::vector<uint8_t> serialize(const Packet &p)
    {
        std::vector<uint8_t> out(HEADER_SIZE + p.payload.size(), 0);
        put32(out, 0, p.seq);
        put32(out, 4, p.ack);
        put16(out, 8, p.window);
        put16(out, 10, static_cast<uint16_t>(p.payload.size()));
        out[12] = p.flags;
        out[13] = VERSION;
        // bytes 14-15 (checksum) are still zero here
        for (size_t i = 0; i < p.payload.size(); ++i)
            out[HEADER_SIZE + i] = p.payload[i];

        put16(out, 14, checksum(out.data(), out.size()));
        return out;
    }

    std::optional<Packet> parse(const uint8_t *data, size_t len)
    {
        if (len < HEADER_SIZE)
            return std::nullopt;
        if (data[13] != VERSION)
            return std::nullopt;

        uint16_t payload_len = get16(data, 10);
        if (payload_len > MAX_PAYLOAD)
            return std::nullopt;
        if (len != HEADER_SIZE + payload_len)
            return std::nullopt;

        // Checksumming the whole packet *including* the stored checksum gives 0
        // if nothing was corrupted.
        if (checksum(data, len) != 0)
            return std::nullopt;

        Packet p;
        p.seq = get32(data, 0);
        p.ack = get32(data, 4);
        p.window = get16(data, 8);
        p.flags = data[12];
        p.payload.assign(data + HEADER_SIZE, data + HEADER_SIZE + payload_len);
        return p;
    }

    std::string describe(const Packet &p)
    {
        std::string f;
        auto add = [&](const char *name)
        {
            if (!f.empty())
                f += "|";
            f += name;
        };
        if (p.has(SYN))
            add("SYN");
        if (p.has(ACK))
            add("ACK");
        if (p.has(FIN))
            add("FIN");
        if (p.has(RST))
            add("RST");
        if (f.empty())
            f = "DATA";

        return "[" + f + "] seq=" + std::to_string(p.seq) +
               " ack=" + std::to_string(p.ack) +
               " win=" + std::to_string(p.window) +
               " len=" + std::to_string(p.payload.size());
    }

} // namespace mt