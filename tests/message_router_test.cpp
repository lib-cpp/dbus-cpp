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

#include <core/dbus/message.h>
#include <core/dbus/message_router.h>

#include <gtest/gtest.h>

namespace dbus = core::dbus;

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

TEST(MessageRouterForType, ARegisteredRouteIsInvokedForMessageOfMatchingType)
{
    bool invoked {false};

    dbus::MessageRouter<dbus::Message::Type> router([](const dbus::Message::Ptr& msg)
    {
        return msg->type();
    });
    router.install_route(dbus::Message::Type::signal, [&](const dbus::Message::Ptr& msg)
    {
        if (msg->type() == dbus::Message::Type::signal)
            invoked = true;
    });
    auto signal = a_signal_message("/core/DBus", "org.freedesktop.DBus", "LaLeLu");
    router(signal);

    EXPECT_TRUE(invoked);
}
