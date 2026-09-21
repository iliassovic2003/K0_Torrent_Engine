#include "../../include/net/event_loop.hpp"

static std::string errno_msg(const std::string& ctx)
{
    return ctx + ": " + std::strerror(errno);
}

EventLoop::EventLoop()
    : dirty_(false)
    , pipe_read_(-1)
    , pipe_write_(-1)
    , stop_flag_(false)
    , running_(false)
{
    int fds[2];
    if (::pipe(fds) == -1)
        throw NetError(errno_msg("pipe"));

    pipe_read_  = fds[0];
    pipe_write_ = fds[1];

    pollfds_.push_back({ pipe_read_, POLLIN, 0 });
}

EventLoop::~EventLoop()
{
    if (pipe_read_  != -1) ::close(pipe_read_);
    if (pipe_write_ != -1) ::close(pipe_write_);
}

void EventLoop::add(int fd, uint32_t events, Callback cb)
{
    if (registered_.count(fd))
        throw NetError("EventLoop::add: fd " + std::to_string(fd) + " already registered");

    registered_[fd] = { events, std::move(cb) };
    dirty_ = true;
}

void EventLoop::modify(int fd, uint32_t events)
{
    auto it = registered_.find(fd);
    if (it == registered_.end())
        throw NetError("EventLoop::modify: fd " + std::to_string(fd) + " not registered");

    it->second.events = events;
    dirty_ = true;
}

void EventLoop::remove(int fd)
{
    if (registered_.erase(fd))
        dirty_ = true;
}

void EventLoop::rebuild_pollfds()
{
    pollfds_.resize(1);
    pollfds_.reserve(1 + registered_.size());

    for (const auto& [fd, entry] : registered_) {
        pollfds_.push_back({ fd,
                             static_cast<short>(entry.events),
                             0 });
    }
    dirty_ = false;
}

int EventLoop::run_once(int timeout_ms)
{
    if (dirty_)
        rebuild_pollfds();

    for (auto& pfd : pollfds_)
        pfd.revents = 0;

    int ready;
    do {
        ready = ::poll(pollfds_.data(),
                       static_cast<nfds_t>(pollfds_.size()),
                       timeout_ms);
    } while (ready < 0 && errno == EINTR);

    if (ready < 0)
        throw NetError(errno_msg("poll"));

    if (ready == 0)
        return 0;

    if (pollfds_[0].revents & POLLIN) {
        char buf[64];
        while (::read(pipe_read_, buf, sizeof(buf)) > 0) {}
        --ready;
    }

    for (size_t i = 1; i < pollfds_.size() && ready > 0; ++i) {
        const short rev = pollfds_[i].revents;
        if (rev == 0)
            continue;

        --ready;
        const int fd = pollfds_[i].fd;

        auto it = registered_.find(fd);
        if (it == registered_.end())
            continue;

        it->second.cb(fd, static_cast<uint32_t>(rev));
    }

    return ready;
}

void EventLoop::run()
{
    running_.store(true, std::memory_order_relaxed);
    stop_flag_.store(false, std::memory_order_relaxed);

    while (!stop_flag_.load(std::memory_order_relaxed))
        run_once(-1);

    running_.store(false, std::memory_order_relaxed);
}

void EventLoop::stop()
{
    stop_flag_.store(true, std::memory_order_relaxed);

    const char wakeup = 1;
    ::write(pipe_write_, &wakeup, 1);
}