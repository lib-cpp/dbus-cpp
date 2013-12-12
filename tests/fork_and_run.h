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

#include <gtest/gtest.h>

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

int fork_and_run(const std::function<void()>& service, const std::function<void()>& client)
{
    auto service_pid = fork();

    if (service_pid < 0)
    {
        throw std::runtime_error(std::string("Could not fork child for service: ") + std::strerror(errno));
    } else if (is_child(service_pid))
    {
        std::cout << "Running the service now: " << getpid() << std::endl;
        service();
        exit(EXIT_SUCCESS);
    } else // parent
    {
        auto client_pid = fork();

        if (client_pid < 0)
        {
            throw std::runtime_error(std::string("Could not fork child for client: ") + std::strerror(errno));
        } else if (is_child(client_pid))
        {
            std::cout << "Running the client now: " << getpid() << std::endl;

            try
            {
                client();
            } catch(...)
            {
                exit(EXIT_FAILURE);
            }

            exit(::testing::Test::HasFatalFailure() || ::testing::Test::HasNonfatalFailure() ? EXIT_FAILURE : EXIT_SUCCESS);
        } else // parent
        {
            std::cout << "In the test process: " << getpid() << std::endl;
            int status;
            auto result = waitpid(client_pid, &status, WUNTRACED);

            if (result == -1)
            {
                throw std::runtime_error(std::string("Error waiting for child to complete: ") + std::strerror(errno));
            }

            ::kill(service_pid, SIGKILL);

            int return_status = EXIT_FAILURE;
            if (WIFEXITED(status))
            {
                std::cout << "Client exited with status: " << WEXITSTATUS(status) << std::endl;
                return_status = WEXITSTATUS(status) == EXIT_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
            } else if (WIFSIGNALED(status))
            {
                return_status = EXIT_FAILURE;
            }

            EXPECT_EQ(EXIT_SUCCESS, return_status);
            return return_status;
        }
    }
}
}

#endif // FORK_AND_RUN_H_
