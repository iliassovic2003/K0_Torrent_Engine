#pragma once

# include "../torrent/torrent_file.hpp"
# include "../tracker/tracker_manager.hpp"
# include "../peer/peer_connection.hpp"
# include "../net/event_loop.hpp"
# include "../io/disk_manager.hpp"
# include "../piece/piece_manager.hpp"
# include "../common/types.hpp"

# include <string>
# include <vector>
# include <memory>

class Session {
public:
    Session(const std::string& torrent_path);
    ~Session();

    void                start();
    void                stop();

private:
    PeerId                                          my_peer_id_;
    uint16_t                                        listen_port_;

    TorrentFile                                     torrent_info_;
    
    EventLoop loop_;
    std::unique_ptr<TrackerManager>                 tracker_mgr_;
    std::unique_ptr<DiskManager>                    disk_mgr_;
    std::unique_ptr<PieceManager>                   piece_mgr_;
    
    std::vector<std::shared_ptr<PeerConnection>>    connections_;

    void                fetch_peers();
    void                on_peer_message(std::shared_ptr<PeerConnection> connection, PeerMessage msg);
    
    void                request_next_blocks(std::shared_ptr<PeerConnection> connection);
};