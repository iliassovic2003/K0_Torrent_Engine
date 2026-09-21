#pragma once

#include "tracker.hpp"

namespace k0 {

    // TrackerManager drives the full multi-tracker announce logic described in
    // BEP 12 (multi-tracker metadata extension):
    //
    //   - Tracker URLs are grouped into tiers (announce-list from .torrent).
    //   - Within each tier, trackers are tried in order; on success the
    //     successful tracker is moved to the front of its tier.
    //   - If every tracker in a tier fails, the next tier is tried.
    //   - A re-announce is scheduled automatically when interval expires.
    //   - Exponential backoff (up to 1800s) on consecutive failures.
    //
    // Usage:
    //   TrackerManager mgr(torrent.announce_list(), peer_id, port);
    //   auto peers = mgr.announce(info_hash, left);
    //   // later:
    //   peers = mgr.reannounce_if_due(info_hash, downloaded, uploaded, left);

    class TrackerManager {
    public:
        TrackerManager(std::vector<std::vector<std::string>> announce_list,
                    PeerId peer_id,
                    uint16_t port);

        // Run a full announce across tiers; returns merged unique peer list.
        std::vector<PeerAddress> announce(const InfoHash& info_hash,
                                        int64_t left,
                                        int64_t downloaded = 0,
                                        int64_t uploaded   = 0,
                                        TrackerEvent event = TrackerEvent::Started);

        std::vector<PeerAddress> reannounce_if_due(const InfoHash& info_hash,
                                                int64_t downloaded,
                                                int64_t uploaded,
                                                int64_t left);

        // Send Stopped event to all responsive trackers.
        void stop(const InfoHash& info_hash, int64_t downloaded, int64_t uploaded);

        int32_t min_interval() const {
            return min_interval_;
        }

    private:
        struct TierEntry {
            std::string                             url;
            std::unique_ptr<Tracker>                tracker;
            int32_t                                 interval   = 1800;
            int32_t                                 failures   = 0;
            std::chrono::steady_clock::time_point   next_announce;

            bool        is_due() const;
            Tracker*    get_tracker();
        };

        using Tier = std::vector<TierEntry>;

        std::vector<Tier> tiers_;
        PeerId            peer_id_;
        uint16_t          port_;
        int32_t           min_interval_ = 1800;

        // Factory: pick HttpTracker or UdpTracker based on URL scheme.
        static std::unique_ptr<Tracker> make_tracker(const std::string& url);

        // Try announcing to a single tier; returns peers on first success.
        std::vector<PeerAddress> try_tier(Tier& tier, const TrackerRequest& req);
    };

}