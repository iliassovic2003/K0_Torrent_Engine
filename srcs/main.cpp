# include "../include/session/session.hpp"
# include "../include/common/logger.hpp"
# include <iostream>
# include <csignal>
# include <memory>

static constexpr const char* TAG = "K0_Main";

std::unique_ptr<Session> g_session = nullptr;

void handle_sigint(int signum) {
    std::cout << "\n";
    LOG_I(TAG, "Caught signal %d (Ctrl+C). Shutting down gracefully...", signum);
    if (g_session) {
        g_session->stop();
    }
    std::_Exit(0);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_torrent_file>\n";
        return 1;
    }

    std::signal(SIGINT, handle_sigint);

    try {
        g_session = std::make_unique<Session>(argv[1]);
        g_session->start();
    } catch (const std::exception& e) {
        LOG_E(TAG, "Fatal error: %s", e.what());
        return 1;
    }

    LOG_I(TAG, "Client terminated successfully.");
    return 0;
}