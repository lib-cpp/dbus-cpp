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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_SERVICE_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_SERVICE_H_

#include "org/freedesktop/dbus/bus.h"
#include "org/freedesktop/dbus/codec.h"
#include "org/freedesktop/dbus/match_rule.h"
#include "org/freedesktop/dbus/message_router.h"
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

namespace std
{
template<>
struct hash<std::tuple<std::string, std::string>>
{
    inline size_t operator()(const std::tuple<std::string, std::string>& key) const
    {
        static const std::hash<std::string> h {};
        return h(std::get<0>(key)) ^ h(std::get<1>(key)); // Using XOR as we do not expect first and second to be equal.
    }

};

inline std::ostream& operator<<(std::ostream& out, const std::tuple<std::string, std::string>& tuple)
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

class Object;

/**
* \brief Represents a service available on the bus.
* \example geoclue/main.cpp
* Provides an example of accessing the Geoclue session service on the bus.
* \example upower/main.cpp
* Provides an example of access the UPower system service on the bus.
*/
class Service : public std::enable_shared_from_this<Service>
{
public:
    typedef std::shared_ptr<Service> Ptr;

    enum RequestNameFlag
    {
        allow_replacement,
        replace_existing,
        do_not_queue
    };

    typedef std::bitset<3> RequestNameFlags;

    static inline const RequestNameFlags& default_request_name_flags()
    {
        static const RequestNameFlags flags
        {
            "010"
        };
        return flags;
    }

    template<typename Interface>
    inline static Ptr add_service(
        const Bus::Ptr& connection,
        const RequestNameFlags& flags = default_request_name_flags())
    {
        static Ptr instance(new Service(connection, traits::Service<Interface>::interface_name(), flags));
        return instance;
    }

    template<typename Interface>
    static inline Ptr use_service(const Bus::Ptr& connection)
    {
        return Ptr(new Service(connection, traits::Service<Interface>::interface_name()));
    }

    static inline Ptr use_service(const Bus::Ptr& connection, const std::string& name)
    {
        return Ptr(new Service(connection, name));
    }

    static inline Ptr use_service_or_throw_if_not_available(const Bus::Ptr& connection, const std::string& name)
    {
        if (!connection->has_owner_for_name(name))
            throw std::runtime_error(name + " is not owned on the bus");
        return Ptr(new Service(connection, name));
    }

    inline const std::shared_ptr<Object>& root_object();
    inline std::shared_ptr<Object> object_for_path(const types::ObjectPath& path);
    inline std::shared_ptr<Object> add_object_for_path(const types::ObjectPath& path);

    inline const std::string& get_name() const
    {
        return name;
    }

protected:
    friend struct Bus;
    friend class Object;
    template<typename T> friend class Property;

    inline Service(const Bus::Ptr& connection, const std::string& name);
    inline Service(const Bus::Ptr& connection, const std::string& name, const RequestNameFlags& flags);

    inline bool is_stub() const
    {
        return stub;
    }

    inline const Bus::Ptr& get_connection() const
    {
        return connection;
    }

    inline void add_match(const MatchRule& rule)
    {
        connection->add_match(rule.as_string());
    }

    inline void remove_match(const MatchRule& rule)
    {
        connection->remove_match(rule.as_string());
    }
private:
    Bus::Ptr connection;
    std::string name;
    std::shared_ptr<Object> root;
    bool stub;
};

template<typename T>
class Property;

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
    inline void emit_signal(const Args& ... args)
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
    inline Result<ResultType> invoke_method_synchronously(const Args& ... args)
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
    inline std::future<Result<ResultType>> invoke_method_asynchronously(const Args& ... args)
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

        auto cb = [](DBusPendingCall* pending, void* user_data)
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
    get_property()
    {
        typedef Property<PropertyDescription> PropertyType;
        auto property =
            PropertyType::make_property(
                shared_from_this());

        return property;
    }

    template<typename Interface>
    inline std::map<std::string, types::Variant<types::Any>>
    get_all_properties()
    {
        return invoke_method_synchronously<
               interfaces::Properties::GetAll,
               std::map<std::string, types::Variant<types::Any>>
               >(traits::Service<Interface>::interface_name()).value();
    }

    template<typename SignalDescription>
    inline const std::shared_ptr<Signal<SignalDescription, typename SignalDescription::ArgumentType>>
    get_signal()
    {
        typedef Signal<SignalDescription, typename SignalDescription::ArgumentType> SignalType;
        auto signal =
            SignalType::make_signal(shared_from_this(),
                                    traits::Service<typename SignalDescription::Interface>::interface_name(),
                                    SignalDescription::name());
        return signal;
    }

    inline std::shared_ptr<Object> add_object_for_path(const types::ObjectPath& path)
    {
        return std::shared_ptr<Object>(new Object(parent, path));
    }

    template<typename Method>
    inline void install_method_handler(const MethodHandler& handler)
    {
        static const dbus::Object::MethodKey key
        {
            dbus::traits::Service<typename Method::Interface>::interface_name(),
                 Method::name()
        };
        method_router.install_route(key, handler);
    }

    template<typename Method>
    inline void uninstall_method_handler()
    {
        static const dbus::Object::MethodKey key
        {
            dbus::traits::Service<typename Method::Interface>::interface_name(),
                 Method::name()
        };
        method_router.uninstall_route(key);
    }

    inline bool is_stub() const
    {
        return parent->is_stub();
    }

private:
    friend class Service;
    template<typename T, typename U> friend class Signal;
    template<typename T> friend class Property;

    inline static void unregister_object_path(DBusConnection*, void*)
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    inline static DBusHandlerResult on_new_message(DBusConnection*, DBusMessage* message, void* user_data)
    {
        auto thiz = static_cast<Object*>(user_data);
        return thiz->method_router(message) ? DBUS_HANDLER_RESULT_HANDLED : DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    inline Object(const std::shared_ptr<Service> parent, const types::ObjectPath& path)
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

        parent->get_connection()->access_signal_router().install_route(object_path,
                                                                       std::bind(&MessageRouter<SignalKey>::operator(),
                                                                                 std::ref(signal_router),
                                                                                 std::placeholders::_1));

        if (!parent->is_stub())
        {
            install_method_handler<interfaces::Properties::Get>(std::bind(&MessageRouter<PropertyKey>::operator(), std::ref(get_property_router), std::placeholders::_1));
            install_method_handler<interfaces::Properties::Set>(std::bind(&MessageRouter<PropertyKey>::operator(), std::ref(set_property_router), std::placeholders::_1));
        }
    }

    inline void add_match(const MatchRule& rule)
    {
        parent->add_match(rule.path(object_path));
    }

    inline void remove_match(const MatchRule& rule)
    {
        parent->remove_match(rule.path(object_path));
    }

    std::shared_ptr<Service> parent;
    types::ObjectPath object_path;
    MessageRouter<SignalKey> signal_router;
    MessageRouter<MethodKey> method_router;
    MessageRouter<PropertyKey> get_property_router;
    MessageRouter<PropertyKey> set_property_router;
};

template<typename PropertyType>
class Property
{
public:
    inline const typename PropertyType::ValueType& value()
    {
        if (parent->is_stub())
            property_value = parent->invoke_method_synchronously<
                             interfaces::Properties::Get,
                             types::Variant<typename PropertyType::ValueType>
                             >(interface, name).value();
        return property_value.get();
    }

    inline void value(const typename PropertyType::ValueType& new_value)
    {
        property_value.set(new_value);
        if (parent->is_stub())
        {
            if (!writable)
                std::runtime_error("Property is not writable");
            parent->invoke_method_synchronously<
            interfaces::Properties::Set,
                       void
                       >(interface, name, property_value);
        }
    }

    inline bool is_writable() const
    {
        return writable;
    }

protected:
    friend class Object;

    inline static std::shared_ptr<Property<PropertyType>> make_property(
                const std::shared_ptr<Object>& parent)
    {
        return std::shared_ptr<Property<PropertyType>>(
                   new Property<PropertyType>(
                       parent,
                       traits::Service<typename PropertyType::Interface>::interface_name(),
                       PropertyType::name(),
                       PropertyType::writable));
    }

private:
    inline Property(const std::shared_ptr<Object>& parent,
             const std::string& interface,
             const std::string& name,
             bool writable)
            : parent(parent),
              interface(interface),
              name(name),
              writable(writable)
    {
        if (!parent->is_stub())
        {
            parent->get_property_router.install_route(
                Object::PropertyKey
            {
                traits::Service<typename PropertyType::Interface>::interface_name(),
                PropertyType::name()
            },
            std::bind(&Property::handle_get, this, std::placeholders::_1));
            parent->set_property_router.install_route(
                Object::PropertyKey
            {
                traits::Service<typename PropertyType::Interface>::interface_name(),
                PropertyType::name()
            },
            std::bind(&Property::handle_set, this, std::placeholders::_1));
        }
    }

    inline void handle_get(DBusMessage* msg)
    {
        auto reply = Message::make_method_return(msg);
        reply->writer() << property_value;

        parent->parent->get_connection()->send(reply->get());
    }

    inline void handle_set(DBusMessage* msg)
    {
        if (!writable)
        {
            auto error = Message::make_error(
                             msg,
                             traits::Service<interfaces::Properties>::interface_name() + ".NotWritableError",
                             name + "is not writable");

            parent->parent->get_connection()->send(error->get());
            return;
        }

        std::string s;
        auto m = Message::from_raw_message(msg);
        try
        {
            m->reader() >> s >> s >> property_value;
        }
        catch (...)
        {
            auto error = Message::make_error(
                             msg,
                             traits::Service<interfaces::Properties>::interface_name() + ".NotWritableError",
                             name + "is not writable");

            parent->parent->get_connection()->send(error->get());
            return;
        }

        auto reply = Message::make_method_return(msg);
        parent->parent->get_connection()->send(reply->get());
    }

    std::shared_ptr<Object> parent;
    std::string interface;
    std::string name;
    bool writable;

    types::Variant<typename PropertyType::ValueType> property_value;
};

namespace signals
{
typedef boost::signals2::connection Connection;
typedef boost::signals2::scoped_connection ScopedConnection;
}

template<typename T>
struct is_not_void
{
    static const bool value = true;
};

template<>
struct is_not_void<void>
{
    static const bool value = false;
};

template<typename SignalDescription, typename Argument = void>
class Signal
{
public:
    typedef std::shared_ptr<Signal<SignalDescription, void>> Ptr;
    typedef std::function<void()> Handler;

    inline ~Signal() noexcept
    {
        parent->signal_router.uninstall_route(Object::SignalKey{interface, name});
        parent->remove_match(rule);
    }

    inline void emit(void)
    {
        parent->emit_signal<SignalDescription>();
    }

    inline signals::Connection connect(const Handler& h)
    {
        return signal.connect(h);
    }
protected:
    friend class Object;

    inline static std::shared_ptr<Signal<SignalDescription, void>> make_signal(const std::shared_ptr<Object>& parent,
            const std::string& interface,
            const std::string& name)
    {
        static auto sp = std::shared_ptr<Signal<SignalDescription, void>> {new Signal<SignalDescription, void>{parent, interface, name}};
        return sp;
    }
private:
    inline Signal(const std::shared_ptr<Object>& parent,
           const std::string& interface,
           const std::string& name) : parent(parent),
        interface(interface),
        name(name)
    {
        parent->signal_router.install_route(Object::SignalKey {interface, name},
                                            std::bind(&Signal<SignalDescription>::operator(), this, std::placeholders::_1));
        parent->add_match(rule.type(Message::Type::signal).interface(interface).member(name));
    }

    inline void operator()(const DBusMessage*)
    {
        signal();
    }

    std::shared_ptr<Object> parent;
    std::string interface;
    std::string name;
    MatchRule rule;
    boost::signals2::signal<void()> signal;
};

template<typename SignalDescription>
class Signal<
    SignalDescription,
    typename std::enable_if<
        is_not_void<typename SignalDescription::ArgumentType>::value,
        typename SignalDescription::ArgumentType
    >::type
>
{
public:
    typedef std::shared_ptr<Signal<SignalDescription, typename SignalDescription::ArgumentType>> Ptr;
    typedef std::function<void(const typename SignalDescription::ArgumentType&)> Handler;

    inline ~Signal() noexcept
    {
        d->parent->signal_router.uninstall_route(Object::SignalKey{d->interface, d->name});
        d->parent->remove_match(d->rule);
    }

    inline void emit(const typename SignalDescription::ArgumentType& arg)
    {
        d->parent->template emit_signal<SignalDescription, typename SignalDescription::ArgumentType>(arg);
    }

    inline signals::Connection connect(const Handler& h)
    {
        return d->signal.connect(h);
    }

protected:
    friend class Object;

    inline static std::shared_ptr<Signal<SignalDescription,typename SignalDescription::ArgumentType>>
            make_signal(
                const std::shared_ptr<Object>& parent,
                const std::string& interface,
                const std::string& name)
    {
        static auto sp = std::shared_ptr<Signal<SignalDescription, typename SignalDescription::ArgumentType>>(new Signal<SignalDescription, typename SignalDescription::ArgumentType>(parent, interface, name));
        return sp;
    }

private:
    inline Signal(const std::shared_ptr<Object>& parent,
           const std::string& interface,
           const std::string& name) : d {new Shared{parent, interface, name}}
    {
        d->parent->signal_router.install_route(Object::SignalKey {interface, name},
        std::bind(&Signal<SignalDescription, typename SignalDescription::ArgumentType>::operator(), this, std::placeholders::_1));
        d->parent->add_match(d->rule.type(Message::Type::signal).interface(interface).member(name));
    }

    inline void operator()(DBusMessage* msg) noexcept
    {
        DBusMessageIter iter;
        dbus_message_iter_init(msg, std::addressof(iter));
        try
        {
            decode_message(std::addressof(iter), d->value);
            d->signal(d->value);
        }
        catch (const std::runtime_error& e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    struct Shared
    {
        inline Shared(const std::shared_ptr<Object>& parent, const std::string& interface, const std::string& name)
            : parent(parent),
              interface(interface),
              name(name)
        {
        }

        typename SignalDescription::ArgumentType value;
        std::shared_ptr<Object> parent;
        std::string interface;
        std::string name;
        MatchRule rule;
        boost::signals2::signal<void(const typename SignalDescription::ArgumentType&)> signal;
    };
    std::shared_ptr<Shared> d;
};


inline Service::Service(const Bus::Ptr& connection, const std::string& name)
    : connection(connection),
      name(name),
      stub(true)
{

}

inline Service::Service(const Bus::Ptr& connection, const std::string& name, const Service::RequestNameFlags& flags)
    : connection(connection),
      name(name),
      stub(false)
{
    struct Scope
    {
        Scope()
        {
            dbus_error_init(std::addressof(error));
        }

        ~Scope()
        {
            dbus_error_free(std::addressof(error));
        }

        DBusError error;
    } scope;
    dbus_bus_request_name(connection->raw(), name.c_str(), flags.to_ulong(), std::addressof(scope.error));

    if (dbus_error_is_set(std::addressof(scope.error)))
        throw std::runtime_error(std::string(scope.error.name) + ": " + std::string(scope.error.message));
}

inline const std::shared_ptr<Object>& Service::root_object()
{
    if (!root)
        root = std::shared_ptr<Object>(new Object(shared_from_this(), types::ObjectPath::root()));
    return root;
}

inline std::shared_ptr<Object> Service::object_for_path(const types::ObjectPath& path)
{
    return std::shared_ptr<Object>(new Object(shared_from_this(), path));
}

inline std::shared_ptr<Object> Service::add_object_for_path(const types::ObjectPath& path)
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

    struct Scope
    {
        Scope()
        {
            dbus_error_init(std::addressof(error));
        }

        ~Scope()
        {
            dbus_error_free(std::addressof(error));
        }

        DBusError error;
    } scope;

    auto result = dbus_connection_try_register_object_path(
                      connection->raw(),
                      path.as_string().c_str(),
                      vtable,
                      object.get(),
                      std::addressof(scope.error));

    if (!result)
    {
        delete vtable;
        throw std::runtime_error(std::string(scope.error.name) + ": " + std::string(scope.error.message));
    }

    return object;
}
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_SERVICE_H_
