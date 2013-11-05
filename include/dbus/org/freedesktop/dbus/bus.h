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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_BUS_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_BUS_H_

#include "org/freedesktop/dbus/error.h"
#include "org/freedesktop/dbus/executor.h"
#include "org/freedesktop/dbus/message_router.h"
#include "org/freedesktop/dbus/visibility.h"
#include "org/freedesktop/dbus/well_known_bus.h"

#include "org/freedesktop/dbus/types/object_path.h"

#include <cstring>

#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <map>
#include <mutex>
#include <stdexcept>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>

#include <dbus/dbus.h>

namespace org
{
namespace freedesktop
{
namespace dbus
{
/**
 * @brief The Bus class constitutes a very thin wrapper and the starting point to expose low-level DBus functionality for internal purposes.
 */
class ORG_FREEDESKTOP_DBUS_DLL_PUBLIC Bus
{
  public:
    typedef std::shared_ptr<Bus> Ptr;
    typedef MessageRouter<types::ObjectPath> SignalRouter;

    /**
     * @brief Creates a connection to a well-known bus. The implementation takes care of setting up thread-safety flags for DBus.
     * @param bus The well-known bus the instance should connect to.
     */
    explicit Bus(WellKnownBus bus);

    Bus(const Bus&) = delete;

    /**
      * @brief Disconnects from the bus and releases the connection corresponding to this instance.
      */
    ~Bus() noexcept;

    Bus& operator=(const Bus&) = delete;
    bool operator==(const Bus&) const = delete;

    /**
     * @brief Sends a raw DBus message over this DBus connection.
     * @param msg The message to send, must not be null.
     * @return A reply serial.
     * @throw std::runtime_error in case of errors.
     */
    uint32_t send(DBusMessage* msg);

    /**
     * @brief Invokes a function and blocks for a specified amount of time waiting for a result.
     * @param msg The method call.
     * @param milliseconds The timeout.
     * @return The reply message or null in case of errors.
     * @throw std::runtime_error if a timeout occurs.
     */
    DBusMessage* send_with_reply_and_block_for_at_most(
        DBusMessage* msg, 
        const std::chrono::milliseconds& milliseconds);

    /**
     * @brief Invokes a function, returning a waitable pending call that times out after the specified time period.
     * @param msg The method call.
     * @param timeout The timeout.
     * @return The waitable, pending call for this method invocation or null in case of errors.
     */
    DBusPendingCall* send_with_reply_and_timeout(
        DBusMessage* msg, 
        const std::chrono::milliseconds& timeout);

    /**
     * @brief Installs a message callback to the underlying DBus connection.
     * @param filter The callback to install.
     * @param cookie The cookie/context to be passed to the callback invocation.
     * @return true if installing the filter was successful, false otherwise.
     */
    bool install_message_filter(
        DBusHandleMessageFunction filter, 
        void* cookie) noexcept;

    /**
     * @brief Uninstalls a message callback from the underlying DBus connection.
     * @param filter The callback to be uninstalled.
     * @param cookie The cookie that further specifies the callback instance.
     */
    void uninstall_message_filter(
        DBusHandleMessageFunction filter, 
        void* cookie) noexcept;

    /**
     * @brief Installs a match rule to the underlying DBus connection.
     * @param rule The match rule to be installed, has to be a valid match rule.
     */
    void add_match(const std::string& rule);

    /**
     * @brief Uninstalls a match rule to the underlying DBus connection.
     * @param rule The match rule to be uninstalled.
     */
    void remove_match(const std::string& rule);

    /**
     * @brief Checks if the given name is owned on this bus connection.
     * @param name The name to check ownership for.
     * @return true if the name is already owned, false otherwise.
     */
    bool has_owner_for_name(const std::string& name);

    /**
     * @brief Installs an executor for this bus connection, enabling signal and method call delivery.
     * @param e The executor instance, must not be null.
     */
    void install_executor(const Executor::Ptr& e);

    /**
     * @brief Stops signal and method call delivery, i.e., stops the underlying executor if any.
     */
    void stop();

    /**
     * @brief Starts signal and method call delivery, i.e., starts the underlying executor if any.
     */
    void run();

    /**
     * @brief Provides mutable access to the contained signal router.
     */
    SignalRouter& access_signal_router();

    /**
     * @brief Provides raw, unmanaged access to the underlying DBus connection.
     */
    DBusConnection* raw() const;

  private:
    static DBusHandlerResult handle_message(
        DBusConnection*, 
        DBusMessage* message, 
        void* data);

    std::shared_ptr<DBusConnection> connection;
    Executor::Ptr executor;
    MessageRouter<int> message_type_router;
    SignalRouter signal_router;
};
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_BUS_H_
