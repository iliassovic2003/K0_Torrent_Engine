#include "../include/net/event_loop.hpp"
#include "../include/peer/peer_connection.hpp"
#include <iostream>
#include <cassert>
#include <thread>
#include <arpa/inet.h>
#include <unistd.h>

static void test_async_handshake() {
    InfoHash dummy_hash; dummy_hash.fill(0xAA);
    PeerId dummy_id;     dummy_id.fill(0xBB);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(9999);

    bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_fd, 1);

    EventLoop loop;
    PeerAddress target_addr{inet_addr("127.0.0.1"), 9999};
    auto peer = std::make_shared<Peer>(target_addr);
    
    auto pc = std::make_shared<PeerConnection>(loop, peer, dummy_hash, dummy_id);
    
    pc->set_message_handler([&](std::shared_ptr<PeerConnection>, PeerMessage) {});

    pc->start();

    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
    assert(client_fd != -1);

    loop.run_once(100);

    uint8_t recv_buf[68];
    ssize_t n = recv(client_fd, recv_buf, sizeof(recv_buf), MSG_WAITALL);
    assert(n == 68);
    assert(recv_buf[0] == 19);

    Handshake response_hs(dummy_hash, dummy_id);
    auto response_buf = response_hs.serialize();
    send(client_fd, response_buf.data(), response_buf.size(), 0);

    loop.run_once(100);

    assert(pc->get_state() == ConnectionState::Connected);
    assert(pc->get_peer()->id_set == true);

    std::cout << "  [PASS] Async PeerConnection & EventLoop handshake\n";

    close(client_fd);
    close(server_fd);
}

int main() {
    std::cout << "=== Async Network tests ===\n";
    test_async_handshake();
    std::cout << "All async network tests passed.\n";
    return 0;
}