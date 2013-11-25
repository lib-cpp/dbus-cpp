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
#include "org/freedesktop/dbus/visibility.h"

#include "org/freedesktop/dbus/interfaces/properties.h"
#include "org/freedesktop/dbus/traits/service.h"
#include "org/freedesktop/dbus/types/any.h"
#include "org/freedesktop/dbus/types/object_path.h"
#include "org/freedesktop/dbus/types/variant.h"
#include "org/freedesktop/dbus/types/stl/map.h"
#include "org/freedesktop/dbus/types/stl/string.h"

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
class Object;

/**
 * \brief Represents a service available on the bus.
 * \example geoclue/main.cpp
 * Provides an example of accessing the Geoclue session service on the bus.
 * \example upower/main.cpp
 * Provides an example of access the UPower system service on the bus.
*/
class ORG_FREEDESKTOP_DBUS_DLL_PUBLIC Service : public std::enable_shared_from_this<Service>
{
public: 
    typedef std::shared_ptr<Service> Ptr;    

    /**
     * @brief Exposes a service on the bus.
     * @returns An instance of Service or nullptr in case of errors.
     * @tparam Interface Needs to a model of concept Service.
     * @param [in] connection Bus to expose the service upon.
     * @param [in] flags Flags specifying behavior when requesting a name on the bus.
     */
    template<typename Interface>
    inline static Ptr add_service(
        const Bus::Ptr& connection,
        const Bus::RequestNameFlag& flags =
            Bus::RequestNameFlag::do_not_queue |
            Bus::RequestNameFlag::replace_existing)
    {
        static Ptr instance(
            new Service(
                connection, 
                traits::Service<Interface>::interface_name(), 
                flags));
        return instance;
    }

    /**
     * @brief Provides access to a service on the bus via a proxy object.
     * @returns An instance of Service or nullptr in case of errors.
     * @tparam Interface Needs to be a model of concept Service.
     * @param [in] connection Bus to access the service upon.
     */
    template<typename Interface>
    inline static Ptr use_service(const Bus::Ptr& connection)
    {
        return use_service(connection, traits::Service<Interface>::interface_name());
    }

    /**
     * @brief Provides access to a service on the bus via a proxy object.
     * @returns An instance of Service or nullptr in case of errors.
     * @param [in] connection Bus to access the service upon.
     * @param [in] name Well-known name of the service on the bus.
     */
    static Ptr use_service(const Bus::Ptr& connection, const std::string& name);

    /**
     * @brief Provides access to a service on the bus via a proxy object.
     * @returns An instance of Service.
     * @throw std::runtime_error in case of errors.
     * @param [in] connection Bus to access the service upon.
     * @param [in] name Well-known name of the service on the bus.
     */
    static Ptr use_service_or_throw_if_not_available(const Bus::Ptr& connection, const std::string& name);

    /**
     * @brief Provides access to the root object of this service instance.
     */
    const std::shared_ptr<Object>& root_object();

    /**
     * @brief Access an existing object on the specified path.
     * @param [in] path The path to access the object upon.
     * @return An instance to the object or nullptr if no object on that path exists.
     */
    std::shared_ptr<Object> object_for_path(const types::ObjectPath& path);

    /**
     * @brief Adds a new object on the specified path.
     * @param [in] path The path to mount the object upon.
     * @return An instance to the object or nullptr if the object could not be added.
     */
    std::shared_ptr<Object> add_object_for_path(const types::ObjectPath& path);

    /**
     * @brief Non-mutable access to the name of the service.
     */
    const std::string& get_name() const;

protected:
    friend class Bus;
    friend class Object;
    template<typename T> friend class Property;

    Service(const Bus::Ptr& connection, const std::string& name);
    Service(const Bus::Ptr& connection, const std::string& name, const Bus::RequestNameFlag& flags);

    bool is_stub() const;

    const Bus::Ptr& get_connection() const;

    void add_match(const MatchRule& rule);
    void remove_match(const MatchRule& rule);

private:
    Bus::Ptr connection;
    std::string name;
    std::shared_ptr<Object> root;
    bool stub;
};
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_SERVICE_H_
