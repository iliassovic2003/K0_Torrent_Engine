#include "../../include/peer/peer_connection.hpp"
#include "../../include/common/error.hpp"
#include "../../include/common/logger.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

static constexpr const char* TAG = "K0_PeerConn";

PeerConnection::PeerConnection(std::shared_ptr<Peer> peer, const InfoHash& info_hash, const PeerId& my_id)
    : peer_(std::move(peer)), socket_fd_(-1), state_(ConnectionState::Disconnected),
      my_info_hash_(info_hash), my_peer_id_(my_id) {}

PeerConnection::~PeerConnection() {
    disconnect();
}

void PeerConnection::disconnect() {
    if (socket_fd_ != -1) {
        ::close(socket_fd_);
        socket_fd_ = -1;
    }
    state_ = ConnectionState::Disconnected;
}

void PeerConnection::send_handshake() {
    Handshake hs(my_info_hash_, my_peer_id_);
    auto buf = hs.serialize();
    
    if (::send(socket_fd_, buf.data(), buf.size(), 0) < 0) {
        LOG_W(TAG, "Failed to send handshake");
        disconnect();
        return;
    }
    state_ = ConnectionState::HandshakeSent;
}

void PeerConnection::send_message(const PeerMessage& msg) {
    if (state_ != ConnectionState::Connected)
        return;

    auto buf = msg.serialize();
    if (send(socket_fd_, buf.data(), buf.size(), 0) < 0)
        disconnect();
}

std::vector<PeerMessage> PeerConnection::receive_data() {
    std::vector<PeerMessage> messages;
    if (socket_fd_ == -1)
        return messages;

    uint8_t temp[4096];
    ssize_t bytes_read = recv(socket_fd_, temp, sizeof(temp), 0);

    if (bytes_read <= 0) {
        disconnect();
        return messages;
    }

    read_buffer_.insert(read_buffer_.end(), temp, temp + bytes_read);

    if (state_ == ConnectionState::HandshakeSent)
        if (!process_handshake()) return messages;

    if (state_ == ConnectionState::Connected) {
        while (true) {
            ParseResult result = parse_peer_message(read_buffer_);
            
            if (!result.complete)
                break;

            if (!result.is_keep_alive)
                messages.push_back(std::move(result.message));

            read_buffer_.erase(read_buffer_.begin(), read_buffer_.begin() + result.consumed_bytes);
        }
    }
    return messages;
}

bool PeerConnection::process_handshake() {
    if (read_buffer_.size() < Handshake::HANDSHAKE_SIZE)
        return false;

    std::array<uint8_t, Handshake::HANDSHAKE_SIZE>  hs_buf;
    std::copy(read_buffer_.begin(), read_buffer_.begin() + Handshake::HANDSHAKE_SIZE, hs_buf.begin());

    try {
        Handshake peer_hs = Handshake::deserialize(hs_buf);
        
        if (peer_hs.info_hash != my_info_hash_)
            throw TrackerError("InfoHash mismatch - peer is confused");

        peer_->id = peer_hs.peer_id;
        peer_->id_set = true;
        state_ = ConnectionState::Connected;
        
        LOG_I(TAG, "Handshake successful!");

        read_buffer_.erase(read_buffer_.begin(), read_buffer_.begin() + Handshake::HANDSHAKE_SIZE);
        return true;

    } catch (const std::exception& e) {
        LOG_W(TAG, "Handshake failed: %s", e.what());
        disconnect();
        
        return false;
    }
}