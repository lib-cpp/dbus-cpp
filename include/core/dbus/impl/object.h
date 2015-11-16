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
#include <iostream>
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

template<typename Method, typename ResultType, typename... Args>
inline void Object::invoke_method_asynchronously_with_callback(
        std::function<void(const Result<ResultType>&)> cb,
        const Args& ... args)
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

    pending_call->then([cb](const Message::Ptr& reply)
    {
        cb(Result<ResultType>::from_message(reply));
    });
}

template<typename PropertyDescription>
inline std::shared_ptr<Property<PropertyDescription>>
Object::get_property()
{
    typedef Property<PropertyDescription> PropertyType;

    // Creating a stub property for a remote object/property instance
    // requires the following steps:
    //   [1.] Look up if we already have a property instance available in the cache, 
    //        leveraging the tuple (path, interface, name) as key.
    //     [1.1] If yes: return the property.
    //     [1.2] If no: Create a new proeprty instance and:
    //       [1.2.1] Make it known to the cache.
    //       [1.2.2] Wire it up for property_changed signal receiving.
    //       [1.2.3] Communicate a new match rule to the dbus daemon to enable reception.
    if (parent->is_stub())
    {
        auto itf = traits::Service<typename PropertyDescription::Interface>::interface_name();
        auto name = PropertyDescription::name(); 
        auto ekey = std::make_tuple(path(), itf, name);

        auto property = Object::property_cache<PropertyDescription>().retrieve_value_for_key(ekey);
        if (property)
        {
            return property;
        }

        auto mr = MatchRule()
            .type(Message::Type::signal)
            .interface(traits::Service<interfaces::Properties>::interface_name())
            .member(interfaces::Properties::Signals::PropertiesChanged::name());

        property = PropertyType::make_property(shared_from_this());

        Object::property_cache<PropertyDescription>().insert_value_for_key(ekey, property);

        // We only ever do this once per object.
        std::call_once(add_match_once, [&]()
        {
            // [1.2.4] Inform the dbus daemon that we would like to receive the respective signals.
            add_match(mr);
        });

        // [1.2.2] Enable dispatching of changes.
        std::weak_ptr<PropertyType> wp{property};
        property_changed_vtable[std::make_tuple(itf, name)] = [wp](const types::Variant& arg)
        {
            if (auto sp = wp.lock())
                sp->handle_changed(arg);
        };

        return property;
    }

    return PropertyType::make_property(shared_from_this());
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
    } else
    {
        // We centrally route org.freedesktop.DBus.Properties.PropertiesChanged
        // through the object, which in turn routes via a custom Property cache.
        signal_router.install_route(
            SignalKey{
                traits::Service<interfaces::Properties>::interface_name(), 
                interfaces::Properties::Signals::PropertiesChanged::name()
            },
            // Passing 'this' is fine as the lifetime of the signal_router is upper limited
            // by the lifetime of 'this'.
            [this](const Message::Ptr& msg)
            {
                interfaces::Properties::Signals::PropertiesChanged::ArgumentType arg;
                msg->reader() >> arg;
                on_properties_changed(arg);
            });
    }
}

inline Object::~Object()
{
    parent->get_connection()->access_signal_router().uninstall_route(object_path);
    parent->get_connection()->unregister_object_path(object_path);

    for (const auto& pair : property_changed_vtable)
    {
        MatchRule mr;
        mr = mr
            .type(Message::Type::signal)
            .interface(traits::Service<interfaces::Properties>::interface_name())
            .member(interfaces::Properties::Signals::PropertiesChanged::name())
            .args({std::make_pair(0, std::get<0>(pair.first))});
        
        try
        {
            remove_match(mr);
        } catch(...)
        {
            // We consciously drop all possible exceptions here. There is hardly 
            // anything we can do about the error anyway.
        }
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

template<typename PropertyDescription>
inline core::dbus::ThreadSafeLifetimeConstrainedCache<
    core::dbus::Object::CacheKey,
    core::dbus::Property<PropertyDescription>>&
core::dbus::Object::property_cache()
{
    static core::dbus::ThreadSafeLifetimeConstrainedCache<
        core::dbus::Object::CacheKey,
        core::dbus::Property<PropertyDescription>
    > cache;
    return cache;
}
}
}

#endif // CORE_DBUS_IMPL_OBJECT_H_
