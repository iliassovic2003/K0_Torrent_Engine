#include "../include/peer/handshake.hpp"
#include "../include/peer/message.hpp"
#include "../include/peer/peer.hpp"
#include <iostream>
#include <cassert>
#include <cstring>

static void test_handshake_serialization() {
    InfoHash ih;
    PeerId pid;
    ih.fill(0xAA);
    pid.fill(0xBB);

    Handshake hs(ih, pid);
    auto buf = hs.serialize();

    assert(buf.size() == 68);
    assert(buf[0] == 19);
    assert(std::memcmp(&buf[1], "BitTorrent protocol", 19) == 0);

    Handshake decoded = Handshake::deserialize(buf);
    assert(decoded.info_hash == ih);
    assert(decoded.peer_id == pid);

    std::cout << "  [PASS] Handshake serialization & parsing\n";
}

static void test_message_framing() {
    std::vector<uint8_t> payload = {0x00, 0x00, 0x00, 0x05};
    PeerMessage msg(MessageId::Have, payload);
    
    auto wire_data = msg.serialize();
    
    assert(wire_data.size() == 4 + 5); 
    assert(wire_data[3] == 5)
    assert(wire_data[4] == 4);

    std::vector<uint8_t> buffer_sim(wire_data.begin(), wire_data.begin() + 3);
    ParseResult res1 = parse_peer_message(buffer_sim);
    assert(res1.complete == false);

    buffer_sim.assign(wire_data.begin(), wire_data.begin() + 6);
    ParseResult res2 = parse_peer_message(buffer_sim);
    assert(res2.complete == false);

    ParseResult res3 = parse_peer_message(wire_data);
    assert(res3.complete == true);
    assert(res3.is_keep_alive == false);
    assert(res3.consumed_bytes == wire_data.size());
    assert(res3.message.id == MessageId::Have);
    assert(res3.message.payload == payload);

    std::cout << "  [PASS] Message length-prefix framing & TCP fragmentation\n";
}

static void test_peer_bitfield() {
    PeerAddress dummy_addr{0, 0};
    Peer p(dummy_addr);

    assert(p.has_piece(0) == false);
    assert(p.has_piece(10) == false);

    p.set_piece(10);
    assert(p.has_piece(10) == true);
    assert(p.has_piece(9) == false);
    assert(p.has_piece(11) == false);

    p.set_piece(100);
    assert(p.has_piece(100) == true);

    std::cout << "  [PASS] Peer bitfield tracking\n";
}

int main() {
    std::cout << "=== Peer Wire tests ===\n";

    test_handshake_serialization();
    test_message_framing();
    test_peer_bitfield();

    std::cout << "All peer wire tests passed.\n";
    return 0;
}