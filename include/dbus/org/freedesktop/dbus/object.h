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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_OBJECT_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_OBJECT_H_

#include "org/freedesktop/dbus/bus.h"
#include "org/freedesktop/dbus/service.h"

#include <functional>
#include <future>
#include <map>
#include <memory>
#include <ostream>
#include <string>

namespace std
{
template<>
struct hash<std::tuple<std::string, std::string>>
{
    size_t operator()(const std::tuple<std::string, std::string>& key) const
    {
        static const std::hash<std::string> h {};
        return h(std::get<0>(key)) ^ h(std::get<1>(key)); // Using XOR as we do not expect first and second to be equal.
    }
};

std::ostream& operator<<(std::ostream& out, const std::tuple<std::string, std::string>& tuple)
{
    out << "(" << std::get<0>(tuple) << "," << std::get<1>(tuple) << ")";
    return out;
}
}

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace types
{
class Any;
class ObjectPath;
template<typename T>
class Variant;
}
class MatchRule;
template<typename T>
class Property;
template<typename T>
struct Result;
template<typename T, typename U>
class Signal;

class Object : public std::enable_shared_from_this<Object>
{
  private:
    typedef std::tuple<std::string, std::string> MethodKey;
    typedef std::tuple<std::string, std::string> PropertyKey;
    typedef std::tuple<std::string, std::string> SignalKey;

  public:
    typedef std::shared_ptr<Object> Ptr;
    typedef std::function<void(DBusMessage*)> MethodHandler;

    template<typename Signal, typename... Args>
    void emit_signal(const Args& ... args);

    template<typename Method, typename ResultType, typename... Args>
    Result<ResultType> invoke_method_synchronously(const Args& ... args);

    template<typename Method, typename ResultType, typename... Args>
    std::future<Result<ResultType>> invoke_method_asynchronously(const Args& ... args);

    template<typename PropertyDescription>
    std::shared_ptr<Property<PropertyDescription>>
    get_property();

    template<typename Interface>
    std::map<std::string, types::Variant<types::Any>>
    get_all_properties();

    template<typename SignalDescription>
    const std::shared_ptr<Signal<SignalDescription, typename SignalDescription::ArgumentType>>
    get_signal();

    std::shared_ptr<Object> 
    add_object_for_path(const types::ObjectPath& path);

    template<typename Method>
    void install_method_handler(const MethodHandler& handler);

    template<typename Method>
    void uninstall_method_handler();

    bool is_stub() const;

  private:
    friend class Service;
    template<typename T, typename U> friend class Signal;
    template<typename T> friend class Property;

    static void unregister_object_path(DBusConnection*, void*);

    static DBusHandlerResult on_new_message(DBusConnection*, DBusMessage* message, void* user_data);

    Object(const std::shared_ptr<Service> parent, const types::ObjectPath& path);

    void add_match(const MatchRule& rule);
    void remove_match(const MatchRule& rule);

    std::shared_ptr<Service> parent;
    types::ObjectPath object_path;
    MessageRouter<SignalKey> signal_router;
    MessageRouter<MethodKey> method_router;
    MessageRouter<PropertyKey> get_property_router;
    MessageRouter<PropertyKey> set_property_router;
};
}
}
}

#include "impl/object.h"

#endif // DBUS_ORG_FREEDESKTOP_DBUS_OBJECT_H_
