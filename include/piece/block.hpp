#pragma once

# include <cstdint>
# include <vector>

constexpr uint32_t BLOCK_SIZE = 16 * 1024;

enum class BlockState {
    Missing,
    Requested,
    Downloaded
};

struct Block {
    uint32_t index;       // Which block within the piece is this? (0, 1, 2...)
    uint32_t offset;      // Byte offset within the piece (index * BLOCK_SIZE)
    uint32_t length;

    BlockState state = BlockState::Missing;
    std::vector<uint8_t> data;
};