#pragma once

# include "tracker.hpp"
# include "../bencode/bencode.hpp"

// HTTP/HTTPS tracker client — implements BEP 3 announce over TCP.
//
// Wire format:
//   GET /announce?info_hash=...&peer_id=...&port=...&... HTTP/1.0\r\n
//   Host: tracker.example.com\r\n
//   \r\n
//
// Response is a bencoded dict:
//   { "interval": <int>,
//     "peers":    <compact 6-byte IPv4 list> | <bencoded peer list>,
//     "complete": <int>,    (optional)
//     "incomplete": <int> } (optional)

class HttpTracker : public Tracker {
public:
    // url  — full announce URL, e.g. "http://tracker.example.com:6969/announce"
    explicit HttpTracker(std::string url, int timeout_s = 15);

    TrackerResponse announce(const TrackerRequest& req) override;

    void scrape(const InfoHash& info_hash,
                int32_t& seeders,
                int32_t& leechers,
                int32_t& completed) override;

private:
    int timeout_s_;

    std::string                     build_query(const TrackerRequest& req) const;
    std::string                     http_get(const std::string& host, uint16_t port, const std::string& path);
    TrackerResponse                 parse_response(const std::string& body);

    static std::vector<PeerAddress> parse_compact_peers(const std::string& raw);
    static std::vector<PeerAddress> parse_dict_peers(const BencodeValue& peers_val);
};