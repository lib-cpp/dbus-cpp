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
 * Authored by: Pete Woods <pete.woods@canonical.com>
 */

#include <core/dbus/dbus.h>
#include <core/dbus/fixture.h>
#include <core/dbus/object.h>
#include <core/dbus/property.h>
#include <core/dbus/dbus.h>
#include <core/dbus/service_watcher.h>
#include <core/dbus/types/stl/tuple.h>
#include <core/dbus/types/stl/vector.h>

#include <core/dbus/asio/executor.h>

#include "sig_term_catcher.h"
#include "test_data.h"
#include "test_service.h"
#include "test_service_tiny.h"

#include <core/testing/cross_process_sync.h>
#include <core/testing/fork_and_run.h>

#include <gtest/gtest.h>

#include <system_error>
#include <thread>

namespace dbus = core::dbus;

namespace
{
struct ServiceWatcher : public core::dbus::testing::Fixture
{
};

auto session_bus_config_file =
        core::dbus::testing::Fixture::default_session_bus_config_file() =
        core::testing::session_bus_configuration_file();

auto system_bus_config_file =
        core::dbus::testing::Fixture::default_system_bus_config_file() =
        core::testing::system_bus_configuration_file();
}

TEST_F(ServiceWatcher, Registration)
{
    core::testing::CrossProcessSync client_is_listening_for_service_registration;

    auto service = [this, &client_is_listening_for_service_registration]()
    {
        core::testing::SigTermCatcher sc;

        EXPECT_EQ(std::uint32_t(1),
                          client_is_listening_for_service_registration.wait_for_signal_ready_for(
                              std::chrono::milliseconds{500}));

        auto bus = session_bus();
        bus->install_executor(core::dbus::asio::make_executor(bus));
        auto service_tiny = dbus::Service::add_service<test::ServiceTiny>(bus);
        auto service = dbus::Service::add_service<test::Service>(bus);

        std::thread t{[bus](){ bus->run(); }};

        sc.wait_for_signal_for(std::chrono::seconds{10});

        bus->stop();

        if (t.joinable())
            t.join();

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    auto client = [this, &client_is_listening_for_service_registration]()
    {
        auto bus = session_bus();
        auto executor = core::dbus::asio::make_executor(bus);
        bus->install_executor(executor);
        std::thread t{[bus](){ bus->run(); }};

        dbus::DBus daemon(bus);
        dbus::ServiceWatcher::Ptr service_watcher(
                daemon.make_service_watcher(dbus::traits::Service<test::Service>::interface_name(),
                        dbus::DBus::WatchMode::registration));

        std::vector<std::pair<std::string, std::string>> owner_changed;
        unsigned int service_registered = 0;
        unsigned int service_unregistered = 0;

        service_watcher->owner_changed().connect(
            [&owner_changed](const std::string& old_owner, const std::string& new_owner)
            {
                owner_changed.push_back({ new_owner, old_owner });
            });
        service_watcher->service_registered().connect([bus, &service_registered]()
            {
                ++service_registered;
                bus->stop();
            });
        service_watcher->service_unregistered().connect([&service_unregistered]()
            {
                ++service_unregistered;
            });

        client_is_listening_for_service_registration.try_signal_ready_for(std::chrono::milliseconds{500});

        if (t.joinable())
            t.join();

        EXPECT_EQ(std::uint32_t(1), owner_changed.size());
        EXPECT_FALSE(owner_changed.at(0).first.empty());
        EXPECT_TRUE(owner_changed.at(0).second.empty());
        EXPECT_EQ(std::uint32_t(1), service_registered);
        EXPECT_EQ(std::uint32_t(0), service_unregistered);

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    EXPECT_EQ(core::testing::ForkAndRunResult::empty, core::testing::fork_and_run(service, client));
}

TEST_F(ServiceWatcher, Unregistration)
{
    core::testing::CrossProcessSync client_is_listening_for_service_unregistration;

    auto service = [this, &client_is_listening_for_service_unregistration]()
    {
        auto bus = session_bus();
        bus->install_executor(core::dbus::asio::make_executor(bus));

        // Ensure that the client is listening for unregistration before we even
        // start the service.
        EXPECT_EQ(std::uint32_t(1),
                client_is_listening_for_service_unregistration.wait_for_signal_ready_for(
                              std::chrono::milliseconds{500}));

        // We just let this be destroyed immediately
        dbus::Service::add_service<test::ServiceTiny>(bus);

        std::thread t{[bus](){ bus->run(); }};

        bus->stop();

        if (t.joinable())
            t.join();

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    auto client = [this, &client_is_listening_for_service_unregistration]()
    {
        auto bus = session_bus();
        auto executor = core::dbus::asio::make_executor(bus);
        bus->install_executor(executor);
        std::thread t{[bus](){ bus->run(); }};

        dbus::DBus daemon(bus);
        dbus::ServiceWatcher::Ptr service_watcher(
                daemon.make_service_watcher(dbus::traits::Service<test::ServiceTiny>::interface_name(),
                        dbus::DBus::WatchMode::unregistration));

        std::vector<std::pair<std::string, std::string>> owner_changed;
        unsigned int service_registered = 0;
        unsigned int service_unregistered = 0;

        service_watcher->owner_changed().connect(
            [&owner_changed](const std::string& old_owner, const std::string& new_owner)
            {
                owner_changed.push_back({ new_owner, old_owner });
            });
        service_watcher->service_registered().connect([&service_registered]()
            {
                ++service_registered;
            });
        service_watcher->service_unregistered().connect([bus, &service_unregistered]()
            {
                ++service_unregistered;
                bus->stop();
            });

        client_is_listening_for_service_unregistration.try_signal_ready_for(std::chrono::milliseconds{500});

        if (t.joinable())
            t.join();

        EXPECT_EQ(std::uint32_t(1), owner_changed.size());
        EXPECT_TRUE(owner_changed.at(0).first.empty());
        EXPECT_FALSE(owner_changed.at(0).second.empty());
        EXPECT_EQ(std::uint32_t(0), service_registered);
        EXPECT_EQ(std::uint32_t(1), service_unregistered);

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    EXPECT_EQ(core::testing::ForkAndRunResult::empty, core::testing::fork_and_run(service, client));
}
