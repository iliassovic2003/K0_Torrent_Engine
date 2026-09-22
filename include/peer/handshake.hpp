#pragma once

# include "../common/types.hpp"
# include "../common/error.hpp"
# include <array>
# include <cstdint>
# include <cstring>

struct Handshake {
    static constexpr uint8_t        PROTOCOL_LEN = 19;
    static constexpr const char*    PROTOCOL_STRING = "BitTorrent protocol";
    static constexpr size_t         HANDSHAKE_SIZE = 68;

    std::array<uint8_t, 8>          reserved; // no DHT
    InfoHash                        info_hash;
    PeerId                          peer_id;

    Handshake() = default;
    Handshake(const InfoHash& ih, const PeerId& pid);

    std::array<uint8_t, HANDSHAKE_SIZE> serialize() const;
    static Handshake                    deserialize(const std::array<uint8_t, HANDSHAKE_SIZE>& buffer);
};