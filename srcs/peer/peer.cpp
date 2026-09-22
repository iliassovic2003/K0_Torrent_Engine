#include "../../include/peer/peer.hpp"

Peer::Peer(const PeerAddress& addr) : address(addr) {}

bool Peer::has_piece(uint32_t piece_index) const {
    size_t byte_index = piece_index / 8;
    size_t bit_offset = 7 - (piece_index % 8); // Big-endian bit ordering

    if (byte_index >= bitfield.size()) {
        return false;
    }
    return (bitfield[byte_index] & (1 << bit_offset)) != 0;
}

void Peer::set_piece(uint32_t piece_index) {
    size_t byte_index = piece_index / 8;
    size_t bit_offset = 7 - (piece_index % 8);

    if (byte_index >= bitfield.size())
        bitfield.resize(byte_index + 1, 0);
    bitfield[byte_index] |= (1 << bit_offset);
}