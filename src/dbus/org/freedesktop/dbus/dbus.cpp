/*
 * Copyright © 2012 Canonical Ltd.
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

#include <org/freedesktop/dbus/types/stl/string.h>
#include <org/freedesktop/dbus/types/stl/vector.h>

namespace org
{
namespace freedesktop
{
namespace dbus
{
struct DBus::ListNames
{
    typedef DBus Interface;

    inline static const std::string& name()
    {
        static const std::string s
        {
            "ListNames"
        };
        return s;
    }

    inline static const std::chrono::milliseconds default_timeout()
    {
        return std::chrono::seconds{1};
    }
};

struct DBus::GetConnectionUnixProcessID
{
    typedef DBus Interface;

    inline static const std::string& name()
    {
        static const std::string s
        {
            "GetConnectionUnixProcessID"
        };
        return s;
    }

    inline static const std::chrono::milliseconds default_timeout()
    {
        return std::chrono::seconds{1};
    }
};

struct DBus::GetConnectionUnixUser
{
    typedef DBus Interface;

    static const std::string& name()
    {
        static const std::string s
        {
            "GetConnectionUnixUser"
        };
        return s;
    }

    inline static const std::chrono::milliseconds default_timeout()
    {
        return std::chrono::seconds{1};
    }
};

const std::string& DBus::name()
{
    static const std::string s{DBUS_SERVICE_DBUS};
    return s;
}

const types::ObjectPath& DBus::path()
{
    static const types::ObjectPath path{DBUS_PATH_DBUS};
    return path;
}

const std::string& DBus::interface()
{
    static const std::string s{DBUS_INTERFACE_DBUS};
    return s;
}

DBus::DBus(const Bus::Ptr& bus)
    : bus(bus),
      service(Service::use_service<DBus>(bus)),
      object(service->object_for_path(DBus::path()))
{
}

std::vector<std::string> DBus::list_names() const
{
    return object->invoke_method_synchronously<ListNames, std::vector<std::string>>().value();
}

uint32_t DBus::get_connection_unix_process_id(const std::string& name) const
{
    return object->invoke_method_synchronously<GetConnectionUnixProcessID, uint32_t>(name).value();
}

uint32_t DBus::get_connection_unix_user(const std::string& name) const
{
    return object->invoke_method_synchronously<GetConnectionUnixUser, uint32_t>(name).value();
}
}
}
}
