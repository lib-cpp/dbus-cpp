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
#ifndef CORE_DBUS_STUB_H_
#define CORE_DBUS_STUB_H_

#include <org/freedesktop/dbus/bus.h>
#include <org/freedesktop/dbus/object.h>
#include <org/freedesktop/dbus/service.h>
#include <org/freedesktop/dbus/visibility.h>

namespace core
{
namespace dbus
{
/**
 * @brief Stub is a template class that helps with accessing proxy objects on the bus.
 * @tparam T The type of the interface for which we want to access a proxy for.
 */
template<typename T>
class ORG_FREEDESKTOP_DBUS_DLL_PUBLIC Stub : public T
{
public:
    virtual ~Stub() noexcept(true) = default;

protected:
    /**
     * @brief Stub creates a proxy for the service on the given bus instance.
     * @param bus The bus that the actual service lives upon
     */
    inline explicit Stub(const Bus::Ptr& bus) : bus(bus),
        service(Service::use_service<T>(bus)),
        root(service->root_object())
    {
    }

    /**
     * @brief access_bus provides access to the underlying bus instance.
     * @return A mutable reference to the underlying bus.
     */
    inline const Bus::Ptr& access_bus() const
    {
        return bus;
    }

    /**
     * @brief Provides access to the root object of the service.
     * @return A mutable reference to the root object.
     */
    inline const Object::Ptr& access_root() const
    {
        return root;
    }

    /**
     * @brief Provides access to the underlying service object that this object is a proxy for.
     * @return A mutable reference to the underlying service object.
     */
    inline const Service::Ptr& access_service() const
    {
        return service;
    }
private:
    Bus::Ptr bus;
    Service::Ptr service;
    Object::Ptr root;
};
}
}
#endif // CORE_DBUS_STUB_H_
