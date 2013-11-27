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
#ifndef CORE_DBUS_DBUS_H_
#define CORE_DBUS_DBUS_H_

#include <org/freedesktop/dbus/bus.h>
#include <org/freedesktop/dbus/codec.h>
#include <org/freedesktop/dbus/object.h>
#include <org/freedesktop/dbus/service.h>
#include <org/freedesktop/dbus/visibility.h>
#include <org/freedesktop/dbus/types/object_path.h>

#include <sstream>

namespace core
{
namespace dbus
{
class ORG_FREEDESKTOP_DBUS_DLL_PUBLIC DBus
{
public:
    /** @brief Query the well-known name of the DBus daemon. */
    ORG_FREEDESKTOP_DBUS_DLL_PUBLIC static const std::string& name();

    /** @brief Query the object path of the DBus daemon. */
    ORG_FREEDESKTOP_DBUS_DLL_PUBLIC static const types::ObjectPath& path();

    /** @brief Query the interface name of the DBus daemon. */
    ORG_FREEDESKTOP_DBUS_DLL_PUBLIC static const std::string& interface();

    ORG_FREEDESKTOP_DBUS_DLL_PUBLIC DBus(const Bus::Ptr& bus);
    DBus(const DBus&) = delete;

    DBus& operator=(const DBus&) = delete;
    bool operator==(const DBus&) const = delete;

    /**
     * @brief Queries the process ID given a name on the bus.
     * @param [in] name Name of the remote peer.
     * @return The process id of the remote peer.
     */
    ORG_FREEDESKTOP_DBUS_DLL_PUBLIC uint32_t get_connection_unix_process_id(const std::string& name) const;

    /**
     * @brief Queries the user ID given a name on the bus.
     * @param [in] name Name of the remote peer.
     * @return The user id that the remote peer runs under.
     */
    ORG_FREEDESKTOP_DBUS_DLL_PUBLIC uint32_t get_connection_unix_user(const std::string& name) const;

    /**
      * @brief List all known names on the bus.
      * @return A vector of all known participants on the bus.
      */
    ORG_FREEDESKTOP_DBUS_DLL_PUBLIC std::vector<std::string> list_names() const;

private:
    struct ListNames;
    struct GetConnectionUnixProcessID;
    struct GetConnectionUnixUser;

    Bus::Ptr bus;
    Service::Ptr service;
    Object::Ptr object;
};
}
}

#endif // CORE_DBUS_DBUS_H_
