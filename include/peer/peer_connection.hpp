#pragma once

#include "peer.hpp"
#include "handshake.hpp"
#include "message.hpp"
#include "../net/event_loop.hpp"
#include "../net/tcp_socket.hpp"
#include <vector>
#include <memory>
#include <functional>

enum class ConnectionState {
    Disconnected,
    HandshakeSent,
    Connected
};

class PeerConnection : public std::enable_shared_from_this<PeerConnection> {
public:
    using MessageHandler = std::function<void(std::shared_ptr<PeerConnection>, PeerMessage)>;

    PeerConnection(EventLoop& loop, std::shared_ptr<Peer> peer, 
                   const InfoHash& info_hash, const PeerId& my_id);
    ~PeerConnection();

    void                    start();
    void                    disconnect();
    void                    queue_message(const PeerMessage& msg);
    
    void                    set_message_handler(MessageHandler handler) { message_handler_ = std::move(handler); }

    ConnectionState         get_state() const { return state_; }
    std::shared_ptr<Peer>   get_peer() const { return peer_; }

private:
    EventLoop&              loop_;
    std::shared_ptr<Peer>   peer_;
    TcpSocket               socket_;
    ConnectionState         state_;

    InfoHash                my_info_hash_;
    PeerId                  my_peer_id_;

    std::vector<uint8_t>    read_buffer_;
    std::vector<uint8_t>    write_buffer_;
    MessageHandler          message_handler_;

    void                    on_socket_event(uint32_t events);
    void                    handle_read();
    void                    handle_write();
    bool                    process_handshake();
};