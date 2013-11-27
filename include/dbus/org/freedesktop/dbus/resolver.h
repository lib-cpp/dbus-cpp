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
#ifndef CORE_DBUS_RESOLVER_H_
#define CORE_DBUS_RESOLVER_H_

#include <org/freedesktop/dbus/bus.h>
#include <org/freedesktop/dbus/stub.h>

namespace core
{
namespace dbus
{
/**
 * @brief Resolves an interface on the bus and creates a proxy object for it.
 * @tparam Interface The interface to be resolved on the bus.
 * @tparam ServiceStub The stub or proxy wrapping access to the interface.
 */
template<typename Interface, typename ServiceStub>
typename ServiceStub::Ptr resolve_service_on_bus(const Bus::Ptr& bus)
{
    static_assert(std::is_base_of<Stub<Interface>, ServiceStub>::value, "Not a stub");
    return typename ServiceStub::Ptr(new ServiceStub(bus));
}
}
}

#endif // CORE_DBUS_RESOLVER_H_
