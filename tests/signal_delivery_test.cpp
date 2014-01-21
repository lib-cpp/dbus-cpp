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

#include <core/dbus/dbus.h>
#include <core/dbus/fixture.h>
#include <core/dbus/object.h>
#include <core/dbus/property.h>
#include <core/dbus/service.h>
#include <core/dbus/interfaces/properties.h>
#include <core/dbus/types/stl/tuple.h>
#include <core/dbus/types/stl/vector.h>

#include <core/dbus/asio/executor.h>

#include "sig_term_catcher.h"
#include "test_data.h"
#include "test_service.h"

#include <core/testing/cross_process_sync.h>
#include <core/testing/fork_and_run.h>

#include <gtest/gtest.h>

#include <system_error>
#include <thread>

namespace dbus = core::dbus;

namespace
{
struct Service : public core::dbus::testing::Fixture
{
};

auto session_bus_config_file =
        core::dbus::testing::Fixture::default_session_bus_config_file() =
        core::testing::session_bus_configuration_file();

auto system_bus_config_file =
        core::dbus::testing::Fixture::default_system_bus_config_file() =
        core::testing::system_bus_configuration_file();
}

TEST_F(Service, SignalDeliveryMultipleObjectsSameInterface)
{
    core::testing::CrossProcessSync server_is_running;
    core::testing::CrossProcessSync client_has_setup_signals_and_connections;

    const int64_t expected_value = 42;

    auto service = [this, expected_value, &server_is_running, &client_has_setup_signals_and_connections]()
    {
        core::testing::SigTermCatcher sc;

        auto bus = session_bus();
        bus->install_executor(core::dbus::asio::make_executor(bus));
        auto service = dbus::Service::add_service<test::Service>(bus);

        auto foo1 = service->add_object_for_path(dbus::types::ObjectPath("/this/is/unlikely/to/exist/Service/Foo1"));
        auto foo2 = service->add_object_for_path(dbus::types::ObjectPath("/this/is/unlikely/to/exist/Service/Foo2"));

        std::thread t{[bus](){ bus->run(); }};

        server_is_running.try_signal_ready_for(std::chrono::milliseconds{1000});
        EXPECT_EQ(1,
                  client_has_setup_signals_and_connections.wait_for_signal_ready_for(
                      std::chrono::milliseconds{500}));

        foo1->emit_signal<
                test::Service::Interfaces::Foo::Signals::Dummy,
                test::Service::Interfaces::Foo::Signals::Dummy::ArgumentType
                > (1);

        foo2->emit_signal<
                test::Service::Interfaces::Foo::Signals::Dummy,
                test::Service::Interfaces::Foo::Signals::Dummy::ArgumentType
                > (2);


        sc.wait_for_signal_for(std::chrono::seconds{10});

        bus->stop();

        if (t.joinable())
            t.join();

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    auto client = [this, expected_value, &server_is_running, &client_has_setup_signals_and_connections]()
    {
        auto bus = session_bus();
        auto executor = core::dbus::asio::make_executor(bus);
        bus->install_executor(executor);
        std::thread t{[bus](){ bus->run(); }};

        test::Service::Interfaces::Foo::Signals::Dummy::ArgumentType received1 = -1;
        test::Service::Interfaces::Foo::Signals::Dummy::ArgumentType received2 = -1;

        // server ready
        EXPECT_EQ(1,
                  server_is_running.wait_for_signal_ready_for(std::chrono::milliseconds{500}));

        auto stub_service = dbus::Service::use_service(bus, dbus::traits::Service<test::Service>::interface_name());

        auto foo1 = stub_service->object_for_path(dbus::types::ObjectPath("/this/is/unlikely/to/exist/Service/Foo1"));
        auto foo2 = stub_service->object_for_path(dbus::types::ObjectPath("/this/is/unlikely/to/exist/Service/Foo2"));

        auto foo1signal = foo1->get_signal<test::Service::Interfaces::Foo::Signals::Dummy>();
        auto foo2signal = foo2->get_signal<test::Service::Interfaces::Foo::Signals::Dummy>();

        foo1signal->connect([&received1](test::Service::Interfaces::Foo::Signals::Dummy::ArgumentType value)
        {
            received1 = value;
        });

        foo2signal->connect([bus, &received2](test::Service::Interfaces::Foo::Signals::Dummy::ArgumentType value)
        {
            received2 = value;
            bus->stop();
        });

        // signals connected
        client_has_setup_signals_and_connections.try_signal_ready_for(std::chrono::milliseconds{500});

        if (t.joinable())
            t.join();

        EXPECT_EQ(received1, 1);
        EXPECT_EQ(received2, 2);

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    EXPECT_EQ(core::testing::ForkAndRunResult::empty, core::testing::fork_and_run(service, client));
}

TEST_F(Service, SignalDeliverySameObjectSameInterfaceDifferentSignal)
{
    core::testing::CrossProcessSync server_is_running;
    core::testing::CrossProcessSync client_has_setup_signals_and_connections;

    const int64_t expected_value = 42;

    auto service = [this, expected_value, &server_is_running, &client_has_setup_signals_and_connections]()
    {
        core::testing::SigTermCatcher sc;

        auto bus = session_bus();
        bus->install_executor(core::dbus::asio::make_executor(bus));
        auto service = dbus::Service::add_service<test::Service>(bus);

        auto foo1 = service->add_object_for_path(dbus::types::ObjectPath("/this/is/unlikely/to/exist/Service/Foo1"));

        std::thread t{[bus](){ bus->run(); }};

        server_is_running.try_signal_ready_for(std::chrono::milliseconds{1000});
        EXPECT_EQ(1,
                  client_has_setup_signals_and_connections.wait_for_signal_ready_for(
                      std::chrono::milliseconds{500}));

        foo1->emit_signal<
                test::Service::Interfaces::Foo::Signals::Dummy,
                test::Service::Interfaces::Foo::Signals::Dummy::ArgumentType
                > (1);

        foo1->emit_signal<
                test::Service::Interfaces::Foo::Signals::Bar,
                test::Service::Interfaces::Foo::Signals::Bar::ArgumentType
                > (2);


        sc.wait_for_signal_for(std::chrono::seconds{10});

        bus->stop();

        if (t.joinable())
            t.join();

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    auto client = [this, expected_value, &server_is_running, &client_has_setup_signals_and_connections]()
    {
        auto bus = session_bus();
        auto executor = core::dbus::asio::make_executor(bus);
        bus->install_executor(executor);
        std::thread t{[bus](){ bus->run(); }};

        test::Service::Interfaces::Foo::Signals::Dummy::ArgumentType received1 = -1;
        test::Service::Interfaces::Foo::Signals::Bar::ArgumentType received2 = -1;

        // server ready
        EXPECT_EQ(1,
                  server_is_running.wait_for_signal_ready_for(std::chrono::milliseconds{500}));

        auto stub_service = dbus::Service::use_service(bus, dbus::traits::Service<test::Service>::interface_name());

        auto foo1 = stub_service->object_for_path(dbus::types::ObjectPath("/this/is/unlikely/to/exist/Service/Foo1"));

        auto foo1signal = foo1->get_signal<test::Service::Interfaces::Foo::Signals::Dummy>();
        auto foo2signal = foo1->get_signal<test::Service::Interfaces::Foo::Signals::Bar>();

        foo1signal->connect([&received1](test::Service::Interfaces::Foo::Signals::Dummy::ArgumentType value)
        {
            received1 = value;
        });

        foo2signal->connect([bus, &received2](test::Service::Interfaces::Foo::Signals::Bar::ArgumentType value)
        {
            received2 = value;
            bus->stop();
        });

        // signals connected
        client_has_setup_signals_and_connections.try_signal_ready_for(std::chrono::milliseconds{500});

        if (t.joinable())
            t.join();

        EXPECT_EQ(received1, 1);
        EXPECT_EQ(received2, 2);

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    EXPECT_EQ(core::testing::ForkAndRunResult::empty, core::testing::fork_and_run(service, client));
}

TEST_F(Service, SignalDeliverySameObjectSameInterfaceSameSignal)
{
    core::testing::CrossProcessSync server_is_running;
    core::testing::CrossProcessSync client_has_setup_signals_and_connections;

    const int64_t expected_value = 42;

    auto service = [this, expected_value, &server_is_running, &client_has_setup_signals_and_connections]()
    {
        core::testing::SigTermCatcher sc;

        auto bus = session_bus();
        bus->install_executor(core::dbus::asio::make_executor(bus));
        auto service = dbus::Service::add_service<test::Service>(bus);

        auto foo1 = service->add_object_for_path(dbus::types::ObjectPath("/this/is/unlikely/to/exist/Service/Foo1"));

        std::thread t{[bus](){ bus->run(); }};

        server_is_running.try_signal_ready_for(std::chrono::milliseconds{1000});
        EXPECT_EQ(1,
                  client_has_setup_signals_and_connections.wait_for_signal_ready_for(
                      std::chrono::milliseconds{500}));

        foo1->emit_signal<
                test::Service::Interfaces::Foo::Signals::Dummy,
                test::Service::Interfaces::Foo::Signals::Dummy::ArgumentType
                > (1);

        foo1->emit_signal<
                test::Service::Interfaces::Foo::Signals::Dummy,
                test::Service::Interfaces::Foo::Signals::Dummy::ArgumentType
                > (2);


        sc.wait_for_signal_for(std::chrono::seconds{10});

        bus->stop();

        if (t.joinable())
            t.join();

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    auto client = [this, expected_value, &server_is_running, &client_has_setup_signals_and_connections]()
    {
        auto bus = session_bus();
        auto executor = core::dbus::asio::make_executor(bus);
        bus->install_executor(executor);
        std::thread t{[bus](){ bus->run(); }};

        test::Service::Interfaces::Foo::Signals::Dummy::ArgumentType received1 = -1;
        test::Service::Interfaces::Foo::Signals::Dummy::ArgumentType received2 = -1;

        // server ready
        EXPECT_EQ(1,
                  server_is_running.wait_for_signal_ready_for(std::chrono::milliseconds{500}));

        auto stub_service = dbus::Service::use_service(bus, dbus::traits::Service<test::Service>::interface_name());

        auto foo1 = stub_service->object_for_path(dbus::types::ObjectPath("/this/is/unlikely/to/exist/Service/Foo1"));

        auto foo1signal = foo1->get_signal<test::Service::Interfaces::Foo::Signals::Dummy>();
        auto foo2signal = foo1->get_signal<test::Service::Interfaces::Foo::Signals::Dummy>();

        foo1signal->connect([&received1](test::Service::Interfaces::Foo::Signals::Dummy::ArgumentType value)
        {
            received1 = value;
        });

        foo2signal->connect([bus, &received2](test::Service::Interfaces::Foo::Signals::Dummy::ArgumentType value)
        {
            received2 = value;
            bus->stop();
        });

        // signals connected
        client_has_setup_signals_and_connections.try_signal_ready_for(std::chrono::milliseconds{500});

        if (t.joinable())
            t.join();

        EXPECT_EQ(received1, 1);
        EXPECT_EQ(received2, 1);

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    EXPECT_EQ(core::testing::ForkAndRunResult::empty, core::testing::fork_and_run(service, client));
}
