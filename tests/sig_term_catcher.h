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

#include <chrono>
#include <system_error>

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

        if (-1 == sigprocmask(SIG_BLOCK, &signal_mask, nullptr))
            throw std::system_error(errno, std::system_category());
    }

    inline ~SigTermCatcher()
    {
    }

    inline bool wait_for_signal()
    {
        int rc = -1; int signal = -1;
        rc = sigwait(&signal_mask, &signal);

        return rc != -1 && signal == SIGTERM;
    }

    sigset_t signal_mask;
};
}
}

#endif // CORE_TESTING_SIG_TERM_CATCHER_H_
