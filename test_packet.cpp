#include <cassert>
#include <iostream>
#include "packet.h"

using namespace mt;

static Packet make(uint32_t seq, uint32_t ack, uint8_t flags, std::string data)
{
    Packet p;
    p.seq = seq;
    p.ack = ack;
    p.window = 8;
    p.flags = flags;
    p.payload.assign(data.begin(), data.end());
    return p;
}

int main()
{
    // 1. Round trip with payload
    {
        Packet p = make(1, 2, ACK, "hello");
        auto bytes = serialize(p);
        assert(bytes.size() == HEADER_SIZE + 5);
        auto q = parse(bytes.data(), bytes.size());
        assert(q.has_value());
        assert(q->seq == 1 && q->ack == 2 && q->window == 8);
        assert(q->flags == ACK);
        assert(q->payload == p.payload);
    }

    // 2. Empty payload (pure ACK/SYN) and odd-length payload
    {
        auto b1 = serialize(make(7, 0, SYN, ""));
        assert(b1.size() == HEADER_SIZE);
        assert(parse(b1.data(), b1.size()).has_value());

        auto b2 = serialize(make(7, 0, 0, "abc")); // 3 bytes: odd length
        assert(parse(b2.data(), b2.size()).has_value());
    }

    // 3. Wire layout is big-endian
    {
        auto b = serialize(make(0x01020304, 0x0A0B0C0D, SYN | ACK, ""));
        assert(b[0] == 0x01 && b[1] == 0x02 && b[2] == 0x03 && b[3] == 0x04);
        assert(b[4] == 0x0A && b[7] == 0x0D);
        assert(b[12] == (SYN | ACK));
    }

    // 4. Any single flipped byte must be rejected
    {
        auto good = serialize(make(5, 6, ACK, "some payload"));
        for (size_t i = 0; i < good.size(); ++i)
        {
            auto bad = good;
            bad[i] ^= 0x5A;
            assert(!parse(bad.data(), bad.size()).has_value());
        }
    }

    // 5. Truncated and padded packets are rejected
    {
        auto good = serialize(make(5, 6, 0, "payload"));
        assert(!parse(good.data(), 10).has_value());              // shorter than header
        assert(!parse(good.data(), good.size() - 1).has_value()); // missing last byte
        auto longer = good;
        longer.push_back(0);
        assert(!parse(longer.data(), longer.size()).has_value()); // trailing junk
    }

    std::cout << "all packet tests passed\n";
}