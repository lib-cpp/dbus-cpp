#ifndef CROSS_PROCESS_SYNC_H_
#define CROSS_PROCESS_SYNC_H_

#include <cstring>
#include <stdexcept>

#include <unistd.h>

namespace test
{
struct CrossProcessSync
{
    static const int read_fd = 0;
    static const int write_fd = 1;

    CrossProcessSync()
    {
        if (pipe(fds) < 0)
            throw std::runtime_error(strerror(errno));
    }

    ~CrossProcessSync() noexcept
    {
        ::close(fds[0]);
        ::close(fds[1]);
    }

    void signal_ready()
    {
        int value = 42;
        if (!write(fds[write_fd], std::addressof(value), sizeof(value)))
            throw std::runtime_error(::strerror(errno));
    }

    void wait_for_signal_ready() const
    {
        int value;
        if (!read(fds[read_fd], std::addressof(value), sizeof(value)))
            throw std::runtime_error(::strerror(errno));
    }

    int fds[2];
};
}

#endif // CROSS_PROCESS_SYNC_H_
