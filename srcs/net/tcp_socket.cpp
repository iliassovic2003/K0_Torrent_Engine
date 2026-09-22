#include "../../include/net/tcp_socket.hpp"

static std::string errno_msg(const std::string& ctx)
{
    return ctx + ": " + std::strerror(errno);
}

TcpSocket::TcpSocket() : fd_(-1) {}

TcpSocket::~TcpSocket()
{
    if (fd_ != -1)
        ::close(fd_);
}

TcpSocket::TcpSocket(TcpSocket&& other) noexcept : fd_(other.fd_)
{
    other.fd_ = -1;
}

TcpSocket& TcpSocket::operator=(TcpSocket&& other) noexcept
{
    if (this != &other) {
        if (fd_ != -1)
            ::close(fd_);
        fd_       = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

void TcpSocket::connect(const std::string& host, uint16_t port)
{
    if (fd_ != -1)
        throw NetError("socket already open — call close() first");

    const std::string port_str = std::to_string(port);

    addrinfo hints{};
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* res = nullptr;
    const int rc  = ::getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res);
    if (rc != 0)
        throw NetError("getaddrinfo(" + host + "): " + ::gai_strerror(rc));

    int sock = -1;
    for (addrinfo* ai = res; ai != nullptr; ai = ai->ai_next) {
        sock = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (sock == -1)
            continue;

        int flags = ::fcntl(sock, F_GETFL, 0);
        ::fcntl(sock, F_SETFL, flags | O_NONBLOCK);

        if (::connect(sock, ai->ai_addr, ai->ai_addrlen) == 0 || errno == EINPROGRESS)
            break;

        ::close(sock);
        sock = -1;
    }
    ::freeaddrinfo(res);

    if (sock == -1)
        throw NetError("could not connect to " + host + ":" + port_str);

    fd_ = sock;
}

void TcpSocket::close()
{
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }
}

size_t TcpSocket::send(const uint8_t* data, size_t size)
{
    if (fd_ == -1)
        throw NetError("send on closed socket");

    ssize_t n;
    do {
        n = ::send(fd_, data, size, MSG_NOSIGNAL);
    } while (n < 0 && errno == EINTR);

    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return 0;
        throw NetError(errno_msg("send"));
    }

    return static_cast<size_t>(n);
}

std::vector<uint8_t> TcpSocket::recv(size_t max_bytes)
{
    if (fd_ == -1)
        throw NetError("recv on closed socket");

    std::vector<uint8_t> buf(max_bytes);
    ssize_t n;
    do {
        n = ::recv(fd_, buf.data(), max_bytes, 0);
    } while (n < 0 && errno == EINTR);

    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return {};
        throw NetError(errno_msg("recv"));
    }

    if (n == 0)
        throw NetError("recv: connection closed by peer");

    buf.resize(static_cast<size_t>(n));
    return buf;
}

void TcpSocket::set_nonblocking(bool on)
{
    if (fd_ == -1)
        throw NetError("set_nonblocking on closed socket");

    int flags = ::fcntl(fd_, F_GETFL, 0);
    if (flags == -1)
        throw NetError(errno_msg("fcntl(F_GETFL)"));

    flags = on ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);

    if (::fcntl(fd_, F_SETFL, flags) == -1)
        throw NetError(errno_msg("fcntl(F_SETFL)"));
}