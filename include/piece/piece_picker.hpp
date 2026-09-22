#pragma once

#include <vector>
#include <cstdint>
#include <unordered_set>
#include "../peer/peer.hpp"

class PiecePicker {
public:
    PiecePicker(uint32_t total_pieces);

    void    mark_completed(uint32_t piece_index);
    int     pick_next_piece(const Peer& peer) const;
    int     pick_next_piece(const Peer& peer,
                            const std::unordered_set<uint32_t>& skip) const;

private:
    uint32_t            total_pieces_;
    std::vector<bool>   completed_pieces_;
};