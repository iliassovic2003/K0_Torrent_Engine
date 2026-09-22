#include "../../include/peer/peer_connection.hpp"
#include "../../include/common/error.hpp"
#include "../../include/common/logger.hpp"
#include <arpa/inet.h>
#include <sys/poll.h>
#include <cerrno>
#include <cstring>

static constexpr const char* TAG = "K0_PeerConn";

static std::string ip_to_string(uint32_t ip) {
    struct in_addr addr;
    addr.s_addr = ip;
    return std::string(inet_ntoa(addr));
}

PeerConnection::PeerConnection(EventLoop& loop, std::shared_ptr<Peer> peer, 
                               const InfoHash& info_hash, const PeerId& my_id)
    : loop_(loop), peer_(std::move(peer)), state_(ConnectionState::Disconnected),
      my_info_hash_(info_hash), my_peer_id_(my_id) {}

PeerConnection::~PeerConnection() {
    disconnect();
}

void PeerConnection::start() {
    try {
        std::string ip_str = ip_to_string(peer_->address.ip);
        
        try {
            socket_.connect(ip_str, peer_->address.port);
        } catch (const std::exception& e) {
            std::string err_msg = e.what();
            if (err_msg.find("Operation now in progress") == std::string::npos && 
                err_msg.find("EINPROGRESS") == std::string::npos)
                throw; 
        }

        std::weak_ptr<PeerConnection> weak_self = shared_from_this();
        loop_.add(socket_.fd(), POLLIN | POLLOUT, [weak_self](int /*fd*/, uint32_t events) {
            if (auto self = weak_self.lock())
                self->on_socket_event(events);
        });

        Handshake   hs(my_info_hash_, my_peer_id_);
        auto        hs_buf = hs.serialize();
        write_buffer_.insert(write_buffer_.end(), hs_buf.begin(), hs_buf.end());
        
        state_ = ConnectionState::HandshakeSent;

    } catch (const std::exception& e) {
        LOG_W(TAG, "Failed to start connection to %s: %s", 
              ip_to_string(peer_->address.ip).c_str(), e.what());
        disconnect();
    }
}

void PeerConnection::disconnect() {
    if (state_ == ConnectionState::Disconnected)
        return;
    
    if (socket_.fd() != -1) {
        loop_.remove(socket_.fd());
        socket_.close();
    }
    state_ = ConnectionState::Disconnected;
}

void PeerConnection::queue_message(const PeerMessage& msg) {
    if (state_ != ConnectionState::Connected)
        return;

    auto buf = msg.serialize();
    write_buffer_.insert(write_buffer_.end(), buf.begin(), buf.end());

    loop_.modify(socket_.fd(), POLLIN | POLLOUT);
}

void PeerConnection::on_socket_event(uint32_t events) {
    try {
        if (events & POLLOUT)
            handle_write();
        if (events & POLLIN) 
            handle_read();

    } catch (const std::exception& e) {
        LOG_W(TAG, "Connection error: %s", e.what());
        disconnect();
    }
}

void PeerConnection::handle_write() {
    if (write_buffer_.empty())
        return;

    size_t sent = socket_.send(write_buffer_.data(), write_buffer_.size());
    if (sent > 0)
        write_buffer_.erase(write_buffer_.begin(), write_buffer_.begin() + sent);

    if (write_buffer_.empty())
        loop_.modify(socket_.fd(), POLLIN);
}

void PeerConnection::handle_read() {
    try {
        auto data = socket_.recv(4096);
        
        if (data.empty()) {
            return;
        }

        read_buffer_.insert(read_buffer_.end(), data.begin(), data.end());

        if (state_ == ConnectionState::HandshakeSent)
            if (!process_handshake())
                return;

        if (state_ == ConnectionState::Connected) {
            while (true) {
                ParseResult result = parse_peer_message(read_buffer_);
                if (!result.complete)
                    break;

                if (!result.is_keep_alive && message_handler_) 
                    message_handler_(shared_from_this(), std::move(result.message));

                read_buffer_.erase(read_buffer_.begin(), read_buffer_.begin() + result.consumed_bytes);
            }
        }
    } catch (const std::exception& e) {
        std::string err_msg = e.what();

        if (err_msg.find("closed by peer") != std::string::npos)
            LOG_I(TAG, "Peer closed connection cleanly (EOF)");
        else
            LOG_W(TAG, "Read error: %s", err_msg.c_str());
        
        disconnect();
    }
}

bool PeerConnection::process_handshake() {
    if (read_buffer_.size() < Handshake::HANDSHAKE_SIZE)
        return false;

    std::array<uint8_t, Handshake::HANDSHAKE_SIZE> hs_buf;
    std::copy(read_buffer_.begin(), read_buffer_.begin() + Handshake::HANDSHAKE_SIZE, hs_buf.begin());

    try {
        Handshake peer_hs = Handshake::deserialize(hs_buf);
        
        if (peer_hs.info_hash != my_info_hash_)
            throw NetError("InfoHash mismatch - peer is confused");

        peer_->id = peer_hs.peer_id;
        peer_->id_set = true;
        state_ = ConnectionState::Connected;
        
        LOG_I(TAG, "TCP connection ESTABLISHED and Handshake successful with peer!");

        read_buffer_.erase(read_buffer_.begin(), read_buffer_.begin() + Handshake::HANDSHAKE_SIZE);
        return true;

    } catch (const std::exception& e) {
        throw NetError(std::string("Handshake failed: ") + e.what());
    }
}