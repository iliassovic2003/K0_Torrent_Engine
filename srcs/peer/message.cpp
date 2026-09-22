#include "../../include/peer/message.hpp"

static void     write_u32(uint8_t* p, uint32_t v) {
    p[0] = (v >> 24) & 0xFF;
    p[1] = (v >> 16) & 0xFF;
    p[2] = (v >> 8)  & 0xFF;
    p[3] = v & 0xFF;
}

static uint32_t read_u32(const uint8_t* p) {
    return (static_cast<uint32_t>(p[0]) << 24) |
           (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8)  |
            static_cast<uint32_t>(p[3]);
}

std::vector<uint8_t> PeerMessage::serialize() const {
    uint32_t length = 1 + static_cast<uint32_t>(payload.size());
    
    std::vector<uint8_t>    buf(4 + length);
    write_u32(buf.data(), length);

    buf[4] = static_cast<uint8_t>(id);
    if (!payload.empty())
        std::copy(payload.begin(), payload.end(), buf.begin() + 5);
    
    return buf;
}

std::vector<uint8_t> PeerMessage::keep_alive() {
    std::vector<uint8_t> buf(4, 0); // 4 bytes of zeros
    return buf;
}

ParseResult parse_peer_message(const std::vector<uint8_t>& buffer) {
    ParseResult     result{};
    result.complete = false;
    
    if (buffer.size() < 4)
        return result; 

    uint32_t length = read_u32(buffer.data());

    if (length == 0) {
        result.complete = true;
        result.consumed_bytes = 4;
        result.is_keep_alive = true;

        return result;
    }

    if (buffer.size() < 4 + length) {
        return result;
    }

    result.complete = true;
    result.consumed_bytes = 4 + length;
    result.is_keep_alive = false;
    
    MessageId id = static_cast<MessageId>(buffer[4]);
    
    std::vector<uint8_t> payload;
    if (length > 1)
        payload.assign(buffer.begin() + 5, buffer.begin() + 4 + length);

    result.message = PeerMessage(id, std::move(payload));
    return result;
}