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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_DBUS_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_DBUS_H_

#include <dbus/dbus.h>

#include "org/freedesktop/dbus/bus.h"
#include "org/freedesktop/dbus/codec.h"
#include "org/freedesktop/dbus/service.h"
#include "org/freedesktop/dbus/types/object_path.h"

#include <sstream>

namespace org
{
namespace freedesktop
{
namespace dbus
{

class DBus
{
  public:
    struct ListNames
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
    
    struct GetConnectionUnixProcessID
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

    struct GetConnectionUnixUser
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

    DBus(const Bus::Ptr& bus) 
            : bus(bus),
              service(Service::use_service<DBus>(bus)),
              object(service->object_for_path(types::ObjectPath(DBUS_PATH_DBUS)))
    {        
    }

    uint32_t get_connection_unix_process_id(const std::string& name)
    {
        return object->invoke_method_synchronously<GetConnectionUnixProcessID, uint32_t>(name).value();
    }

    uint32_t get_connection_unix_user(const std::string& name)
    {
        return object->invoke_method_synchronously<GetConnectionUnixUser, uint32_t>(name).value();
    }

  private:
    Bus::Ptr bus;
    Service::Ptr service;
    Object::Ptr object;
};

namespace traits
{
template<> 
struct Service<DBus>
{
    static const std::string& interface_name()
    {
        static const std::string s
        {
            DBUS_SERVICE_DBUS
        };
        return s;
    }
};
}
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_DBUS_H_
