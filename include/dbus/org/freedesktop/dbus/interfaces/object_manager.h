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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_INTERFACES_OBJECT_MANAGER_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_INTERFACES_OBJECT_MANAGER_H_

#include <org/freedesktop/dbus/service.h>
#include <org/freedesktop/dbus/types/any.h>
#include <org/freedesktop/dbus/types/object_path.h>
#include <org/freedesktop/dbus/types/variant.h>

#include <chrono>
#include <string>

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace interfaces
{
class ObjectManager
{
public:
    virtual ~ObjectManager() = default;

    std::map<types::ObjectPath, std::map<std::string,std::map<std::string,types::Variant<types::Any>>>> get_managed_objects()
    {
        return service->root_object()->invoke_method_synchronously<
               GetManagedObjects, std::map<types::ObjectPath, std::map<std::string,std::map<std::string,types::Variant<types::Any>>>>
               >();
    }

protected:
    ObjectManager(const Service::Ptr& service) : service(service)
    {
    }
private:
    struct GetManagedObjects
    {
        typedef ObjectManager Interface;
        inline static std::string name()
        {
            return "GetManagedObjects";
        }
        static const bool call_synchronously = true;
        inline static const std::chrono::milliseconds default_timeout()
        {
            return std::chrono::seconds{1};
        }
    };
    Service::Ptr service;
};
}
template<>
struct ServiceTraits<interfaces::ObjectManager>
{
    inline static const std::string& interface_name()
    {
        static const std::string s
        {
            "org.freedesktop.DBus.ObjectManager"
        };
        return s;
    }
};
}
}
}


#endif // DBUS_ORG_FREEDESKTOP_DBUS_INTERFACES_INTROSPECTABLE_H_
