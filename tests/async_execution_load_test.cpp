/*
 * Copyright © 2014 Canonical Ltd.
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

#include <core/dbus/fixture.h>
#include <core/dbus/macros.h>
#include <core/dbus/object.h>
#include <core/dbus/service.h>

#include <core/dbus/asio/executor.h>

#include <core/dbus/types/stl/string.h>
#include <core/dbus/types/stl/vector.h>

#include <core/testing/fork_and_run.h>

#include <gmock/gmock.h>

#include "test_data.h"

namespace
{
// Our fixture for starting up private system- and session-bus instances.
struct AsyncExecutionLoadTest : public core::dbus::testing::Fixture
{
};

auto session_bus_config_file =
        core::dbus::testing::Fixture::default_session_bus_config_file() =
        core::testing::session_bus_configuration_file();

auto system_bus_config_file =
        core::dbus::testing::Fixture::default_system_bus_config_file() =
        core::testing::system_bus_configuration_file();

struct DBus
{
    static const std::string& name()
    {
        static const std::string s{DBUS_SERVICE_DBUS};
        return s;
    }

    DBUS_CPP_METHOD_WITH_TIMEOUT_DEF(ListNames, DBus, 25000)
};

struct CountingEventCollector
{
    CountingEventCollector(std::uint64_t expected)
        : expected{expected},
          counter{0}
    {
    }

    void update()
    {
        if (++counter == expected)
            wait_condition.notify_all();
    }

    ::testing::AssertionResult wait_for(const std::chrono::milliseconds& ms)
    {
        std::unique_lock<std::mutex> ul(guard);

        auto result = wait_condition.wait_for(ul, ms, [this]() { return counter == expected; });

        if (result)
            return ::testing::AssertionSuccess();

        return ::testing::AssertionFailure() << "Current count of "
                                             << counter << " does not match " << expected;
    }

    std::uint64_t expected;
    std::atomic<std::uint64_t> counter;

    std::mutex guard;
    std::condition_variable wait_condition;
};

void invoke_list_names_n_times_and_update_event_collector(
        // The object referring to the bus daemon
        const core::dbus::Object::Ptr& dbus,
        // Number of iterations
        std::size_t n,
        // The event collector instance that should be updated
        const std::shared_ptr<CountingEventCollector>& ec)
{
    for (unsigned int i = 0; i < n; i++)
    {
        dbus->invoke_method_asynchronously_with_callback<DBus::ListNames, std::vector<std::string>>([&ec](const core::dbus::Result<std::vector<std::string>>& vs)
        {
            if (not vs.is_error())
                ec->update();
        });
    }
}
}

TEST_F(AsyncExecutionLoadTest, RepeatedlyInvokingAnAsyncFunctionWorks)
{
    using namespace ::testing;

    auto bus = session_bus();
    bus->install_executor(core::dbus::asio::make_executor(bus));

    std::thread worker{[bus]() { bus->run(); }};

    auto service = core::dbus::Service::use_service(bus, DBus::name());
    auto dbus = service->object_for_path(core::dbus::types::ObjectPath{DBUS_PATH_DBUS});

    auto ec = std::make_shared<CountingEventCollector>(5000);

    std::thread t1{[dbus, ec]() {invoke_list_names_n_times_and_update_event_collector(dbus, 1000, ec);}};
    std::thread t2{[dbus, ec]() {invoke_list_names_n_times_and_update_event_collector(dbus, 1000, ec);}};
    std::thread t3{[dbus, ec]() {invoke_list_names_n_times_and_update_event_collector(dbus, 1000, ec);}};
    std::thread t4{[dbus, ec]() {invoke_list_names_n_times_and_update_event_collector(dbus, 1000, ec);}};
    std::thread t5{[dbus, ec]() {invoke_list_names_n_times_and_update_event_collector(dbus, 1000, ec);}};

    EXPECT_TRUE(ec->wait_for(std::chrono::seconds{10}));

    bus->stop();

    if (t1.joinable()) t1.join();
    if (t2.joinable()) t2.join();
    if (t3.joinable()) t3.join();
    if (t4.joinable()) t4.join();
    if (t5.joinable()) t5.join();

    if (worker.joinable())
        worker.join();

    dbus_shutdown();
}
