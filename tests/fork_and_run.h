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
