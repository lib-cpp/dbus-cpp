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

#include <core/dbus/service.h>

#include <core/dbus/bus.h>
#include <core/dbus/codec.h>
#include <core/dbus/match_rule.h>
#include <core/dbus/message_router.h>
#include <core/dbus/object.h>
#include <core/dbus/result.h>

#include <core/dbus/interfaces/properties.h>
#include <core/dbus/traits/service.h>
#include <core/dbus/types/any.h>
#include <core/dbus/types/object_path.h>
#include <core/dbus/types/variant.h>
#include <core/dbus/types/stl/map.h>
#include <core/dbus/types/stl/string.h>

#include <bitset>
#include <future>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>

namespace core
{
namespace dbus
{
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

Service::Service(const Bus::Ptr& connection, const std::string& name, const Bus::RequestNameFlag& flags)
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

    connection->register_object_for_path(path, object);

    return object;
}
}
}
