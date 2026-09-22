#pragma once

# include <cstdint>
# include <vector>
# include <stdexcept>
# include <string>

enum class MessageId : uint8_t {
    Choke         = 0,
    Unchoke       = 1,
    Interested    = 2,
    NotInterested = 3,
    Have          = 4,
    Bitfield      = 5,
    Request       = 6,
    Piece         = 7,
    Cancel        = 8
};

struct PeerMessage {
    MessageId               id;
    std::vector<uint8_t>    payload;

    PeerMessage(MessageId id, std::vector<uint8_t> payload = {})
        : id(id), payload(std::move(payload)) {}

    std::vector<uint8_t>        serialize() const;

    static std::vector<uint8_t> keep_alive();
};

struct ParseResult {
    bool complete;          // Do we have a full message?
    size_t consumed_bytes;  // How many bytes to remove from the read buffer
    bool is_keep_alive;     // Is it just a keep-alive ping?
    PeerMessage message;    // The parsed message (if complete and not keep-alive)
};

ParseResult parse_peer_message(const std::vector<uint8_t>& buffer);