#pragma once

#include "../common/types.hpp"
#include "../common/error.hpp"

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <set>
#include <cassert>
#include <cstdio>
#include <iostream>

// ── Tracker request ───────────────────────────────────────────────────────────
enum class TrackerEvent : int32_t {
    None      = 0,
    Completed = 1,
    Started   = 2,
    Stopped   = 3,
};

struct TrackerRequest {
    InfoHash    info_hash;
    PeerId      peer_id;
    uint16_t    port        = 6881;
    int64_t     uploaded    = 0;
    int64_t     downloaded  = 0;
    int64_t     left        = 0;
    TrackerEvent event      = TrackerEvent::Started;
    int32_t     num_want    = 50;
    bool        compact     = true;
};

// ── Tracker response ──────────────────────────────────────────────────────────

struct TrackerResponse {
    int32_t                 interval     = 1800;
    int32_t                 min_interval = 0;
    int32_t                 complete     = 0;
    int32_t                 incomplete   = 0;
    std::vector<PeerAddress> peers;
    std::string             warning;
};

// ── Abstract tracker base ─────────────────────────────────────────────────────

class Tracker {
public:
    virtual ~Tracker() = default;

    virtual TrackerResponse announce(const TrackerRequest& req) = 0;

    virtual void scrape(const InfoHash& /*info_hash*/,
                        int32_t& /*seeders*/,
                        int32_t& /*leechers*/,
                        int32_t& /*completed*/) {
        throw TrackerError("Scrape not supported by this tracker");
    }

    const std::string& url() const { return url_; }

protected:
    explicit Tracker(std::string url) : url_(std::move(url)) {}
    std::string url_;
};

std::string url_encode(const uint8_t* data, size_t len);
std::string url_encode_hash(const InfoHash& h);
std::string url_encode_peer_id(const PeerId& id);
