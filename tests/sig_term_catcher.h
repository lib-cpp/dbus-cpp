/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */

#ifndef CORE_TESTING_SIG_TERM_CATCHER_H_
#define CORE_TESTING_SIG_TERM_CATCHER_H_

#include <system_error>

#include <poll.h>
#include <sys/signalfd.h>

namespace core
{
namespace testing
{
struct SigTermCatcher
{
    inline SigTermCatcher()
    {
        sigemptyset(&signal_mask);

        if (-1 == sigaddset(&signal_mask, SIGTERM))
            throw std::system_error(errno, std::system_category());

        if (-1 == sigprocmask(SIG_BLOCK, &signal_mask, NULL))
            throw std::system_error(errno, std::system_category());

        if (-1 == (signal_fd = signalfd(-1, &signal_mask, 0)))
            throw std::system_error(errno, std::system_category());
    }

    inline ~SigTermCatcher()
    {
        ::close(signal_fd);
    }

    inline void wait_for_signal_for(const std::chrono::milliseconds& ms)
    {
        pollfd fd; fd.fd = signal_fd; fd.events = POLLIN;
        if (poll(&fd, 1, ms.count()) > 0)
        {
            signalfd_siginfo siginfo;
            if (-1 == ::read(signal_fd, &siginfo, sizeof(siginfo)))
                throw std::system_error(errno, std::system_category());
        } else
        {
            throw std::system_error(errno, std::system_category());
        }
    }

    sigset_t signal_mask;
    int signal_fd = -1;
};
}
}

#endif // CORE_TESTING_SIG_TERM_CATCHER_H_
