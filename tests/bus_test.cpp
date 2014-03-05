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

#include <core/dbus/bus.h>
#include <core/dbus/codec.h>
#include <core/dbus/dbus.h>
#include <core/dbus/fixture.h>
#include <core/dbus/match_rule.h>
#include <core/dbus/message_streaming_operators.h>

#include <core/dbus/types/stl/string.h>
#include <core/dbus/types/stl/vector.h>

#include <core/dbus/asio/executor.h>

#include "test_data.h"

#include <core/posix/exec.h>

#include <boost/asio.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <memory>

namespace dbus = core::dbus;

namespace
{
struct Bus : public core::dbus::testing::Fixture
{
};

auto session_bus_config_file =
        core::dbus::testing::Fixture::default_session_bus_config_file() =
        core::testing::session_bus_configuration_file();

auto system_bus_config_file =
        core::dbus::testing::Fixture::default_system_bus_config_file() =
        core::testing::system_bus_configuration_file();

}

TEST_F(Bus, BlockingMethodInvocationSucceedsForValidMessage)
{
    static const char* expected_signature = DBUS_TYPE_ARRAY_AS_STRING DBUS_TYPE_STRING_AS_STRING;
    auto msg = core::dbus::Message::make_method_call(
                dbus::DBus::name(),
                dbus::DBus::path(),
                dbus::DBus::name(),
                "ListNames");

    auto bus = session_bus();
    const std::chrono::milliseconds timeout = std::chrono::seconds(10);
    std::shared_ptr<core::dbus::Message> reply = nullptr;
    EXPECT_NO_THROW(
        reply = bus->send_with_reply_and_block_for_at_most(
                    msg,
                    timeout));

    EXPECT_NE(nullptr, reply.get());
    EXPECT_EQ(core::dbus::Message::Type::method_return, reply->type());
    EXPECT_EQ(expected_signature, reply->signature());
}

TEST_F(Bus, NonBlockingMethodInvocationSucceedsForValidMessage)
{
    auto msg = core::dbus::Message::make_method_call(
                dbus::DBus::name(),
                dbus::DBus::path(),
                dbus::DBus::name(),
                "ListNames");

    auto bus = session_bus();
    bus->install_executor(dbus::asio::make_executor(bus));
    std::thread t{[bus](){bus->run();}};

    const std::chrono::milliseconds timeout = std::chrono::seconds(10);

    auto call = bus->send_with_reply_and_timeout(msg, timeout);
    auto promise = std::make_shared<std::promise<std::vector<std::string>>>();
    auto future = promise->get_future();
    call->then([promise](const core::dbus::Message::Ptr& reply)
    {
        std::vector<std::string> result; reply->reader() >> result;
        promise->set_value(result);
    });

    EXPECT_TRUE(future.get().size() > 0);

    bus->stop();

    if (t.joinable())
        t.join();
}

TEST_F(Bus, HasOwnerForNameReturnsTrueForExistingName)
{
    auto bus = session_bus();
    EXPECT_TRUE(bus->has_owner_for_name(dbus::DBus::name()));
}

TEST_F(Bus, HasOwnerForNameReturnsFalseForNonExistingName)
{
    auto bus = session_bus();
    static const std::string non_existing_name = "com.canonical.does.not.exist";
    EXPECT_FALSE(bus->has_owner_for_name(non_existing_name));
}

TEST_F(Bus, AddingAndRemovingAValidMatchRuleDoesNotThrow)
{
    auto bus = session_bus();

    static const dbus::MatchRule valid_match_rule = dbus::MatchRule().type(dbus::Message::Type::signal);

    struct ScopedMatch
    {
        ScopedMatch(core::dbus::Bus::Ptr bus, const dbus::MatchRule& match_rule) : bus(bus), match_rule(match_rule)
        {
            EXPECT_NO_THROW(bus->add_match(match_rule););
        }

        ~ScopedMatch()
        {
            EXPECT_NO_THROW(bus->remove_match(match_rule););
        }

        core::dbus::Bus::Ptr bus;
        dbus::MatchRule match_rule;
    };

    ScopedMatch match(bus, valid_match_rule);
}

namespace
{
dbus::Message::Ptr a_signal_message(const std::string& path, const std::string& interface, const std::string& name)
{
    return dbus::Message::make_signal(
                path,
                interface,
                name);
}
}

TEST_F(Bus, InstallingARouteForSignalsResultsInTheRouteBeingInvoked)
{
    const std::string path{"/org/gnome/SettingsDaemon/Power"};
    const core::dbus::types::ObjectPath to_route_for(path);
    bool invoked {false};
    auto bus = session_bus();
    bus->install_executor(core::dbus::asio::make_executor(bus));
    bus->access_signal_router().install_route(to_route_for,[&](const dbus::Message::Ptr&)
                                              {
                                                  invoked = true;
                                                  bus->stop();
                                              });
    auto signal = a_signal_message(
                to_route_for.as_string(),
                "org.gnome.SettingsDaemon.Power",
                "LaLeLu");
    bus->access_signal_router()(signal);
}
