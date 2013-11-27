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
#ifndef CORE_DBUS_SKELETON_H_
#define CORE_DBUS_SKELETON_H_

#include <core/dbus/bus.h>
#include <core/dbus/service.h>
#include <core/dbus/visibility.h>

namespace core
{
namespace dbus
{
/**
 * @brief Skeleton is a template class that helps with exposing interface implementations on the bus.
 * @tparam T The type of the interface for which we want to expose an implementation for.
 */
template<typename T>
class ORG_FREEDESKTOP_DBUS_DLL_PUBLIC Skeleton : public T
{
public:
    virtual ~Skeleton() noexcept = default;

protected:
    /**
     * @brief Skeleton announces the service on the given bus instance.
     * @param bus The bus that the actual service lives upon
     */
    inline explicit Skeleton(const Bus::Ptr& bus) : bus(bus),
        service(Service::add_service<T>(bus))
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
};
}
}
#endif // CORE_DBUS_SKELETON_H_
