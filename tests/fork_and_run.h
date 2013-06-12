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

#ifndef FORK_AND_RUN_H_
#define FORK_AND_RUN_H_

#include <cstring>
#include <functional>
#include <stdexcept>

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

namespace test
{
bool is_child(pid_t pid)
{
    return pid == 0;
}

int fork_and_run(std::function<void()> child, std::function<void()> parent)
{
    auto pid = fork();

    if (pid < 0)
    {
        throw std::runtime_error(std::string("Could not fork child: ") + std::strerror(errno));
    }

    if (is_child(pid))
    {
        child();
        return EXIT_SUCCESS;
    }
    else
    {
        parent();
        kill(pid, SIGKILL);
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
}

#endif // FORK_AND_RUN_H_
