#pragma once

#include "block.hpp"
#include "piece_picker.hpp"
#include "../torrent/torrent_file.hpp"
#include "../peer/message.hpp"
#include "../crypto/sha1.hpp"
#include <unordered_map>
#include <memory>

struct ActivePiece {
    uint32_t            index;
    uint32_t            total_blocks;
    uint32_t            blocks_downloaded;
    std::vector<Block>  blocks;
};

class PieceManager {
public:
    PieceManager(const TorrentFile& torrent);

    std::unique_ptr<PeerMessage> create_next_request(const Peer& peer);

    bool                        process_block(uint32_t piece_index, uint32_t block_offset, const std::vector<uint8_t>& data);

    std::vector<uint8_t>        get_completed_piece(uint32_t piece_index) const;

private:
    TorrentFile         torrent_;
    PiecePicker         picker_;
    
    std::unordered_map<uint32_t, ActivePiece> active_pieces_;

    void                        init_active_piece(uint32_t piece_index);
    bool                        verify_piece_hash(uint32_t piece_index) const;
};