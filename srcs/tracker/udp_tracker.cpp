#include "../../include/tracker/udp_tracker.hpp"
#include "../../include/common/error.hpp"
#include "../../include/common/logger.hpp"

static constexpr const char* TAG = "K0_UdpTracker";

// BEP 15 magic connection ID used in the initial connect request
static constexpr uint64_t CONNECT_MAGIC = 0x41727101980ULL;

namespace k0 {

    // ── Big-endian write helpers ──────────────────────────────────────────────────
    static void write_u16(uint8_t* p, uint16_t v) {
        p[0] = (v >> 8) & 0xFF;
        p[1] =  v       & 0xFF;
    }
    static void write_u32(uint8_t* p, uint32_t v) {
        p[0] = (v >> 24) & 0xFF; p[1] = (v >> 16) & 0xFF;
        p[2] = (v >>  8) & 0xFF; p[3] =  v         & 0xFF;
    }
    static void write_u64(uint8_t* p, uint64_t v) {
        write_u32(p,     static_cast<uint32_t>(v >> 32));
        write_u32(p + 4, static_cast<uint32_t>(v));
    }
    static void write_i64(uint8_t* p, int64_t v) {
        write_u64(p, static_cast<uint64_t>(v));
    }
    static uint32_t read_u32(const uint8_t* p) {
        return (static_cast<uint32_t>(p[0]) << 24)
            | (static_cast<uint32_t>(p[1]) << 16)
            | (static_cast<uint32_t>(p[2]) <<  8)
            |  static_cast<uint32_t>(p[3]);
    }
    static uint64_t read_u64(const uint8_t* p) {
        return (static_cast<uint64_t>(read_u32(p)) << 32)
            | static_cast<uint64_t>(read_u32(p + 4));
    }

    // ── Ctor ──────────────────────────────────────────────────────────────────────
    UdpTracker::UdpTracker(std::string url, int timeout_s)
        : Tracker(std::move(url))
        , timeout_s_(timeout_s)
        , key_(static_cast<uint32_t>(std::rand()))
    {
        const std::string& u = url_;
        size_t sep = u.find("://");
        if (sep == std::string::npos)
            throw TrackerError("Invalid UDP tracker URL: " + u);

        size_t host_start = sep + 3;
        size_t slash = u.find('/', host_start);
        std::string host_port = (slash == std::string::npos)
            ? u.substr(host_start)
            : u.substr(host_start, slash - host_start);

        size_t colon = host_port.rfind(':');
        if (colon == std::string::npos)
            throw TrackerError("UDP tracker URL missing port: " + u);

        host_ = host_port.substr(0, colon);
        port_ = static_cast<uint16_t>(std::stoi(host_port.substr(colon + 1)));
    }

    // ── connect_udp ───────────────────────────────────────────────────────────────
    int UdpTracker::connect_udp(const std::string& host, uint16_t port) {
        struct addrinfo hints{};
        hints.ai_family   = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;

        struct addrinfo* res = nullptr;
        std::string port_str = std::to_string(port);
        int rv = getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res);
        if (rv != 0)
            throw NetworkError(std::string("DNS failed for ") + host + ": " + gai_strerror(rv));

        int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (fd < 0) {
            freeaddrinfo(res);
            throw NetworkError("socket() failed");
        }

        if (::connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
            freeaddrinfo(res);
            ::close(fd);
            throw NetworkError("UDP connect() failed to " + host);
        }
        freeaddrinfo(res);
        return fd;
    }

    // ── send_recv with exponential-backoff retry ──────────────────────────────────
    ssize_t UdpTracker::send_recv(int sock_fd,
                                const uint8_t* send_buf, size_t send_len,
                                uint8_t*       recv_buf, size_t recv_cap,
                                int            base_timeout_s)
    {
        // BEP 15 specifies timeout = 15 * 2^n seconds, max 8 tries.
        for (int attempt = 0; attempt < 4; ++attempt) {
            int timeout = base_timeout_s * (1 << attempt);

            if (::send(sock_fd, send_buf, send_len, 0) < 0)
                throw NetworkError("UDP send() failed");

            struct timeval tv{ timeout, 0 };
            setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

            ssize_t n = ::recv(sock_fd, recv_buf, recv_cap, 0);
            if (n > 0)
                return n;

            LOG_W(TAG, "No response (attempt %d), retrying in %ds...", attempt + 1, timeout * 2);
        }
        throw TrackerError("UDP tracker timed out after 4 attempts");
    }

    // ── BEP 15 connect handshake ──────────────────────────────────────────────────

    uint64_t UdpTracker::do_connect(int sock_fd, uint32_t& txn_id_out) {
        uint32_t txn_id = static_cast<uint32_t>(std::rand());
        txn_id_out = txn_id;

        uint8_t req[16];

        write_u64(req,      CONNECT_MAGIC);
        write_u32(req + 8,  0);
        write_u32(req + 12, txn_id);

        uint8_t resp[16];
        ssize_t n = send_recv(sock_fd, req, sizeof(req), resp, sizeof(resp), 15);

        if (n < 16)
            throw TrackerError("UDP connect response too short");
        if (read_u32(resp) != 0)
            throw TrackerError("UDP connect response has wrong action");
        if (read_u32(resp + 4) != txn_id)
            throw TrackerError("UDP connect response transaction_id mismatch");

        uint64_t connection_id = read_u64(resp + 8);
        LOG_D(TAG, "UDP connect OK, connection_id=0x%llx", (unsigned long long)connection_id);
        
        return connection_id;
    }

    // ── BEP 15 announce ───────────────────────────────────────────────────────────

    TrackerResponse UdpTracker::do_announce(int sock_fd,
                                            uint64_t connection_id,
                                            const TrackerRequest& req)
    {
        uint32_t txn_id = static_cast<uint32_t>(std::rand());

        // Build 98-byte announce request
        uint8_t buf[98];
        memset(buf, 0, sizeof(buf));

        write_u64(buf,      connection_id);
        write_u32(buf + 8,  1);
        write_u32(buf + 12, txn_id);

        memcpy(buf + 16, req.info_hash.data(), 20);
        memcpy(buf + 36, req.peer_id.data(),   20);

        write_i64(buf + 56, req.downloaded);
        write_i64(buf + 64, req.left);
        write_i64(buf + 72, req.uploaded);

        write_u32(buf + 80, static_cast<uint32_t>(req.event));
        write_u32(buf + 84, 0);
        write_u32(buf + 88, key_);
        write_u32(buf + 92, static_cast<uint32_t>(req.num_want));

        write_u16(buf + 96, req.port);

        // Response can be 20 + 6*n bytes; allow up to 200 peers
        constexpr size_t MAX_RESP = 20 + 6 * 200;
        uint8_t resp[MAX_RESP];
        ssize_t n = send_recv(sock_fd, buf, sizeof(buf), resp, MAX_RESP, 15);

        if (n < 20)
            throw TrackerError("UDP announce response too short");
        if (read_u32(resp) != 1)
            throw TrackerError("UDP announce response has wrong action");
        if (read_u32(resp + 4) != txn_id)
            throw TrackerError("UDP announce transaction_id mismatch");

        TrackerResponse out;
        out.interval   = static_cast<int32_t>(read_u32(resp + 8));
        out.incomplete = static_cast<int32_t>(read_u32(resp + 12));
        out.complete   = static_cast<int32_t>(read_u32(resp + 16));

        size_t num_peers = static_cast<size_t>(n - 20) / 6;
        for (size_t i = 0; i < num_peers; ++i) {
            const uint8_t* entry = resp + 20 + i * 6;
            PeerAddress pa;

            memcpy(&pa.ip, entry, 4);
            uint16_t p; memcpy(&p, entry + 4, 2);
            pa.port = ntohs(p);
            out.peers.push_back(pa);
        }

        LOG_I(TAG, "UDP announce OK — interval=%ds seeders=%d leechers=%d peers=%zu",
            out.interval, out.complete, out.incomplete, out.peers.size());

        return out;
    }

    // ── Public API ────────────────────────────────────────────────────────────────
    TrackerResponse UdpTracker::announce(const TrackerRequest& req) {
        int fd = connect_udp(host_, port_);

        try {
            uint32_t txn_id = 0;
            uint64_t conn_id = do_connect(fd, txn_id);
            TrackerResponse resp = do_announce(fd, conn_id, req);
            ::close(fd);
            return resp;
        } catch (...) {
            ::close(fd);
            throw;
        }
    }

}