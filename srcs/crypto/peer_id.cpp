#include "../../include/crypto/peer_id.hpp"

PeerId generate_peer_id() {
    PeerId id;

    const char* myPrefix = "-KU0500-";  // better to manual update it each changelog version

    std::memcpy(id.data(), myPrefix, 8);

    std::random_device              rd;
    std::mt19937                    rng(rd());
    std::uniform_int_distribution<> dist(0, 255);

    for (size_t i = 8; i < 20; i++)
        id[i] = static_cast<uint8_t>(dist(rng));

    return id;
}