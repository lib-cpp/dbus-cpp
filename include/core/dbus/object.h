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
#ifndef CORE_DBUS_OBJECT_H_
#define CORE_DBUS_OBJECT_H_

#include <core/dbus/bus.h>
#include <core/dbus/service.h>

#include <functional>
#include <future>
#include <map>
#include <memory>
#include <ostream>
#include <string>

namespace std
{
/**
 * @brief Template specialization of std::hash for a std::tuple<std::string, std::string>.
 */
template<>
struct hash<std::tuple<std::string, std::string>>
{
    size_t operator()(const std::tuple<std::string, std::string>& key) const
    {
        static const std::hash<std::string> h {};
        // Using XOR as we do not expect first and second to be equal.
        return h(std::get<0>(key)) ^ h(std::get<1>(key));
    }
};

/**
 * @brief Pretty prints a std::tuple<std::string, std::string>.
 * @param out The output stream to write to.
 * @param tuple The tuple to print.
 * @return Returns a reference to the output stream.
 */
inline std::ostream& operator<<(std::ostream& out,
                                const std::tuple<std::string, std::string>& tuple)
{
    out << "(" << std::get<0>(tuple) << "," << std::get<1>(tuple) << ")";
    return out;
}
}

namespace core
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
class Result;
template<typename T, typename U>
class Signal;

/**
 * @brief The Object class models a DBus object living on the bus.
 */
class Object : public std::enable_shared_from_this<Object>
{
  private:
    typedef std::tuple<std::string, std::string> MethodKey;
    typedef std::tuple<std::string, std::string> PropertyKey;
    typedef std::tuple<std::string, std::string> SignalKey;

  public:
    typedef std::shared_ptr<Object> Ptr;
    typedef std::function<void(const Message::Ptr&)> MethodHandler;

    /**
     * @brief Emits a signal with arguments for this object.
     */
    template<typename Signal, typename... Args>
    inline void emit_signal(const Args& ... args);

    /**
     * @brief Invokes a method of a remote object blocking while waiting for the result.
     * @tparam Method The method to invoke.
     * @tparam ResultType The expected type of the result.
     * @tparam Args Parameter pack of arguments passed to the invocation.
     * @param [in] args Argument instances passed to the invocation.
     * @return An invocation result, either signalling an error or containing the result of the invocation.
     */
    template<typename Method, typename ResultType, typename... Args>
    inline Result<ResultType> invoke_method_synchronously(const Args& ... args);

    /**
     * @brief Invokes a method of a remote object returning a std::future to synchronize with the result
     * @tparam Method The method to invoke.
     * @tparam ResultType The expected type of the result.
     * @tparam Args Parameter pack of arguments passed to the invocation.
     * @param [in] args Argument instances passed to the invocation.
     * @return A future wrapping an invocation result, either signalling an error or containing the result of the invocation.
     */
    template<typename Method, typename ResultType, typename... Args>
    inline std::future<Result<ResultType>> invoke_method_asynchronously(const Args& ... args);

    /**
     * @brief Accesses a property of the object.
     * @return An instance of the property or nullptr in case of errors.
     */
    template<typename PropertyDescription>
    std::shared_ptr<Property<PropertyDescription>>
    inline get_property();

    /**
     * @brief Queries all properties in one go for the object.
     */
    template<typename Interface>
    std::map<std::string, types::Variant<types::Any>>
    inline get_all_properties();

    /**
     * @brief Accesses a signal of the object.
     * @return An instance of the signal or nullptr in case of errors.
     */
    template<typename SignalDescription>
    const std::shared_ptr<Signal<SignalDescription, typename SignalDescription::ArgumentType>>
    inline get_signal();

    /**
     * @brief Adds an object as a child of this object.
     * @param [in] path The path to associate the object with.
     * @return An object instance or nullptr in case of errors.
     */
    std::shared_ptr<Object> 
    inline add_object_for_path(const types::ObjectPath& path);

    /**
     * @brief Installs an implementation for a specific method of this object instance.
     * @tparam Method The method to install the implementation for.
     * @param [in] handler The implementation.
     */
    template<typename Method>
    inline void install_method_handler(const MethodHandler& handler);

    /**
     * @brief Uninstalls an implementation for a specific method of this object instance.
     * @tparam Method The method to uninstall the implementation for.
     */
    template<typename Method>
    inline void uninstall_method_handler();

    /**
     * @brief Queries whether this object is a stub instance.
     * @return true if this object is a stub instance, false otherwise.
     */
    inline bool is_stub() const;

    /**
     * @brief Requests the object to process a message
     * @param msg The message to be processed.
     * @return true iff the msg has been handled.
     */
    inline bool on_new_message(const Message::Ptr& msg);

    /**
     * @return object path of the Object
     */
    inline const types::ObjectPath& path() const;

  private:
    friend class Service;
    template<typename T, typename U> friend class Signal;
    template<typename T> friend class Property;

    Object(const std::shared_ptr<Service> parent, const types::ObjectPath& path);

    void add_match(const MatchRule& rule);
    void remove_match(const MatchRule& rule);
    void on_properties_changed(
            const interfaces::Properties::Signals::PropertiesChanged::ArgumentType&);

    std::shared_ptr<Service> parent;
    types::ObjectPath object_path;    
    MessageRouter<SignalKey> signal_router;
    MessageRouter<MethodKey> method_router;
    MessageRouter<PropertyKey> get_property_router;
    MessageRouter<PropertyKey> set_property_router;
    std::map<
        std::tuple<std::string, std::string>,
        std::function<void(const types::Variant<types::Any>&)>
    > property_changed_vtable;
    std::shared_ptr<
        Signal<
            interfaces::Properties::Signals::PropertiesChanged,
            interfaces::Properties::Signals::PropertiesChanged::ArgumentType
        >
    > signal_properties_changed;
};
}
}

#include "impl/object.h"

#endif // CORE_DBUS_OBJECT_H_
