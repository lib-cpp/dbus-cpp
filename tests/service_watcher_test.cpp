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
#include <core/dbus/service.h>
#include <core/dbus/service_watcher.h>
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

TEST_F(ServiceWatcher, BasicBehaviour)
{
    core::testing::CrossProcessSync server_is_running;
    core::testing::CrossProcessSync client_is_listening_for_service_registration;

    auto service = [this, &server_is_running, &client_is_listening_for_service_registration]()
    {
        core::testing::SigTermCatcher sc;

        EXPECT_EQ(1,
                          client_is_listening_for_service_registration.wait_for_signal_ready_for(
                              std::chrono::milliseconds{500}));

        auto bus = session_bus();
        bus->install_executor(core::dbus::asio::make_executor(bus));
        auto service = dbus::Service::add_service<test::Service>(bus);

        std::thread t{[bus](){ bus->run(); }};

        sc.wait_for_signal_for(std::chrono::seconds{10});

        bus->stop();

        if (t.joinable())
            t.join();

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    auto client = [this, &server_is_running, &client_is_listening_for_service_registration]()
    {
        auto bus = session_bus();
        auto executor = core::dbus::asio::make_executor(bus);
        bus->install_executor(executor);
        std::thread t{[bus](){ bus->run(); }};

        dbus::DBus daemon(bus);
        dbus::ServiceWatcher::Ptr watcher_one(
                daemon.make_service_watcher(dbus::traits::Service<test::Service>::interface_name(),
                        dbus::DBus::WatchMode::registration));

        std::string owner_changed_old_owner;
        std::string owner_changed_new_owner;
        bool service_registered = false;
        bool service_unregistered = false;

        watcher_one->owner_changed().connect(
            [&owner_changed_new_owner, &owner_changed_old_owner](const std::string& old_owner, const std::string& new_owner)
            {
                owner_changed_new_owner = new_owner;
                owner_changed_old_owner = old_owner;
            });
        watcher_one->service_registered().connect([bus, &service_registered]()
            {
                service_registered = true;
                bus->stop();
            });
        watcher_one->service_unregistered().connect([&service_unregistered]()
            {
                service_unregistered = true;
            });

        client_is_listening_for_service_registration.try_signal_ready_for(std::chrono::milliseconds{500});

        if (t.joinable())
            t.join();

        EXPECT_TRUE(owner_changed_old_owner.empty());
        EXPECT_FALSE(owner_changed_new_owner.empty());
        EXPECT_TRUE(service_registered);
        EXPECT_FALSE(service_unregistered);

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    };

    EXPECT_EQ(core::testing::ForkAndRunResult::empty, core::testing::fork_and_run(service, client));
}
