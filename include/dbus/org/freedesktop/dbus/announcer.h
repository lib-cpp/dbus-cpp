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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_ANNOUNCER_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_ANNOUNCER_H_

#include <org/freedesktop/dbus/service.h>
#include <org/freedesktop/dbus/skeleton.h>

namespace org
{
namespace freedesktop
{
namespace dbus
{
/**
 * @brief Announces a service implementing the specified interface, dispatching requests to the specified implementation.
 * @tparam Interface The interface that we want to expose a service for.
 * @tparam Implementation The actual backend that calls are dispatched to.
 * @tparam ConstructArgs Parameter bundle passed on to the implementation c'tor.
 * @param [in] connection The DBus connection to announce the service upon.
 * @param [in] args Parameter pack to be passed on the implementation's c'tor.
 */
template<typename Interface, typename Implementation, typename... ConstructArgs>
inline static typename Implementation::Ptr announce_service_on_bus(
    const Bus::Ptr& connection,
    const ConstructArgs&... args)
{
    static_assert(std::is_base_of<Skeleton<Interface>, Implementation>::value,
                  "Implementation is not a Skeleton");
    static_assert(std::is_base_of<Interface, Implementation>::value,
                  "Implementation type does not inherit from Interface type.");
    return typename Implementation::Ptr(new Implementation(connection, args...));
}
}
}
}
#endif // DBUS_ORG_FREEDESKTOP_DBUS_ANNOUNCER_H_
