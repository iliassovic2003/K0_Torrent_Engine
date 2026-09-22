#include "../include/piece/piece_manager.hpp"
#include "../include/crypto/sha1.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <cassert>
#include <vector>
#include <string>

static uint32_t read_u32(const std::vector<uint8_t>& buf, size_t offset) {
    return (buf[offset] << 24) | (buf[offset + 1] << 16) | 
           (buf[offset + 2] << 8) | buf[offset + 3];
}

static void test_piece_manager_logic() {
    // 1. Create fake data for a single 32KB piece (which requires TWO 16KB blocks)
    std::vector<uint8_t> block0(16384, 'A');
    std::vector<uint8_t> block1(16384, 'B'); 
    
    std::string full_piece_data;
    full_piece_data.append(reinterpret_cast<char*>(block0.data()), block0.size());
    full_piece_data.append(reinterpret_cast<char*>(block1.data()), block1.size());

    // 2. Hash the fake data so the PieceManager's verification will actually pass
    std::array<uint8_t, 20> real_hash = sha1_raw(full_piece_data);

    // 3. Mock the TorrentFile struct
    TorrentFile torrent;
    torrent.piece_length = 32768; // 32 KB
    
    // Push the hash array directly into the pieces vector
    torrent.pieces.push_back(real_hash); 
    
    // Create the FileInfo struct and add it
    FileInfo fi;
    fi.path.push_back("dummy.mp4");
    fi.length = 32768;
    torrent.files.push_back(fi);

    // 4. Initialize the Piece Manager
    PieceManager pm(torrent);

    // 5. Mock a Peer that has Piece 0
    PeerAddress addr;
    addr.ip = inet_addr("127.0.0.1"); // Convert string IP to uint32_t network byte order
    addr.port = 1234;
    Peer peer(addr);
    peer.set_piece(0);

    // --- TEST PHASE 1: GENERATING REQUESTS ---
    
    auto req1 = pm.create_next_request(peer);
    assert(req1 != nullptr);
    assert(req1->id == MessageId::Request);
    assert(read_u32(req1->payload, 0) == 0);
    assert(read_u32(req1->payload, 4) == 0);
    assert(read_u32(req1->payload, 8) == 16384);

    auto req2 = pm.create_next_request(peer);
    assert(req2 != nullptr);
    assert(read_u32(req2->payload, 4) == 16384);

    auto req3 = pm.create_next_request(peer);
    assert(req3 == nullptr); 

    // --- TEST PHASE 2: PROCESSING DATA & VERIFICATION ---
    
    bool is_done1 = pm.process_block(0, 0, block0);
    assert(is_done1 == false);

    bool is_done2 = pm.process_block(0, 16384, block1);
    assert(is_done2 == true);

    std::cout << "  [PASS] Piece Picker sequential logic & SHA-1 Verification\n";
}

int main() {
    std::cout << "=== Piece Manager tests ===\n";
    test_piece_manager_logic();
    std::cout << "All Piece Manager tests passed.\n";
    return 0;
}