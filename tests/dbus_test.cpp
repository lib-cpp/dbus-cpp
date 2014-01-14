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
#include <core/dbus/service.h>
#include <core/dbus/asio/executor.h>

#include "sig_term_catcher.h"
#include "test_data.h"
#include "test_service.h"

#include <core/testing/cross_process_sync.h>
#include <core/testing/fork_and_run.h>

#include <gtest/gtest.h>

namespace dbus = core::dbus;

namespace
{
struct DBus : public core::dbus::testing::Fixture
{
};

auto session_bus_config_file =
        core::dbus::testing::Fixture::default_session_bus_config_file() =
        core::testing::session_bus_configuration_file();

auto system_bus_config_file =
        core::dbus::testing::Fixture::default_system_bus_config_file() =
        core::testing::system_bus_configuration_file();
}

TEST_F(DBus, QueryingUnixProcessIdReturnsCorrectResult)
{
    const std::string path{"/this/is/just/a/test/service"};

    core::testing::CrossProcessSync barrier;

    auto client = core::posix::fork([this, path, &barrier]()
    {
        barrier.wait_for_signal_ready_for(std::chrono::milliseconds{500});

        auto bus = session_bus();

        auto service = dbus::Service::use_service<test::Service>(bus);
        auto object = service->object_for_path(dbus::types::ObjectPath{path});

        object->invoke_method_synchronously<test::Service::Method, void>();

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    }, core::posix::StandardStream::empty);

    uint32_t pid = client.pid();
    uint32_t uid = getuid();

    auto service = core::posix::fork([this, path, pid, uid, &barrier]()
    {
        core::testing::SigTermCatcher sc;

        auto bus = session_bus();
        bus->install_executor(core::dbus::asio::make_executor(bus));
        dbus::DBus daemon{bus};

        auto service = dbus::Service::add_service<test::Service>(bus);
        auto object = service->add_object_for_path(dbus::types::ObjectPath{path});

        uint32_t sender_pid = 0, sender_uid = 0;

        auto handler = [&daemon, &sender_pid, &sender_uid, bus](const dbus::Message::Ptr& msg)
        {
            auto sender = msg->sender();
            sender_pid = daemon.get_connection_unix_process_id(sender);
            sender_uid = daemon.get_connection_unix_user(sender);

            auto reply = dbus::Message::make_method_return(msg);
            bus->send(reply);
            bus->stop();
        };

        object->install_method_handler<test::Service::Method>(handler);
        barrier.try_signal_ready_for(std::chrono::milliseconds{500});

        std::thread t{[bus](){ bus->run(); }};

        sc.wait_for_signal_for(std::chrono::seconds{2});

        if (t.joinable())
            t.join();

        EXPECT_EQ(pid, sender_pid);
        EXPECT_EQ(uid, sender_uid);

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    }, core::posix::StandardStream::empty);

    auto client_result = client.wait_for(core::posix::wait::Flags::untraced);

    EXPECT_EQ(core::posix::wait::Result::Status::exited,
              client_result.status);
    EXPECT_EQ(core::posix::exit::Status::success,
              client_result.detail.if_exited.status);

    service.send_signal_or_throw(core::posix::Signal::sig_term);
    auto service_result = service.wait_for(core::posix::wait::Flags::untraced);

    EXPECT_EQ(core::posix::wait::Result::Status::exited,
              service_result.status);
    EXPECT_EQ(core::posix::exit::Status::success,
              service_result.detail.if_exited.status);
}
