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
#ifndef CORE_DBUS_IMPL_OBJECT_H_
#define CORE_DBUS_IMPL_OBJECT_H_

#include <core/dbus/bus.h>
#include <core/dbus/lifetime_constrained_cache.h>
#include <core/dbus/match_rule.h>
#include <core/dbus/message_router.h>
#include <core/dbus/message_streaming_operators.h>
#include <core/dbus/result.h>
#include <core/dbus/signal.h>
#include <core/dbus/interfaces/properties.h>
#include <core/dbus/traits/service.h>
#include <core/dbus/types/any.h>
#include <core/dbus/types/object_path.h>
#include <core/dbus/types/variant.h>
#include <core/dbus/types/stl/map.h>
#include <core/dbus/types/stl/string.h>
#include <core/dbus/types/stl/tuple.h>
#include <core/dbus/types/stl/vector.h>

#include <functional>
#include <future>
#include <map>
#include <memory>
#include <string>

namespace core
{
namespace dbus
{
template<typename Signal, typename... Args>
inline void Object::emit_signal(const Args& ... args)
{
    auto msg_factory = parent->get_connection()->message_factory();
    auto msg = msg_factory->make_signal(
        object_path.as_string(),
        traits::Service<typename Signal::Interface>::interface_name(),
        Signal::name());
    if (!msg)
        throw std::runtime_error("No memory available to allocate DBus message");

    auto writer = msg->writer();
    encode_message(writer, args...);
    parent->get_connection()->send(msg);
}

template<typename Method, typename ResultType, typename... Args>
inline Result<ResultType> Object::invoke_method_synchronously(const Args& ... args)
{
    auto msg_factory = parent->get_connection()->message_factory();
    auto msg = msg_factory->make_method_call(
        parent->get_name(),
        object_path.as_string(),
        traits::Service<typename Method::Interface>::interface_name().c_str(),
        Method::name());
    
    if (!msg)
        throw std::runtime_error("No memory available to allocate DBus message");
    
    auto writer = msg->writer();
    encode_message(writer, args...);
    
    auto reply = parent->get_connection()->send_with_reply_and_block_for_at_most(
                msg,
                Method::default_timeout());
    
    return Result<ResultType>::from_message(reply);
}

template<typename Method, typename ResultType, typename... Args>
inline Result<ResultType> Object::transact_method(const Args& ... args)
{
    return invoke_method_asynchronously<Method, ResultType, Args...>(args...).get();
}

template<typename Method, typename ResultType, typename... Args>
inline std::future<Result<ResultType>> Object::invoke_method_asynchronously(const Args& ... args)
{
    auto msg_factory = parent->get_connection()->message_factory();
    auto msg = msg_factory->make_method_call(
        parent->get_name(),
        object_path.as_string(),
        traits::Service<typename Method::Interface>::interface_name().c_str(),
        Method::name());
    
    if (!msg)
        throw std::runtime_error("No memory available to allocate DBus message");
    
    auto writer = msg->writer();
    encode_message(writer, args...);

    auto pending_call =
            parent->get_connection()->send_with_reply_and_timeout(
                msg, Method::default_timeout());
    
    auto promise = std::make_shared<std::promise<Result<ResultType>>>();
    auto future = promise->get_future();

    pending_call->then([promise](const Message::Ptr& reply)
    {
        promise->set_value(Result<ResultType>::from_message(reply));
    });

    return future;
}

template<typename PropertyDescription>
inline std::shared_ptr<Property<PropertyDescription>>
Object::get_property()
{
    // If this is a proxy object we set up listening for property changes the
    // first time someone accesses properties.
    if (parent->is_stub())
    {
        if (!signal_properties_changed)
        {
            signal_properties_changed
                = get_signal<interfaces::Properties::Signals::PropertiesChanged>();

            signal_properties_changed->connect(
                std::bind(
                    &Object::on_properties_changed,
                    shared_from_this(),
                    std::placeholders::_1));
        }
    }

    typedef Property<PropertyDescription> PropertyType;
    auto property =
            PropertyType::make_property(
                shared_from_this());

    if (parent->is_stub())
    {
        auto tuple = std::make_tuple(
                traits::Service<typename PropertyDescription::Interface>::interface_name(),
                PropertyDescription::name());

        property_changed_vtable[tuple] = std::bind(
                &Property<PropertyDescription>::handle_changed,
                property,
                std::placeholders::_1);
    }
    return property;
}

template<typename Interface>
inline std::map<std::string, types::Variant>
Object::get_all_properties()
{
    return
        std::move(
                invoke_method_synchronously<
                    interfaces::Properties::GetAll,
                    std::map<std::string, types::Variant>
                >(traits::Service<Interface>::interface_name()).value());
}

template<typename SignalDescription>
inline const std::shared_ptr<Signal<SignalDescription, typename SignalDescription::ArgumentType>>
Object::get_signal()
{
    typedef Signal<SignalDescription, typename SignalDescription::ArgumentType> SignalType;
    auto signal =
            SignalType::make_signal(shared_from_this(),
                                    traits::Service<typename SignalDescription::Interface>::interface_name(),
                                    SignalDescription::name());
    return signal;
}

inline std::shared_ptr<Object> Object::add_object_for_path(const types::ObjectPath& path)
{
    return std::shared_ptr<Object>(new Object(parent, path));
}

template<typename Method>
inline void Object::install_method_handler(const MethodHandler& handler)
{
    static const dbus::Object::MethodKey key
    {
        dbus::traits::Service<typename Method::Interface>::interface_name(),
        Method::name()
    };
    method_router.install_route(key, handler);
}

template<typename Method>
inline void Object::uninstall_method_handler()
{
    static const dbus::Object::MethodKey key
    {
        dbus::traits::Service<typename Method::Interface>::interface_name(),
        Method::name()
    };
    method_router.uninstall_route(key);
}

inline bool Object::is_stub() const
{
    return parent->is_stub();
}

inline bool Object::on_new_message(const Message::Ptr& msg)
{
    return method_router(msg);
}

inline const types::ObjectPath& Object::path() const
{
    return object_path;
}

inline Object::Object(
    const std::shared_ptr<Service> parent, 
    const types::ObjectPath& path)
        : parent(parent),
          object_path(path),
          signal_router
          {
              [](const Message::Ptr& msg)
              {
                  return SignalKey {msg->interface(), msg->member()};
              }
          },
          method_router
          {
              [](const Message::Ptr& msg)
              {
                  return MethodKey {msg->interface(), msg->member()};
              }
          },
          get_property_router
          {
              [](const Message::Ptr& msg)
              {
                  std::string interface, member;
                  msg->reader() >> interface >> member;
                  return PropertyKey{interface, member};
              }
          },
          set_property_router
          {
              [](const Message::Ptr& msg)
              {
                  std::string interface, member;
                  msg->reader() >> interface >> member;
                  return PropertyKey {interface, member};
              }
          }
{
    parent->get_connection()->access_signal_router().install_route(
        object_path,
        std::bind(&MessageRouter<SignalKey>::operator(),
                  std::ref(signal_router),
                  std::placeholders::_1));

    if (!parent->is_stub())
    {
        install_method_handler<interfaces::Properties::Get>(
            std::bind(
                &MessageRouter<PropertyKey>::operator(), 
                std::ref(get_property_router), 
                std::placeholders::_1));

        install_method_handler<interfaces::Properties::Set>(
            std::bind(
                &MessageRouter<PropertyKey>::operator(), 
                std::ref(set_property_router), 
                std::placeholders::_1));
    }
}

inline void Object::add_match(const MatchRule& rule)
{
    parent->add_match(rule.path(object_path));
}

inline void Object::remove_match(const MatchRule& rule)
{
    parent->remove_match(rule.path(object_path));
}

inline void Object::on_properties_changed(
        const interfaces::Properties::Signals::PropertiesChanged::ArgumentType& arg)
{
    const auto& interface = std::get<0>(arg);
    const auto& changed_values = std::get<1>(arg);

    for (const auto& value : changed_values)
    {
        auto it = property_changed_vtable.find(std::make_tuple(interface, value.first));
        if (it != property_changed_vtable.end())
        {
            it->second(value.second);
        }
    }
}
}
}

#endif // CORE_DBUS_IMPL_OBJECT_H_
