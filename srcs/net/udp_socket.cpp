#include "udp_socket.hpp"

static std::string errno_msg(const std::string& ctx)
{
    return ctx + ": " + std::strerror(errno);
}

static addrinfo* resolve(const std::string& host, uint16_t port)
{
    const std::string port_str = std::to_string(port);

    addrinfo hints{};
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    addrinfo* res = nullptr;
    const int rc  = ::getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res);
    if (rc != 0)
        throw NetError("getaddrinfo(" + host + "): " + ::gai_strerror(rc));

    return res;
}

UdpSocket::UdpSocket() : fd_(-1) {}

UdpSocket::~UdpSocket()
{
    if (fd_ != -1)
        ::close(fd_);
}

UdpSocket::UdpSocket(UdpSocket&& other) noexcept : fd_(other.fd_)
{
    other.fd_ = -1;
}

UdpSocket& UdpSocket::operator=(UdpSocket&& other) noexcept
{
    if (this != &other) {
        if (fd_ != -1)
            ::close(fd_);
        fd_       = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

void UdpSocket::bind(uint16_t port)
{
    if (fd_ != -1)
        throw NetError("bind: socket already open");

    fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd_ == -1)
        throw NetError(errno_msg("socket"));

    int opt = 1;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    if (::bind(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
        ::close(fd_);
        fd_ = -1;
        throw NetError(errno_msg("bind"));
    }
}

void UdpSocket::connect(const std::string& host, uint16_t port)
{
    if (fd_ == -1) {
        fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (fd_ == -1)
            throw NetError(errno_msg("socket"));
    }

    addrinfo* res = resolve(host, port);
    const int rc  = ::connect(fd_, res->ai_addr, res->ai_addrlen);
    ::freeaddrinfo(res);

    if (rc == -1)
        throw NetError(errno_msg("connect"));
}

void UdpSocket::close()
{
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }
}

void UdpSocket::send_to(const std::vector<uint8_t>& data,
                         const std::string& host, uint16_t port)
{
    if (fd_ == -1)
        throw NetError("send_to on closed socket");

    addrinfo* res = resolve(host, port);
    const ssize_t n = ::sendto(fd_,
                               data.data(), data.size(),
                               MSG_NOSIGNAL,
                               res->ai_addr, res->ai_addrlen);
    ::freeaddrinfo(res);

    if (n < 0)
        throw NetError(errno_msg("sendto"));

    if (static_cast<size_t>(n) != data.size())
        throw NetError("sendto: partial datagram sent");
}

std::vector<uint8_t> UdpSocket::recv_from(size_t max_bytes, PeerAddr& addr)
{
    if (fd_ == -1)
        throw NetError("recv_from on closed socket");

    std::vector<uint8_t> buf(max_bytes);

    sockaddr_storage src{};
    socklen_t        src_len = sizeof(src);

    ssize_t n;
    do {
        n = ::recvfrom(fd_, buf.data(), max_bytes, 0, reinterpret_cast<sockaddr*>(&src), &src_len);
    } while (n < 0 && errno == EINTR);

    if (n < 0)
        throw NetError(errno_msg("recvfrom"));

    char ip_buf[INET6_ADDRSTRLEN] = {};

    if (src.ss_family == AF_INET) {
        auto* s4 = reinterpret_cast<sockaddr_in*>(&src);
        ::inet_ntop(AF_INET, &s4->sin_addr, ip_buf, sizeof(ip_buf));
        addr = { ip_buf, ntohs(s4->sin_port) };
    } else {
        auto* s6 = reinterpret_cast<sockaddr_in6*>(&src);
        ::inet_ntop(AF_INET6, &s6->sin6_addr, ip_buf, sizeof(ip_buf));
        addr = { ip_buf, ntohs(s6->sin6_port) };
    }

    buf.resize(static_cast<size_t>(n));
    return buf;
}

void UdpSocket::send(const std::vector<uint8_t>& data)
{
    if (fd_ == -1)
        throw NetError("send on closed socket");

    const ssize_t n = ::send(fd_, data.data(), data.size(), MSG_NOSIGNAL);
    if (n < 0)
        throw NetError(errno_msg("send"));
    if (static_cast<size_t>(n) != data.size())
        throw NetError("send: partial datagram sent");
}

std::vector<uint8_t> UdpSocket::recv(size_t max_bytes)
{
    if (fd_ == -1)
        throw NetError("recv on closed socket");

    std::vector<uint8_t> buf(max_bytes);
    ssize_t n;
    do {
        n = ::recv(fd_, buf.data(), max_bytes, 0);
    } while (n < 0 && errno == EINTR);

    if (n < 0)
        throw NetError(errno_msg("recv"));

    buf.resize(static_cast<size_t>(n));
    return buf;
}

void UdpSocket::set_nonblocking(bool on)
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