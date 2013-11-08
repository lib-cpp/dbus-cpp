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

#include "org/freedesktop/dbus/bus.h"
#include "org/freedesktop/dbus/match_rule.h"

#include "org/freedesktop/dbus/asio/executor.h"

#include <boost/asio.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <memory>

namespace dbus = org::freedesktop::dbus;

namespace
{
org::freedesktop::dbus::Bus::Ptr the_session_bus()
{
    org::freedesktop::dbus::Bus::Ptr session_bus = std::make_shared<org::freedesktop::dbus::Bus>(org::freedesktop::dbus::WellKnownBus::session);
    return session_bus;
}
}

TEST(Bus, ConstructionForSessionBusDoesNotThrow)
{
    std::shared_ptr<org::freedesktop::dbus::Bus> bus;
    EXPECT_NO_THROW(bus.reset(new org::freedesktop::dbus::Bus(org::freedesktop::dbus::WellKnownBus::session)););

    EXPECT_TRUE(bus->raw() != nullptr);
}

TEST(Bus, BlockingMethodInvocationSucceedsForValidMessage)
{
    static const char* expected_signature = DBUS_TYPE_ARRAY_AS_STRING DBUS_TYPE_STRING_AS_STRING;
    auto msg = org::freedesktop::dbus::Message::make_method_call(
                DBUS_SERVICE_DBUS,
                DBUS_PATH_DBUS,
                DBUS_SERVICE_DBUS,
                "ListNames");

    auto bus = the_session_bus();
    const std::chrono::milliseconds timeout = std::chrono::seconds(10);
    std::shared_ptr<org::freedesktop::dbus::Message> reply = nullptr;
    EXPECT_NO_THROW(
        reply = bus->send_with_reply_and_block_for_at_most(
                    msg,
                    timeout));

    EXPECT_NE(nullptr, reply.get());
    EXPECT_EQ(org::freedesktop::dbus::Message::Type::method_return, reply->type());
    EXPECT_EQ(expected_signature, reply->signature());
}

TEST(Bus, NonBlockingMethodInvocationSucceedsForValidMessage)
{
    static const char* expected_signature = DBUS_TYPE_ARRAY_AS_STRING DBUS_TYPE_STRING_AS_STRING;
    auto msg = org::freedesktop::dbus::Message::make_method_call(
                DBUS_SERVICE_DBUS,
                DBUS_PATH_DBUS,
                DBUS_SERVICE_DBUS,
                "ListNames");

    auto bus = the_session_bus();
    const std::chrono::milliseconds timeout = std::chrono::seconds(10);
    std::shared_ptr<DBusPendingCall> call;

    EXPECT_NO_THROW(
        call = std::shared_ptr<DBusPendingCall>(
                   bus->send_with_reply_and_timeout(msg, timeout),
                   [](DBusPendingCall* call)
    {
        dbus_pending_call_unref(call);
    }));

    dbus_pending_call_block(call.get());
    std::shared_ptr<DBusMessage> reply(
        dbus_pending_call_steal_reply(call.get()),
        [](DBusMessage* msg)
    {
        dbus_message_unref(msg);
    });

    EXPECT_NE(nullptr, reply);
    EXPECT_EQ(DBUS_MESSAGE_TYPE_METHOD_RETURN, dbus_message_get_type(reply.get()));
    EXPECT_STREQ(expected_signature, dbus_message_get_signature(reply.get()));
}

TEST(Bus, HasOwnerForNameReturnsTrueForExistingName)
{
    org::freedesktop::dbus::Bus bus(org::freedesktop::dbus::WellKnownBus::session);

    EXPECT_TRUE(bus.has_owner_for_name(DBUS_SERVICE_DBUS));
}

TEST(Bus, HasOwnerForNameReturnsFalseForNonExistingName)
{
    auto bus = the_session_bus();
    static const std::string non_existing_name = "com.canonical.does.not.exist";
    EXPECT_FALSE(bus->has_owner_for_name(non_existing_name));
}

TEST(Bus, AddingAndRemovingAValidMatchRuleDoesNotThrow)
{
    auto bus = the_session_bus();

    static const dbus::MatchRule valid_match_rule = dbus::MatchRule().type(dbus::Message::Type::signal);

    struct ScopedMatch
    {
        ScopedMatch(org::freedesktop::dbus::Bus::Ptr bus, const dbus::MatchRule& match_rule) : bus(bus), match_rule(match_rule)
        {
            EXPECT_NO_THROW(bus->add_match(match_rule););
        }

        ~ScopedMatch()
        {
            EXPECT_NO_THROW(bus->remove_match(match_rule););
        }

        org::freedesktop::dbus::Bus::Ptr bus;
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
TEST(Bus, InstallingARouteForSignalsResultsInTheRouteBeingInvoked)
{
    const std::string path{"/org/gnome/SettingsDaemon/Power"};
    const org::freedesktop::dbus::types::ObjectPath to_route_for(path);
    bool invoked {false};
    auto bus = the_session_bus();
    bus->install_executor(org::freedesktop::dbus::asio::make_executor(bus));
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

