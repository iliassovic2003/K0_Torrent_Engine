#pragma once

# include <vector>
# include <cstdint>
# include "../peer/peer.hpp"

class PiecePicker {
public:
    PiecePicker(uint32_t total_pieces);

    void                mark_completed(uint32_t piece_index);
    int                 pick_next_piece(const Peer& peer) const;

private:
    uint32_t            total_pieces_;
    std::vector<bool>   completed_pieces_;
};