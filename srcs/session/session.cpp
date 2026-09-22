#include "../../include/session/session.hpp"
#include "../../include/crypto/peer_id.hpp"
#include "../../include/common/logger.hpp"
#include "../../include/common/error.hpp"
#include <iostream>

static constexpr const char* TAG = "K0_Session";

Session::Session(const std::string& torrent_path) 
    : listen_port_(6881)
{
    LOG_I(TAG, "Initializing session for %s", torrent_path.c_str());

    torrent_info_ = parse_torrent(torrent_path);
    my_peer_id_   = generate_peer_id();

    tracker_mgr_ = std::make_unique<TrackerManager>(torrent_info_.announce_list, my_peer_id_, listen_port_);
    disk_mgr_    = std::make_unique<DiskManager>(torrent_info_, "./downloads");
    piece_mgr_   = std::make_unique<PieceManager>(torrent_info_);
}

Session::~Session() {
    stop();
}

void Session::fetch_peers() {
    LOG_I(TAG, "Contacting trackers...");

    int64_t left = 0;
    for (const auto& file : torrent_info_.files)
        left += file.length;

    std::vector<PeerAddress> peer_ips = tracker_mgr_->announce(
        torrent_info_.info_hash,
        left
    );

    LOG_I(TAG, "Tracker returned %zu peers", peer_ips.size());

    for (const auto& addr : peer_ips) {
        auto peer = std::make_shared<Peer>(addr);
        auto conn = std::make_shared<PeerConnection>(loop_, peer, torrent_info_.info_hash, my_peer_id_);

        conn->set_message_handler([this](std::shared_ptr<PeerConnection> c, PeerMessage m) {
            this->on_peer_message(c, std::move(m));
        });

        connections_.push_back(conn);
        conn->start();
    }
}

void Session::start() {
    fetch_peers();

    if (connections_.empty()) {
        LOG_W(TAG, "No peers found. Exiting session.");
        return;
    }

    LOG_I(TAG, "Starting Event Loop. Press Ctrl+C to stop.");
    loop_.run(); 
}

void Session::stop() {
    LOG_I(TAG, "Stopping session...");
    loop_.stop();
    for (auto& conn : connections_) {
        conn->disconnect();
    }
    connections_.clear();
}

void Session::request_next_blocks(std::shared_ptr<PeerConnection> connection) {
    if (connection->get_peer()->peer_choking)
        return;

    for (int i = 0; i < 5; ++i) {
        auto req = piece_mgr_->create_next_request(*connection->get_peer());
        
        if (req)
            connection->queue_message(*req);
        else
            break;
    }
}

void Session::on_peer_message(std::shared_ptr<PeerConnection> connection, PeerMessage msg) {
    switch (msg.id) {
        case MessageId::Choke:
            LOG_I(TAG, "Peer choked us");
            connection->get_peer()->peer_choking = true;
            break;
            
        case MessageId::Unchoke:
            LOG_I(TAG, "Peer UNCHOKED us! Requesting data...");
            connection->get_peer()->peer_choking = false;
            request_next_blocks(connection);
            break;
            
        case MessageId::Interested:
            connection->get_peer()->peer_interested = true;
            break;
            
        case MessageId::NotInterested:
            connection->get_peer()->peer_interested = false;
            break;
            
        case MessageId::Have:
            if (msg.payload.size() == 4) {
                uint32_t piece_index = (msg.payload[0] << 24) | (msg.payload[1] << 16) | 
                                       (msg.payload[2] << 8)  | msg.payload[3];
                connection->get_peer()->set_piece(piece_index);
                
                if (!connection->get_peer()->am_interested) {
                    connection->get_peer()->am_interested = true;
                    connection->queue_message(PeerMessage(MessageId::Interested));
                }
            }
            break;
            
        case MessageId::Bitfield:
            LOG_I(TAG, "Received Bitfield from peer");
            connection->get_peer()->bitfield = msg.payload;
            
            if (!msg.payload.empty()) {
                connection->get_peer()->am_interested = true;
                connection->queue_message(PeerMessage(MessageId::Interested));
            }
            break;
            
        case MessageId::Piece: {
            if (msg.payload.size() <= 8) break;

            uint32_t index = (msg.payload[0] << 24) | (msg.payload[1] << 16) | 
                             (msg.payload[2] << 8)  | msg.payload[3];
            uint32_t begin = (msg.payload[4] << 24) | (msg.payload[5] << 16) | 
                             (msg.payload[6] << 8)  | msg.payload[7];
            
            std::vector<uint8_t> block_data(msg.payload.begin() + 8, msg.payload.end());

            bool piece_finished = piece_mgr_->process_block(index, begin, block_data);

            if (piece_finished) {
                std::vector<uint8_t> full_piece = piece_mgr_->get_completed_piece(index);
                
                try {
                    disk_mgr_->write_block(index, 0, full_piece);
                    LOG_I(TAG, "Successfully wrote Piece %u to disk!", index);
                } catch (const std::exception& e) {
                    LOG_E(TAG, "Disk write failed: %s", e.what());
                }
            }

            request_next_blocks(connection);
            break;
        }
            
        default:
            break;
    }
}