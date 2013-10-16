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

#include "benchmark_service.h"

#include "org/freedesktop/dbus/announcer.h"
#include "org/freedesktop/dbus/resolver.h"
#include "org/freedesktop/dbus/asio/executor.h"
#include "org/freedesktop/dbus/types/stl/vector.h"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/moment.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include <cstdio>
#include <fstream>

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

namespace acc = boost::accumulators;
namespace dbus = org::freedesktop::dbus;

namespace
{
dbus::Bus::Ptr the_session_bus()
{
    dbus::Bus::Ptr session_bus = std::make_shared<dbus::Bus>(dbus::WellKnownBus::session);
    return session_bus;
}

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

    void wait_for_signal_ready()
    {
        int value;
        if (!read(fds[read_fd], std::addressof(value), sizeof(value)))
            throw std::runtime_error(::strerror(errno));
    }

    int fds[2];
};

bool is_child(pid_t pid)
{
    return pid == 0;
}

int fork_and_run(int argc, char** argv, std::function<int(int, char**)> child, std::function<int(int, char**, pid_t)> parent)
{
    auto pid = fork();

    if (pid < 0)
    {
        throw std::runtime_error(std::string("Could not fork child: ") + std::strerror(errno));
    }

    try
    {
        if (is_child(pid))
        {
            return child(argc, argv);
        }

        return parent(argc, argv, pid);
    }
    catch (...)
    {
        kill(pid, SIGKILL);
    }

    return EXIT_FAILURE;
}
}

int main(int argc, char** argv)
{
    CrossProcessSync cross_process_sync;

    auto server = [&cross_process_sync](int, char**)
    {
        auto bus = the_session_bus();
        bus->install_executor(org::freedesktop::dbus::asio::make_executor(bus));
        std::thread t1
        {
            [&]()
            {
                bus->run();
            }
        };
        test::BenchmarkService::Ptr benchmark_service = dbus::announce_service_on_bus<test::IBenchmarkService, test::BenchmarkService>(bus);
        cross_process_sync.signal_ready();
        if (t1.joinable())
            t1.join();
        return EXIT_SUCCESS;
    };

    auto client = [&cross_process_sync](int, char**, pid_t pid)
    {
        auto bus = the_session_bus();

        cross_process_sync.wait_for_signal_ready();

        auto stub = dbus::resolve_service_on_bus<test::IBenchmarkService, test::BenchmarkServiceStub>(bus);

        std::ofstream out("dbus_benchmark_int64_t.txt");
        acc::accumulator_set<double, acc::stats<acc::tag::mean, acc::tag::lazy_variance > > as;
        std::chrono::high_resolution_clock::time_point before;
        const int32_t default_value = 42;
        const unsigned int iteration_count = 10000;
        for (unsigned int i = 0; i < iteration_count; i++)
        {
            before = std::chrono::high_resolution_clock::now();
            auto value = stub->method_int64(default_value);
            if (value != default_value)
                return EXIT_FAILURE;
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - before);
            out << duration.count() << std::endl;
            as(duration.count());
        }

        std::cout << "MethodInt64 -> Mean: " << acc::mean(as) << " [µs], std. dev.: " << std::sqrt(acc::lazy_variance(as)) << " [µs]" << std::endl;

        out.close();
        out.open("dbus_benchmark_vector_int32_t.txt");
        as = acc::accumulator_set<double, acc::stats<acc::tag::mean, acc::tag::lazy_variance > >();
        const size_t element_count = 100;
        std::vector<int32_t> value(element_count, default_value);
        for (unsigned int i = 0; i < iteration_count; i++)
        {
            before = std::chrono::high_resolution_clock::now();
            stub->method_vector_int32(value);
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - before);
            out << duration.count() << std::endl;
            as(duration.count());
        }

        std::cout << "MethodVectorInt32 -> Mean: " << acc::mean(as) << " [µs], std. dev.: " << std::sqrt(acc::lazy_variance(as)) << " [µs]" << std::endl;

        kill(pid, SIGKILL);

        return EXIT_SUCCESS;
    };

    return fork_and_run(argc, argv, server, client);
}
