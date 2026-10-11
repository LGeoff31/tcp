#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <string>
#include "packet.h"

int main()
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(9002);
    if (bind(fd, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        return 1;
    }
    std::cout << "listening on UDP :9002\n";

    while (true)
    {
        uint8_t buf[2048];
        ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, nullptr, nullptr);
        if (n < 0)
        {
            perror("recvfrom");
            continue;
        }

        auto p = mt::parse(buf, n);
        if (!p)
        {
            std::cout << "dropped invalid packet (" << n << " bytes)\n";
            continue;
        }
        std::cout << mt::describe(*p);
        if (!p->payload.empty())
            std::cout << " payload=\"" << std::string(p->payload.begin(), p->payload.end()) << "\"";
        std::cout << "\n";
    }
}