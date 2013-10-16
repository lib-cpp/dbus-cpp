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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_IMPL_OBJECT_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_IMPL_OBJECT_H_

#include "org/freedesktop/dbus/bus.h"
#include "org/freedesktop/dbus/codec.h"
#include "org/freedesktop/dbus/match_rule.h"
#include "org/freedesktop/dbus/message_router.h"
#include "org/freedesktop/dbus/result.h"
#include "org/freedesktop/dbus/signal.h"
#include "org/freedesktop/dbus/interfaces/properties.h"
#include "org/freedesktop/dbus/traits/service.h"
#include "org/freedesktop/dbus/types/any.h"
#include "org/freedesktop/dbus/types/object_path.h"
#include "org/freedesktop/dbus/types/variant.h"
#include "org/freedesktop/dbus/types/stl/map.h"
#include "org/freedesktop/dbus/types/stl/string.h"

#include <functional>
#include <future>
#include <map>
#include <memory>
#include <string>

namespace org
{
namespace freedesktop
{
namespace dbus
{
template<typename Signal, typename... Args>
inline void Object::emit_signal(const Args& ... args)
{
    auto msg = Message::make_signal(
        object_path.as_string(),
        traits::Service<typename Signal::Interface>::interface_name(),
        Signal::name());
    if (!msg)
        throw std::runtime_error("No memory available to allocate DBus message");

    msg->writer().append(args...);
    parent->get_connection()->send(msg->get());
}

template<typename Method, typename ResultType, typename... Args>
inline Result<ResultType> Object::invoke_method_synchronously(const Args& ... args)
{
    auto msg = Message::make_method_call(
        parent->get_name(),
        object_path.as_string(),
        traits::Service<typename Method::Interface>::interface_name().c_str(),
        Method::name());
    
    if (!msg)
        throw std::runtime_error("No memory available to allocate DBus message");
    
    msg->writer().append(args...);
    
    auto reply = Message::from_raw_message(
        parent->get_connection()->send_with_reply_and_block_for_at_most(
            msg->get(), Method::default_timeout()));
    
    Result<ResultType> result;
    result.from_message(reply->get());
    return result;
}

template<typename Method, typename ResultType, typename... Args>
inline std::future<Result<ResultType>> Object::invoke_method_asynchronously(const Args& ... args)
{
    auto msg = Message::make_method_call(
        parent->get_name(),
        object_path.as_string(),
        traits::Service<typename Method::Interface>::interface_name().c_str(),
        Method::name());
    
    if (!msg)
        throw std::runtime_error("No memory available to allocate DBus message");
    
    msg->writer().append(args...);

    auto pending_call =
            parent->get_connection()->send_with_reply_and_timeout(
                msg->get(), Method::default_timeout());
    
    auto cb = 
            [](DBusPendingCall* pending, void* user_data)
            {
                auto promise = static_cast<std::promise<Result<ResultType>>*>(user_data);
                
                auto msg = dbus_pending_call_steal_reply(pending);
                if (msg)
                {
                    Result<ResultType> result;
                    result.from_message(msg);
                    promise->set_value(result);
                }
                else
                {
                    promise->set_exception(std::make_exception_ptr(std::runtime_error("Method invocation timed out")));
                }
            };

    auto promise = new std::promise<Result<ResultType>>();

    dbus_pending_call_set_notify(
        pending_call,
        cb,
        promise,
        [](void* p)
        {
            delete static_cast<std::promise<Result<ResultType>>*>(p);
        });

    return promise->get_future();
}

template<typename PropertyDescription>
inline std::shared_ptr<Property<PropertyDescription>>
Object::get_property()
{
    typedef Property<PropertyDescription> PropertyType;
    auto property =
            PropertyType::make_property(
                shared_from_this());
    
    return property;
}

template<typename Interface>
inline std::map<std::string, types::Variant<types::Any>>
Object::get_all_properties()
{
    return invoke_method_synchronously<
        interfaces::Properties::GetAll,
        std::map<std::string, types::Variant<types::Any>>
        >(traits::Service<Interface>::interface_name()).value();
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

inline void Object::unregister_object_path(DBusConnection*, void*)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

inline DBusHandlerResult Object::on_new_message(DBusConnection*, DBusMessage* message, void* user_data)
{
    auto thiz = static_cast<Object*>(user_data);
    return thiz->method_router(message) ? DBUS_HANDLER_RESULT_HANDLED : DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

inline Object::Object(
    const std::shared_ptr<Service> parent, 
    const types::ObjectPath& path)
        : parent(parent),
          object_path(path),
          signal_router
          {
              [](DBusMessage* msg)
              {
                  return SignalKey {dbus_message_get_interface(msg), dbus_message_get_member(msg)};
              }
          },
          method_router
          {
              [](DBusMessage* msg)
              {
                  return MethodKey {dbus_message_get_interface(msg), dbus_message_get_member(msg)};
              }
          },
          get_property_router
          {
              [](DBusMessage* msg)
              {
                  std::string interface, member;
                  auto m = Message::from_raw_message(msg);
                  m->reader() >> interface >> member;
                  return PropertyKey {interface, member};
              }
          },
          set_property_router
          {
              [](DBusMessage* msg)
              {
                  std::string interface, member;
                  auto m = Message::from_raw_message(msg);
                  m->reader() >> interface >> member;
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
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_IMPL_OBJECT_H_
