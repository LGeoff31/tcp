#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>

int main()
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(9000);

    if (bind(listen_fd, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        return 1;
    }
    listen(listen_fd, 16);
    std::cout << "TCP server on :9000\n";

    while (true)
    {
        int fd = accept(listen_fd, nullptr, nullptr);
        char buf[1024];
        ssize_t n;
        while ((n = recv(fd, buf, sizeof(buf), 0)) > 0)
            send(fd, buf, n, 0);
        close(fd);
    }
}