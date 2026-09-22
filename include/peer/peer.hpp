#pragma once

#include "../common/types.hpp"
# include <vector>
# include <cstddef>
# include <cstdint>

class Peer {
public:
    Peer(const PeerAddress& addr);

    PeerAddress address;
    PeerId      id{};
    bool        id_set = false;

    bool        am_choking      = true;  // We are choking the peer
    bool        am_interested   = false; // We are interested in the peer
    bool        peer_choking    = true;  // The peer is choking us
    bool        peer_interested = false; // The peer is interested in us

    std::vector<uint8_t> bitfield;

    bool has_piece(uint32_t piece_index) const;
    void set_piece(uint32_t piece_index);
};