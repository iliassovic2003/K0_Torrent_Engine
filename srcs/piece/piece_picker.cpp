#include "../../include/piece/piece_picker.hpp"
#include "../../include/common/logger.hpp"

PiecePicker::PiecePicker(uint32_t total_pieces)
    : total_pieces_(total_pieces)
{
    completed_pieces_.resize(total_pieces_, false);
}

void PiecePicker::mark_completed(uint32_t piece_index) {
    if (piece_index < total_pieces_)
        completed_pieces_[piece_index] = true;
}

int PiecePicker::pick_next_piece(const Peer& peer,
                                  const std::unordered_set<uint32_t>& skip) const {
    for (uint32_t i = 0; i < total_pieces_; ++i) {
        if (completed_pieces_[i])
            continue;
        if (skip.count(i))
            continue;
        if (!peer.has_piece(i))
            continue;
        return static_cast<int>(i);
    }
    return -1;
}