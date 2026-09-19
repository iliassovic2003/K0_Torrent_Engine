#pragma once
#include "tcp_socket.hpp"

#include <cstdint>
#include <string>
#include <vector>
#include <cerrno>
#include <cstring>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct PeerAddr {
    std::string ip;
    uint16_t    port;
};

class UdpSocket {
public:
    UdpSocket();
    ~UdpSocket();

    UdpSocket(const UdpSocket&)            = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    UdpSocket(UdpSocket&& other) noexcept;
    UdpSocket& operator=(UdpSocket&& other) noexcept;

    void bind(uint16_t port);
    void connect(const std::string& host, uint16_t port);
    void close();
    void send_to(const std::vector<uint8_t>& data,
                 const std::string& host, uint16_t port);

    std::vector<uint8_t> recv_from(size_t max_bytes, PeerAddr& addr);

    void                 send(const std::vector<uint8_t>& data);
    std::vector<uint8_t> recv(size_t max_bytes);

    void set_nonblocking(bool on);
    int  fd()      const { return fd_; }
    bool is_open() const { return fd_ != -1; }

private:
    int fd_;
};