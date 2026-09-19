#pragma once
# include "tcp_socket.hpp"

# include <atomic>
# include <cstdint>
# include <functional>
# include <unordered_map>
# include <vector>
# include <poll.h>
# include <cerrno>
# include <cstring>
# include <stdexcept>
# include <unistd.h>

namespace Event {
    constexpr uint32_t READ  = POLLIN;
    constexpr uint32_t WRITE = POLLOUT;
    constexpr uint32_t ERROR = POLLERR | POLLHUP;
    constexpr uint32_t ALL   = READ | WRITE | ERROR;
}

class EventLoop {
public:
    using Callback = std::function<void(int fd, uint32_t events)>;

    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop&)            = delete;
    EventLoop& operator=(const EventLoop&) = delete;
    EventLoop(EventLoop&&)                 = delete;
    EventLoop& operator=(EventLoop&&)      = delete;

    void    add(int fd, uint32_t events, Callback cb);
    void    modify(int fd, uint32_t events);
    void    remove(int fd);

    int     run_once(int timeout_ms = -1);
    void    run();
    void    stop();

    bool    is_running() const {
        return running_.load(std::memory_order_relaxed);
    }

private:
    void rebuild_pollfds();

    struct Entry {
        uint32_t events;
        Callback cb;
    };

    std::unordered_map<int, Entry>  registered_;
    std::vector<pollfd>             pollfds_;
    bool                            dirty_;

    int                             pipe_read_;
    int                             pipe_write_;

    std::atomic<bool>               stop_flag_;
    std::atomic<bool>               running_;
};