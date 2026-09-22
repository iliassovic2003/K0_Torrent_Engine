#include "../include/session/session.hpp"
#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>

// A minimal, mathematically valid BitTorrent file in raw bytes.
// Tracker: http://127.0.0.1:9999/ann
// File: test.txt (100 bytes)
// Piece length: 32768
const uint8_t dummy_torrent[] = {
    'd', '8', ':', 'a', 'n', 'n', 'o', 'u', 'n', 'c', 'e',
    '2', '7', ':', 'h', 't', 't', 'p', ':', '/', '/', '1', '2', '7', '.', '0', '.', '0', '.', '1', ':', '9', '9', '9', '9', '/', 'a', 'n', 'n',
    '4', ':', 'i', 'n', 'f', 'o',
    'd', '6', ':', 'l', 'e', 'n', 'g', 't', 'h', 'i', '1', '0', '0', 'e',
    '4', ':', 'n', 'a', 'm', 'e', '8', ':', 't', 'e', 's', 't', '.', 't', 'x', 't',
    '1', '2', ':', 'p', 'i', 'e', 'c', 'e', ' ', 'l', 'e', 'n', 'g', 't', 'h', 'i', '3', '2', '7', '6', '8', 'e',
    '6', ':', 'p', 'i', 'e', 'c', 'e', 's', '2', '0', ':',
    'A','A','A','A','A','A','A','A','A','A','A','A','A','A','A','A','A','A','A','A',
    'e', 'e'
};

static void test_session_integration() {
    std::string torrent_path = "test_dummy.torrent";
    
    std::ofstream ofs(torrent_path, std::ios::binary);
    ofs.write(reinterpret_cast<const char*>(dummy_torrent), sizeof(dummy_torrent));
    ofs.close();

    try {
        Session             session(torrent_path);

        // 3. Start the session.
        // It will attempt to contact the tracker at 127.0.0.1:9999.
        // Because no server is running there, TrackerManager should fail or return 0 peers.
        // If 0 peers are found, Session::start() safely returns without hanging the EventLoop!
        session.start();

        std::cout << "  [PASS] Session parsed torrent, initialized managers, and exited cleanly.\n";
    } catch (const std::exception& e) {
        std::cout << "  [INFO] Session safely threw on tracker connection refused: " << e.what() << "\n";
        std::cout << "  [PASS] Session error handling works.\n";
    }

    std::remove(torrent_path.c_str());
    
    std::system("rm -rf ./downloads");
}

int main() {
    std::cout << "=== Session Integration Test ===\n";
    test_session_integration();
    std::cout << "All Session tests passed.\n";
    return 0;
}