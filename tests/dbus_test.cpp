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

#include <org/freedesktop/dbus/dbus.h>

#include <org/freedesktop/dbus/asio/executor.h>

#include "test_service.h"
#include "cross_process_sync.h"
#include "fork_and_run.h"

#include <gtest/gtest.h>

namespace dbus = core::dbus;

namespace
{
dbus::Bus::Ptr the_session_bus()
{
    dbus::Bus::Ptr session_bus = std::make_shared<dbus::Bus>(dbus::WellKnownBus::session);
    return session_bus;
}
}

TEST(DBus, QueryingUnixProcessIdReturnsCorrectResult)
{
    const std::string path{"/this/is/just/a/test/service"};

    uint32_t pid = getpid();
    uint32_t uid = getuid();

    test::CrossProcessSync barrier;

    auto child = [path, pid, uid, &barrier]()
    {
        auto bus = the_session_bus();
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
        barrier.signal_ready();
        bus->run();

        EXPECT_EQ(pid, sender_pid);
        EXPECT_EQ(uid, sender_uid);
    };

    auto parent = [path, &barrier]()
    {
        auto bus = the_session_bus();
        
        auto service = dbus::Service::use_service<test::Service>(bus);
        auto object = service->object_for_path(dbus::types::ObjectPath{path});

        barrier.wait_for_signal_ready();

        object->invoke_method_synchronously<test::Service::Method, void>();        
    };

    ASSERT_NO_FATAL_FAILURE(test::fork_and_run(child, parent));
}
