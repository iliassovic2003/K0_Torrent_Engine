#include "../../include/piece/piece_manager.hpp"
#include "../../include/common/logger.hpp"
#include <algorithm>
#include <cstring>

static constexpr const char* TAG = "K0_PieceMr";

PieceManager::PieceManager(const TorrentFile& torrent)
    : torrent_(torrent), 
      picker_(torrent.pieces.size()) 
{}

void PieceManager::init_active_piece(uint32_t piece_index) {
    ActivePiece     active;

    active.index = piece_index;
    active.blocks_downloaded = 0;

    uint32_t total_torrent_size = 0;
    for (const auto& f : torrent_.files)
        total_torrent_size += f.length;

    uint32_t piece_length = torrent_.piece_length;
    uint32_t offset_in_torrent = piece_index * piece_length;

    if (offset_in_torrent + piece_length > total_torrent_size)
        piece_length = total_torrent_size - offset_in_torrent;

    uint32_t blocks = (piece_length + BLOCK_SIZE - 1) / BLOCK_SIZE;
    active.total_blocks = blocks;

    for (uint32_t i = 0; i < blocks; ++i) {
        Block       b;

        b.index = i;
        b.offset = i * BLOCK_SIZE;
        b.length = std::min(BLOCK_SIZE, piece_length - b.offset);
        b.state = BlockState::Missing;
        active.blocks.push_back(b);
    }

    active_pieces_[piece_index] = std::move(active);
}

std::unique_ptr<PeerMessage> PieceManager::create_next_request(const Peer& peer) {
    int         next_piece = picker_.pick_next_piece(peer);
    if (next_piece == -1)
        return nullptr;

    uint32_t    p_idx = static_cast<uint32_t>(next_piece);

    if (active_pieces_.find(p_idx) == active_pieces_.end())
        init_active_piece(p_idx);

    ActivePiece& active = active_pieces_[p_idx];

    for (auto& block : active.blocks) {
        if (block.state == BlockState::Missing) {
            block.state = BlockState::Requested;

            std::vector<uint8_t> payload(12);
            
            payload[0] = (p_idx >> 24) & 0xFF; payload[1] = (p_idx >> 16) & 0xFF;
            payload[2] = (p_idx >> 8) & 0xFF;  payload[3] = p_idx & 0xFF;

            payload[4] = (block.offset >> 24) & 0xFF; payload[5] = (block.offset >> 16) & 0xFF;
            payload[6] = (block.offset >> 8) & 0xFF;  payload[7] = block.offset & 0xFF;

            payload[8] = (block.length >> 24) & 0xFF; payload[9] = (block.length >> 16) & 0xFF;
            payload[10] = (block.length >> 8) & 0xFF; payload[11] = block.length & 0xFF;

            return std::make_unique<PeerMessage>(MessageId::Request, payload);
        }
    }

    return nullptr; 
}

bool PieceManager::process_block(uint32_t piece_index, uint32_t block_offset, const std::vector<uint8_t>& data) {
    if (active_pieces_.find(piece_index) == active_pieces_.end())
        return false;

    ActivePiece& active = active_pieces_[piece_index];

    for (auto& block : active.blocks) {
        if (block.offset == block_offset && block.state != BlockState::Downloaded) {
            block.data = data;
            block.state = BlockState::Downloaded;
            active.blocks_downloaded++;

            if (active.blocks_downloaded == active.total_blocks) {
                if (verify_piece_hash(piece_index)) {
                    picker_.mark_completed(piece_index);
                    LOG_I(TAG, "Piece %u downloaded and verified!", piece_index);
                    return true;
                } else {
                    LOG_W(TAG, "Piece %u FAILED HASH CHECK! Throwing away data.", piece_index);
                    active.blocks_downloaded = 0;
                    for (auto& b : active.blocks) {
                        b.state = BlockState::Missing;
                        b.data.clear();
                    }
                    return false;
                }
            }
            return false;
        }
    }
    return false;
}

std::vector<uint8_t> PieceManager::get_completed_piece(uint32_t piece_index) const {
    std::vector<uint8_t>    full_piece;

    auto it = active_pieces_.find(piece_index);
    if (it != active_pieces_.end())
        for (const auto& block : it->second.blocks)
            full_piece.insert(full_piece.end(), block.data.begin(), block.data.end());
    
    return full_piece;
}

bool PieceManager::verify_piece_hash(uint32_t piece_index) const {
    std::vector<uint8_t> full_piece = get_completed_piece(piece_index);

    std::string data_str(reinterpret_cast<const char*>(full_piece.data()), full_piece.size());
    std::array<uint8_t, 20> calculated_hash = sha1_raw(data_str);

    std::array<uint8_t, 20> expected_hash = torrent_.pieces[piece_index];
    
    return calculated_hash == expected_hash;
}