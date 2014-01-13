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

#include <core/dbus/visibility.h>

#include <memory>
#include <string>
#include <vector>

namespace core
{
namespace dbus
{
class Bus;
class Object;
class Service;
namespace types
{
class ObjectPath;
}
/**
 * @brief The DBus class provides access to dbus daemon on the bus.
 */
class ORG_FREEDESKTOP_DBUS_DLL_PUBLIC DBus
{
public:
    /** @brief Query the well-known name of the DBus daemon. */
    static const std::string& name();

    /** @brief Query the object path of the DBus daemon. */
    static const types::ObjectPath& path();

    /** @brief Query the interface name of the DBus daemon. */
    static const std::string& interface();

    DBus(const std::shared_ptr<Bus>& bus);
    DBus(const DBus&) = delete;

    DBus& operator=(const DBus&) = delete;
    bool operator==(const DBus&) const = delete;

    /**
     * @brief Queries the process ID given a name on the bus.
     * @param [in] name Name of the remote peer.
     * @return The process id of the remote peer.
     */
    uint32_t get_connection_unix_process_id(const std::string& name) const;

    /**
     * @brief Queries the user ID given a name on the bus.
     * @param [in] name Name of the remote peer.
     * @return The user id that the remote peer runs under.
     */
    uint32_t get_connection_unix_user(const std::string& name) const;

    /**
      * @brief Say hello to the message bus daemon.
      * @return The unique name assigned to this connection.
      */
    std::string hello() const;

    /**
      * @brief List all known names on the bus.
      * @return A vector of all known participants on the bus.
      */
    std::vector<std::string> list_names() const;

private:
    struct ListNames;
    struct Hello;
    struct GetConnectionUnixProcessID;
    struct GetConnectionUnixUser;

    std::shared_ptr<Bus> bus;
    std::shared_ptr<Service> service;
    std::shared_ptr<Object> object;
};
}
}

#endif // CORE_DBUS_DBUS_H_
