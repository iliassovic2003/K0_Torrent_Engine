#include "../../include/tracker/http_tracker.hpp"
#include "../../include/bencode/bencode.hpp"
#include "../../include/common/error.hpp"
#include "../../include/common/logger.hpp"

static constexpr const char* TAG = "K0_0HttpTracker";

HttpTracker::HttpTracker(std::string url, int timeout_s)
    : Tracker(std::move(url)), timeout_s_(timeout_s)
{}

// ── URL parsing helpers ───────────────────────────────────────────────────────

// Parse "http://host:port/path" → host, port, path
static void parse_url(const std::string& url,
                    std::string& scheme,
                    std::string& host,
                    uint16_t&    port,
                    std::string& path)
{
    size_t scheme_end = url.find("://");
    if (scheme_end == std::string::npos)
        throw TrackerError("Invalid tracker URL: " + url);

    scheme = url.substr(0, scheme_end);
    size_t host_start = scheme_end + 3;

    size_t path_start = url.find('/', host_start);
    std::string host_port = (path_start == std::string::npos)
        ? url.substr(host_start)
        : url.substr(host_start, path_start - host_start);

    path = (path_start == std::string::npos) ? "/" : url.substr(path_start);

    size_t colon = host_port.rfind(':');
    if (colon != std::string::npos) {
        host = host_port.substr(0, colon);
        port = static_cast<uint16_t>(std::stoi(host_port.substr(colon + 1)));
    } else {
        host = host_port;
        port = (scheme == "https") ? 443 : 80;
    }
}

// ── Query string builder ──────────────────────────────────────────────────────
std::string HttpTracker::build_query(const TrackerRequest& req) const {
    std::ostringstream ss;
    ss << "info_hash="  << url_encode_hash(req.info_hash)
    << "&peer_id="   << url_encode_peer_id(req.peer_id)
    << "&port="      << req.port
    << "&uploaded="  << req.uploaded
    << "&downloaded="<< req.downloaded
    << "&left="      << req.left
    << "&compact="   << (req.compact ? 1 : 0)
    << "&numwant="   << req.num_want;

    switch (req.event) {
        case TrackerEvent::Started:   ss << "&event=started";   break;
        case TrackerEvent::Stopped:   ss << "&event=stopped";   break;
        case TrackerEvent::Completed: ss << "&event=completed"; break;
        case TrackerEvent::None: break;
    }
    return ss.str();
}

// ── Low-level HTTP/1.0 GET ────────────────────────────────────────────────────
std::string HttpTracker::http_get(const std::string& host,
                                uint16_t           port,
                                const std::string& path)
{
    struct addrinfo hints{};
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    std::string port_str = std::to_string(port);
    struct addrinfo* res = nullptr;
    int rv = getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res);
    if (rv != 0)
        throw NetworkError(std::string("DNS failed for ") + host + ": " + gai_strerror(rv));

    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) {
        freeaddrinfo(res);
        throw NetworkError("socket() failed");
    }

    struct timeval tv{ timeout_s_, 0 };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
        freeaddrinfo(res);
        close(fd);
        throw NetworkError(std::string("connect() failed to ") + host);
    }
    freeaddrinfo(res);

    std::string req =
        "GET " + path + " HTTP/1.0\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "User-Agent: K0-0Torrent/0.1\r\n"
        "\r\n";

    if (send(fd, req.c_str(), req.size(), 0) < 0) {
        close(fd);
        throw NetworkError("send() failed");
    }

    std::string response;
    char buf[4096];
    ssize_t n;
    while ((n = recv(fd, buf, sizeof(buf), 0)) > 0)
        response.append(buf, static_cast<size_t>(n));
    close(fd);

    size_t body_start = response.find("\r\n\r\n");
    if (body_start == std::string::npos)
        throw TrackerError("Malformed HTTP response (no header end)");

    return response.substr(body_start + 4);
}

// ── Response parsing ──────────────────────────────────────────────────────────
std::vector<PeerAddress> HttpTracker::parse_compact_peers(const std::string& raw) {
    std::vector<PeerAddress> peers;
    if (raw.size() % 6 != 0)
        throw TrackerError("Compact peer list has invalid length");

    for (size_t i = 0; i + 6 <= raw.size(); i += 6) {
        PeerAddress pa;
        memcpy(&pa.ip, raw.data() + i, 4);

        uint16_t p;
        memcpy(&p, raw.data() + i + 4, 2);
        
        pa.port = ntohs(p);
        peers.push_back(pa);
    }
    return peers;
}

std::vector<PeerAddress> HttpTracker::parse_dict_peers(const BencodeValue& peers_val) {
    std::vector<PeerAddress> peers;
    try {
        for (const auto& entry : peers_val.as_list()) {
            const auto& d = entry.as_dict();
            std::string ip_str = d.at("ip").as_string();
            uint16_t    port   = static_cast<uint16_t>(d.at("port").as_int());

            struct in_addr addr;
            if (inet_aton(ip_str.c_str(), &addr) == 0)
                continue;

            PeerAddress pa;
            pa.ip   = addr.s_addr;
            pa.port = port;
            peers.push_back(pa);
        }
    } catch (...) {
        throw TrackerError("Failed to parse dict peer list");
    }
    return peers;
}

TrackerResponse HttpTracker::parse_response(const std::string& body) {
    BencodeValue root;
    size_t pos = 0;

    try {
        root = decode(body, pos);
    } catch (const std::exception& e) {
        throw TrackerError(std::string("Bencode parse error in tracker response: ") + e.what());
    }

    const auto& d = root.as_dict();

    if (d.count("failure reason"))
        throw TrackerError("Tracker failure: " + d.at("failure reason").as_string());

    TrackerResponse resp;

    if (d.count("interval"))
        resp.interval = static_cast<int32_t>(d.at("interval").as_int());
    if (d.count("min interval"))
        resp.min_interval = static_cast<int32_t>(d.at("min interval").as_int());
    if (d.count("complete"))
        resp.complete = static_cast<int32_t>(d.at("complete").as_int());
    if (d.count("incomplete"))
        resp.incomplete = static_cast<int32_t>(d.at("incomplete").as_int());
    if (d.count("warning message"))
        resp.warning = d.at("warning message").as_string();

    // Peers — either compact string or dict list
    if (d.count("peers")) {
        const auto& peers_val = d.at("peers");

        if (peers_val.is_string())
            resp.peers = parse_compact_peers(peers_val.as_string());
        else if (peers_val.is_list())
            resp.peers = parse_dict_peers(peers_val);
    }

    LOG_I(TAG, "Announce OK — interval=%ds seeders=%d leechers=%d peers=%zu",
        resp.interval, resp.complete, resp.incomplete, resp.peers.size());

    return resp;
}

// ── Public API ────────────────────────────────────────────────────────────────
TrackerResponse HttpTracker::announce(const TrackerRequest& req) {
    std::string scheme, host, path;
    uint16_t port;
    parse_url(url_, scheme, host, port, path);

    std::string full_path = path + "?" + build_query(req);
    LOG_D(TAG, "GET http://%s:%u%s", host.c_str(), port, full_path.c_str());

    std::string body = http_get(host, port, full_path);
    return parse_response(body);
}

void HttpTracker::scrape(const InfoHash& info_hash,
                        int32_t& seeders,
                        int32_t& leechers,
                        int32_t& completed)
{
    // Build scrape URL — replace /announce with /scrape
    std::string scrape_url = url_;
    size_t pos_url = scrape_url.rfind("/announce");
    
    if (pos_url == std::string::npos)
        throw TrackerError("Cannot derive scrape URL from: " + url_);
    scrape_url.replace(pos_url, 9, "/scrape");

    std::string         scheme, host, path;
    uint16_t port;
    
    parse_url(scrape_url, scheme, host, port, path);

    std::string         query = "?info_hash=" + url_encode_hash(info_hash);
    std::string         body  = http_get(host, port, path + query);

    try {
        size_t pos = 0;
        auto root = decode(body, pos);
        
        const auto& files = root.as_dict().at("files").as_dict();
        
        std::string hash_key(reinterpret_cast<const char*>(info_hash.data()), 20);
        const auto& entry = files.at(hash_key).as_dict();

        seeders   = static_cast<int32_t>(entry.at("complete").as_int());
        leechers  = static_cast<int32_t>(entry.at("incomplete").as_int());
        completed = static_cast<int32_t>(entry.at("downloaded").as_int());
    } catch (const std::exception& e) {
        throw TrackerError(std::string("Scrape parse error: ") + e.what());
    }
}