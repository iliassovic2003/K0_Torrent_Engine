#include "../../include/peer/handshake.hpp"

Handshake::Handshake(const InfoHash& ih, const PeerId& pid)
    : info_hash(ih), peer_id(pid) {}

std::array<uint8_t, Handshake::HANDSHAKE_SIZE> Handshake::serialize() const {
    std::array<uint8_t, HANDSHAKE_SIZE> buf;
    size_t offset = 0;

    buf[offset++] = PROTOCOL_LEN;

    std::memcpy(buf.data() + offset, PROTOCOL_STRING, PROTOCOL_LEN);
    offset += PROTOCOL_LEN;

    std::memcpy(buf.data() + offset, reserved.data(), reserved.size());
    offset += reserved.size();

    std::memcpy(buf.data() + offset, info_hash.data(), info_hash.size());
    offset += info_hash.size();

    std::memcpy(buf.data() + offset, peer_id.data(), peer_id.size());
    return buf;
}

Handshake Handshake::deserialize(const std::array<uint8_t, Handshake::HANDSHAKE_SIZE>& buffer) {
    if (buffer[0] != PROTOCOL_LEN) {
        throw ParseError("Invalid handshake protocol length");
    }
    
    if (std::memcmp(buffer.data() + 1, PROTOCOL_STRING, PROTOCOL_LEN) != 0) {
        throw ParseError("Invalid handshake protocol string");
    }

    Handshake   hs;
    size_t      offset = 1 + PROTOCOL_LEN;

    std::memcpy(hs.reserved.data(), buffer.data() + offset, hs.reserved.size());
    offset += hs.reserved.size();

    std::memcpy(hs.info_hash.data(), buffer.data() + offset, hs.info_hash.size());
    offset += hs.info_hash.size();

    std::memcpy(hs.peer_id.data(), buffer.data() + offset, hs.peer_id.size());
    return hs;
}