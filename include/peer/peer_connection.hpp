#pragma once

#include "peer.hpp"
#include "handshake.hpp"
#include "message.hpp"
#include <vector>
#include <memory>

enum class ConnectionState {
    Disconnected,
    Connecting,
    HandshakeSent,
    HandshakeReceived,
    Connected
};

class PeerConnection {
public:
    PeerConnection(std::shared_ptr<Peer> peer, const InfoHash& info_hash, const PeerId& my_id);
    ~PeerConnection();

    void                    connect_tcp();
    void                    disconnect();

    void                    send_handshake();
    void                    send_message(const PeerMessage& msg);
    
    std::vector<PeerMessage> receive_data(); 

    ConnectionState         get_state() const { return state_; }
    std::shared_ptr<Peer>   get_peer() const { return peer_; }

private:
    std::shared_ptr<Peer>   peer_;
    int                     socket_fd_;
    ConnectionState         state_;

    InfoHash                my_info_hash_;
    PeerId                  my_peer_id_;

    std::vector<uint8_t>    read_buffer_;
    std::vector<uint8_t>    write_buffer_;

    bool process_handshake();
};