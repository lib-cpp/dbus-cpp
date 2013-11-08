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

#include "org/freedesktop/dbus/service.h"

#include "org/freedesktop/dbus/bus.h"
#include "org/freedesktop/dbus/codec.h"
#include "org/freedesktop/dbus/match_rule.h"
#include "org/freedesktop/dbus/message_router.h"
#include "org/freedesktop/dbus/object.h"
#include "org/freedesktop/dbus/result.h"

#include "org/freedesktop/dbus/interfaces/properties.h"
#include "org/freedesktop/dbus/traits/service.h"
#include "org/freedesktop/dbus/types/any.h"
#include "org/freedesktop/dbus/types/object_path.h"
#include "org/freedesktop/dbus/types/variant.h"
#include "org/freedesktop/dbus/types/stl/map.h"
#include "org/freedesktop/dbus/types/stl/string.h"

#include <boost/signals2.hpp>

#include <dbus/dbus.h>

#include <bitset>
#include <future>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>

namespace org
{
namespace freedesktop
{
namespace dbus
{
Service::RequestNameFlag operator|(Service::RequestNameFlag lhs, Service::RequestNameFlag rhs)
{
    return static_cast<Service::RequestNameFlag>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs));
}

Service::RequestNameFlag Service::default_request_name_flags()
{
    return RequestNameFlag::do_not_queue;
}

Service::Ptr Service::use_service(const Bus::Ptr& connection, const std::string& name)
{
    return Ptr(new Service(connection, name));
}

Service::Ptr Service::use_service_or_throw_if_not_available(const Bus::Ptr& connection, const std::string& name)
{
    if (!connection->has_owner_for_name(name))
        throw std::runtime_error(name + " is not owned on the bus");
    return Ptr(new Service(connection, name));
}

const std::string& Service::get_name() const
{
    return name;
}

bool Service::is_stub() const
{
    return stub;
}

const Bus::Ptr& Service::get_connection() const
{
    return connection;
}

void Service::add_match(const MatchRule& rule)
{
    connection->add_match(rule);
}

void Service::remove_match(const MatchRule& rule)
{
    connection->remove_match(rule);
}

Service::Service(const Bus::Ptr& connection, const std::string& name)
    : connection(connection),
      name(name),
      stub(true)
{

}

Service::Service(const Bus::Ptr& connection, const std::string& name, const Service::RequestNameFlag& flags)
    : connection(connection),
      name(name),
      stub(false)
{
    Error error;
    dbus_bus_request_name(connection->raw(), name.c_str(), static_cast<unsigned int>(flags), std::addressof(error.raw()));

    if (error)
        throw std::runtime_error(error.name() + ": " + error.message());
}

const std::shared_ptr<Object>& Service::root_object()
{
    if (!root)
        root = std::shared_ptr<Object>(new Object(shared_from_this(), types::ObjectPath::root()));
    return root;
}

std::shared_ptr<Object> Service::object_for_path(const types::ObjectPath& path)
{
    return std::shared_ptr<Object>(new Object(shared_from_this(), path));
}

std::shared_ptr<Object> Service::add_object_for_path(const types::ObjectPath& path)
{
    auto object = std::shared_ptr<Object>(new Object(shared_from_this(), path));
    auto vtable = new DBusObjectPathVTable
    {
        Object::unregister_object_path,
        Object::on_new_message,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    };

    Error e;
    auto result = dbus_connection_try_register_object_path(
                      connection->raw(),
                      path.as_string().c_str(),
                      vtable,
                      object.get(),
                      std::addressof(e.raw()));

    if (!result)
    {
        delete vtable;
        throw std::runtime_error(e.name()+ ": " + e.message());
    }

    return object;
}
}
}
}
