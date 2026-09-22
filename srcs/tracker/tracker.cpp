#include "../../include/tracker/tracker.hpp"

std::string url_encode(const uint8_t* data, size_t len) {
    std::ostringstream ss;
    ss << std::hex;
    for (size_t i = 0; i < len; ++i) {
        ss << '%';
        ss.width(2);
        ss.fill('0');
        ss << static_cast<unsigned>(data[i]);
    }
    return ss.str();
}

std::string url_encode_hash(const InfoHash& h) {
    return url_encode(h.data(), h.size());
}

std::string url_encode_peer_id(const PeerId& id) {
    return url_encode(id.data(), id.size());
}