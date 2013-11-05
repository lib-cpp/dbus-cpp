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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_STUB_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_STUB_H_

#include <org/freedesktop/dbus/bus.h>
#include <org/freedesktop/dbus/object.h>
#include <org/freedesktop/dbus/service.h>
#include <org/freedesktop/dbus/visibility.h>

namespace org
{
namespace freedesktop
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
    inline explicit Stub(const Bus::Ptr& bus) : bus(bus),
        service(Service::use_service<T>(bus)),
        root(service->root_object())
    {
    }

    inline const Bus::Ptr& access_bus() const
    {
        return bus;
    }

    inline const Object::Ptr& access_root() const
    {
        return root;
    }

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
}
#endif // DBUS_ORG_FREEDESKTOP_DBUS_STUB_H_
