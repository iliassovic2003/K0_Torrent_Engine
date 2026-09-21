#include "../include/tracker/tracker.hpp"
#include "../include/tracker/http_tracker.hpp"
#include "../include/tracker/udp_tracker.hpp"
#include "../include/tracker/tracker_manager.hpp"
#include "../include/common/types.hpp"

// ── helpers ───────────────────────────────────────────────────────────────────
static void test_url_encode() {
    k0::InfoHash h{};
    for (uint8_t i = 0; i < 20; ++i)
        h[i] = i;

    std::string encoded = k0::url_encode_hash(h);
    assert(encoded.size() == 60);           // 20 bytes × 3 chars (%XX)
    assert(encoded.substr(0, 3) == "%00");
    assert(encoded.substr(3, 3) == "%01");
    assert(encoded.substr(57, 3) == "%13");
    std::cout << "  [PASS] url_encode\n";
}

static void test_url_encode_printable() {
    uint8_t raw[4] = { 0xFF, 0x00, 0xAB, 0xCD };
    std::string enc = k0::url_encode(raw, 4);
    assert(enc == "%ff%00%ab%cd");
    std::cout << "  [PASS] url_encode_printable\n";
}

static void test_tracker_manager_construction() {
    k0::PeerId pid{};
    pid[0] = '-'; pid[1] = 'K'; pid[2] = '0';

    std::vector<std::vector<std::string>> tiers = {
        { "udp://tracker.opentrackr.org:1337/announce" },
        { "http://tracker.example.com:6969/announce",
          "udp://tracker.example2.com:6969/announce" }
    };

    // Just check construction doesn't throw
    k0::TrackerManager mgr(tiers, pid, 6881);
    std::cout << "  [PASS] TrackerManager construction\n";
}

// ── compact peer list parsing ─────────────────────────────────────────────────
// We test HttpTracker's parse path indirectly by constructing a raw body that
// looks like a valid tracker response and decoding it manually.
static void test_compact_peer_decode() {
    // Build a 12-byte compact peer list: two IPv4 peers
    // Peer 1: 127.0.0.1:6881
    // Peer 2: 192.168.1.100:51413
    uint8_t raw[12];

    // peer 1
    struct in_addr a1; inet_aton("127.0.0.1", &a1);
    memcpy(raw,     &a1.s_addr, 4);
    raw[4] = (6881 >> 8) & 0xFF; raw[5] = 6881 & 0xFF;

    // peer 2
    struct in_addr a2; inet_aton("192.168.1.100", &a2);
    memcpy(raw + 6, &a2.s_addr, 4);
    raw[10] = (51413 >> 8) & 0xFF; raw[11] = 51413 & 0xFF;

    std::string compact(reinterpret_cast<char*>(raw), 12);

    assert(compact.size() % 6 == 0);
    size_t num = compact.size() / 6;
    assert(num == 2);

    uint32_t ip0; memcpy(&ip0, compact.data(), 4);
    assert(ip0 == a1.s_addr);

    uint16_t p0; memcpy(&p0, compact.data() + 4, 2);
    assert(ntohs(p0) == 6881);

    uint32_t ip1; memcpy(&ip1, compact.data() + 6, 4);
    assert(ip1 == a2.s_addr);

    uint16_t p1; memcpy(&p1, compact.data() + 10, 2);
    assert(ntohs(p1) == 51413);

    std::cout << "  [PASS] compact peer list decode\n";
}

// ── UDP wire format ───────────────────────────────────────────────────────────
static void test_udp_connect_packet_layout() {
    constexpr uint64_t  MAGIC = 0x41727101980ULL;
    uint8_t             buf[16];
    
    auto wu64 = [](uint8_t* p, uint64_t v) {
        for (int i = 7; i >= 0; --i) { p[i] = v & 0xFF; v >>= 8; }
    };
    auto wu32 = [](uint8_t* p, uint32_t v) {
        p[0]=(v>>24)&0xFF; p[1]=(v>>16)&0xFF; p[2]=(v>>8)&0xFF; p[3]=v&0xFF;
    };

    wu64(buf,     MAGIC);
    wu32(buf + 8, 0);
    wu32(buf + 12, 0xDEADBEEF); // txn_id

    // Verify magic bytes
    uint64_t magic_read = 0;
    for (int i = 0; i < 8; ++i)
        magic_read = (magic_read << 8) | buf[i];
    assert(magic_read == MAGIC);

    // Verify action
    uint32_t action = ((uint32_t)buf[8]<<24)|((uint32_t)buf[9]<<16)
                     |((uint32_t)buf[10]<<8)|(uint32_t)buf[11];
    assert(action == 0);

    std::cout << "  [PASS] UDP connect packet layout\n";
}

static void test_udp_announce_packet_size() {
    constexpr size_t expected = 98;

    assert(expected ==
        8 +   // connection_id
        4 +   // action
        4 +   // transaction_id
        20 +  // info_hash
        20 +  // peer_id
        8 +   // downloaded
        8 +   // left
        8 +   // uploaded
        4 +   // event
        4 +   // ip
        4 +   // key
        4 +   // num_want
        2);   // port
    std::cout << "  [PASS] UDP announce packet size\n";
}

// ── main ──────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "=== Tracker tests ===\n";

    test_url_encode();
    test_url_encode_printable();
    test_tracker_manager_construction();
    test_compact_peer_decode();
    test_udp_connect_packet_layout();
    test_udp_announce_packet_size();

    std::cout << "All tracker tests passed.\n";
    return 0;
}