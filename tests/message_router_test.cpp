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

#include "org/freedesktop/dbus/message.h"
#include "org/freedesktop/dbus/message_router.h"

#include <gtest/gtest.h>

namespace
{
std::shared_ptr<DBusMessage> a_signal_message(const std::string& path, const std::string& interface, const std::string& name)
{
    DBusMessage* msg = dbus_message_new_signal(path.c_str(), interface.c_str(), name.c_str());
    return std::shared_ptr<DBusMessage> {msg, [](DBusMessage* msg)
    {
        dbus_message_unref(msg);
    }
                                        };
}
}

TEST(MessageRouterForType, ARegisteredRouteIsInvokedForMessageOfMatchingType)
{
    bool invoked {false};

    org::freedesktop::dbus::MessageRouter<org::freedesktop::dbus::Message::Type> router([](DBusMessage* msg)
    {
        return static_cast<org::freedesktop::dbus::Message::Type>(dbus_message_get_type(msg));
    });
    router.install_route(org::freedesktop::dbus::Message::Type::signal, [&](DBusMessage* msg)
    {
        if (dbus_message_get_type(msg) == dbus_message_get_type(msg))
            invoked = true;
    });
    auto msg = a_signal_message("/org/freedesktop/DBus", "org.freedesktop.DBus", "LaLeLu");
    router(msg.get());

    EXPECT_TRUE(invoked);
}
