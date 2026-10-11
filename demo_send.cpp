#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <vector>
#include "packet.h"

static void send_bytes(int fd, const sockaddr_in &to, const std::vector<uint8_t> &b)
{
    sendto(fd, b.data(), b.size(), 0, (const sockaddr *)&to, sizeof(to));
}

int main()
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in to{};
    to.sin_family = AF_INET;
    to.sin_port = htons(9002);
    inet_pton(AF_INET, "127.0.0.1", &to.sin_addr);

    mt::Packet syn;
    syn.seq = 100;
    syn.flags = mt::SYN;
    syn.window = 8;
    send_bytes(fd, to, mt::serialize(syn));

    mt::Packet data;
    data.seq = 101;
    data.flags = mt::ACK;
    std::string msg = "hello from my protocol";
    data.payload.assign(msg.begin(), msg.end());
    send_bytes(fd, to, mt::serialize(data));

    // Corrupt one payload byte after serializing: the receiver should drop it
    auto corrupted = mt::serialize(data);
    corrupted[mt::HEADER_SIZE] ^= 0xFF;
    send_bytes(fd, to, corrupted);

    close(fd);
}