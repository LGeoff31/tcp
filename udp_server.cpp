#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>

int main()
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(9001);

    if (bind(fd, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        return 1;
    }
    std::cout << "UDP server on :9001\n";

    while (true)
    {
        char buf[1024];
        sockaddr_in client{};
        socklen_t len = sizeof(client);
        ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, (sockaddr *)&client, &len);
        if (n < 0)
        {
            perror("recvfrom");
            continue;
        }
        sendto(fd, buf, n, 0, (sockaddr *)&client, len);
    }
}