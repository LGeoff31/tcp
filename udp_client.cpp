#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <string>

int main()
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(9001);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    std::string line;
    while (std::getline(std::cin, line))
    {
        sendto(fd, line.data(), line.size(), 0, (sockaddr *)&server, sizeof(server));
        char buf[1024];
        ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, nullptr, nullptr);
        if (n < 0)
            break;
        std::cout << "echo: " << std::string(buf, n) << "\n";
    }
    close(fd);
}