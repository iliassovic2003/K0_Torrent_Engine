#pragma once

# include <cstdint>
# include <array>

using InfoHash = std::array<uint8_t, 20>;
using PeerId   = std::array<uint8_t, 20>;

struct PeerAddress {
    uint32_t ip;
    uint16_t port;
};