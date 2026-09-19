#pragma once

# include <string>
# include <vector>
# include <cstdint>
# include <stdexcept>
# include <cerrno>
# include <cstring>
# include <fcntl.h>
# include <netdb.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <unistd.h>


struct NetError : public std::runtime_error {
    explicit NetError(const std::string& msg) : std::runtime_error("net: " + msg) {}
};

class TcpSocket {
    public:
        TcpSocket();
        ~TcpSocket();

        TcpSocket(const TcpSocket&)            = delete;
        TcpSocket& operator=(const TcpSocket&) = delete;

        TcpSocket(TcpSocket&& other) noexcept;
        TcpSocket& operator=(TcpSocket&& other) noexcept;

        void connect(const std::string& host, uint16_t port);
        void close();

        void                  send(const std::vector<uint8_t>& data);
        std::vector<uint8_t>  recv(size_t max_bytes);

        void set_nonblocking(bool on);
        int  fd() const { return fd_; }
        bool is_open() const { return fd_ != -1; }

    private:
        int fd_;
};