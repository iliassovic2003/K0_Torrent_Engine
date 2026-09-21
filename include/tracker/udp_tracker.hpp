#pragma once

#include "tracker.hpp"

namespace k0 {

    // UDP tracker client — implements BEP 15.
    //
    // Protocol (all fields big-endian):
    //
    //   ── Connect ──────────────────────────────────────────────────────────
    //   Request  (16 bytes):
    //     uint64  connection_id  = 0x41727101980   (magic)
    //     uint32  action         = 0               (connect)
    //     uint32  transaction_id = random
    //
    //   Response (16 bytes):
    //     uint32  action         = 0
    //     uint32  transaction_id
    //     uint64  connection_id              ← keep this; expires after 60s
    //
    //   ── Announce ──────────────────────────────────────────────────────────
    //   Request  (98 bytes):
    //     uint64  connection_id
    //     uint32  action         = 1
    //     uint32  transaction_id
    //     uint8[20] info_hash
    //     uint8[20] peer_id
    //     int64   downloaded
    //     int64   left
    //     int64   uploaded
    //     uint32  event          (0=none,1=completed,2=started,3=stopped)
    //     uint32  ip             = 0 (default)
    //     uint32  key            = random
    //     int32   num_want       = -1
    //     uint16  port
    //
    //   Response (20 + 6*n bytes):
    //     uint32  action         = 1
    //     uint32  transaction_id
    //     uint32  interval
    //     uint32  leechers
    //     uint32  seeders
    //     (uint32 ip + uint16 port) * n

    class UdpTracker : public Tracker {
    public:
        // url — full announce URL, e.g. "udp://tracker.example.com:6969/announce"
        explicit UdpTracker(std::string url, int timeout_s = 10);

        TrackerResponse announce(const TrackerRequest& req) override;

    private:
        // Resolve host, create UDP socket, return fd. Throws NetworkError.
        int connect_udp(const std::string& host, uint16_t port);

        // Perform the BEP 15 connect handshake; returns connection_id.
        uint64_t do_connect(int sock_fd, uint32_t& txn_id_out);

        // Perform the BEP 15 announce using an existing connection_id.
        TrackerResponse do_announce(int sock_fd,
                                    uint64_t connection_id,
                                    const TrackerRequest& req);

        // Returns number of bytes received into recv_buf.
        ssize_t send_recv(int sock_fd,
                        const uint8_t* send_buf, size_t send_len,
                        uint8_t*       recv_buf, size_t recv_cap,
                        int            base_timeout_s = 15);

        int      timeout_s_;
        uint32_t key_;

        std::string host_;
        uint16_t    port_;
    };

}