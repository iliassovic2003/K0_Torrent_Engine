#include "../../include/tracker/tracker_manager.hpp"
#include "../../include/tracker/http_tracker.hpp"
#include "../../include/tracker/udp_tracker.hpp"
#include "../../include/common/error.hpp"
#include "../../include/common/logger.hpp"

static constexpr const char* TAG = "K0_TrackerMgr";

using Clock = std::chrono::steady_clock;

// ── TierEntry ─────────────────────────────────────────────────────────────────
bool TrackerManager::TierEntry::is_due() const {
    return Clock::now() >= next_announce;
}

Tracker* TrackerManager::TierEntry::get_tracker() {
    if (!tracker) tracker = TrackerManager::make_tracker(url);
    return tracker.get();
}

// ── Factory ───────────────────────────────────────────────────────────────────
std::unique_ptr<Tracker> TrackerManager::make_tracker(const std::string& url) {
    if (url.substr(0, 4) == "udp:")
        return std::make_unique<UdpTracker>(url);
    if (url.substr(0, 4) == "http")
        return std::make_unique<HttpTracker>(url);
    throw TrackerError("Unsupported tracker scheme: " + url);
}

// ── Ctor ──────────────────────────────────────────────────────────────────────
TrackerManager::TrackerManager(std::vector<std::vector<std::string>> announce_list,
                               PeerId peer_id,
                               uint16_t port)
    : peer_id_(peer_id), port_(port)
{
    auto now = Clock::now();
    for (auto& tier_urls : announce_list) {
        Tier tier;
        for (auto& url : tier_urls) {
            TierEntry e;
            e.url           = std::move(url);
            e.next_announce = now;
            tier.push_back(std::move(e));
        }
        if (!tier.empty())
            tiers_.push_back(std::move(tier));
    }
}

// ── try_tier ──────────────────────────────────────────────────────────────────
std::vector<PeerAddress> TrackerManager::try_tier(Tier& tier,
                                                   const TrackerRequest& req)
{
    for (size_t i = 0; i < tier.size(); ++i) {
        TierEntry& e = tier[i];
        try {
            TrackerResponse resp = e.get_tracker()->announce(req);

            if (i > 0)
                std::rotate(tier.begin(), tier.begin() + i, tier.begin() + i + 1);

            e.failures      = 0;
            e.interval      = resp.interval;
            e.next_announce = Clock::now() + std::chrono::seconds(resp.interval);

            if (resp.min_interval > 0)
                min_interval_ = std::min(min_interval_, resp.min_interval);

            return resp.peers;

        } catch (const std::exception& ex) {
            ++e.failures;

            int32_t backoff = std::min(60 * (1 << std::min(e.failures, 5)), 1800);
            e.next_announce = Clock::now() + std::chrono::seconds(backoff);
            
            LOG_W(TAG, "Tracker %s failed (attempt %d): %s — retry in %ds",
                  e.url.c_str(), e.failures, ex.what(), backoff);
        }
    }
    return {};
}

// ── announce ─────────────────────────────────────────────────────────────────

std::vector<PeerAddress> TrackerManager::announce(const InfoHash& info_hash,
                                                   int64_t left,
                                                   int64_t downloaded,
                                                   int64_t uploaded,
                                                   TrackerEvent event)
{
    TrackerRequest req;
    req.info_hash  = info_hash;
    req.peer_id    = peer_id_;
    req.port       = port_;
    req.downloaded = downloaded;
    req.uploaded   = uploaded;
    req.left       = left;
    req.event      = event;

    std::set<std::pair<uint32_t, uint16_t>> seen;
    std::vector<PeerAddress> all_peers;

    for (auto& tier : tiers_) {
        auto peers = try_tier(tier, req);
        for (auto& p : peers) {
            auto key = std::make_pair(p.ip, p.port);
            if (seen.insert(key).second)
                all_peers.push_back(p);
        }
    }

    LOG_I(TAG, "Announce complete — %zu unique peers from %zu tiers",
          all_peers.size(), tiers_.size());
    return all_peers;
}

// ── reannounce_if_due ────────────────────────────────────────────────────────
std::vector<PeerAddress> TrackerManager::reannounce_if_due(const InfoHash& info_hash,
                                                            int64_t downloaded,
                                                            int64_t uploaded,
                                                            int64_t left)
{
    TrackerRequest req;
    req.info_hash  = info_hash;
    req.peer_id    = peer_id_;
    req.port       = port_;
    req.downloaded = downloaded;
    req.uploaded   = uploaded;
    req.left       = left;
    req.event      = TrackerEvent::None;

    std::set<std::pair<uint32_t, uint16_t>> seen;
    std::vector<PeerAddress> all_peers;

    for (auto& tier : tiers_) {
        if (tier.empty() || !tier.front().is_due())
            continue;

        auto peers = try_tier(tier, req);
        for (auto& p : peers) {
            auto key = std::make_pair(p.ip, p.port);

            if (seen.insert(key).second)
                all_peers.push_back(p);
        }
    }
    return all_peers;
}

// ── stop ─────────────────────────────────────────────────────────────────────
void TrackerManager::stop(const InfoHash& info_hash,
                           int64_t downloaded,
                           int64_t uploaded)
{
    TrackerRequest req;
    req.info_hash  = info_hash;
    req.peer_id    = peer_id_;
    req.port       = port_;
    req.downloaded = downloaded;
    req.uploaded   = uploaded;
    req.left       = 0;
    req.event      = TrackerEvent::Stopped;

    for (auto& tier : tiers_) {
        if (tier.empty())
            continue;

        try {
            tier.front().get_tracker()->announce(req);
        } catch (const std::exception& ex) {
            LOG_W(TAG, "Stop announce to %s failed: %s", tier.front().url.c_str(), ex.what());
        }
    }
}