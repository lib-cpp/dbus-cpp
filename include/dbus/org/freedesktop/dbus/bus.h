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

#include <org/freedesktop/dbus/error.h>
#include <org/freedesktop/dbus/executor.h>
#include <org/freedesktop/dbus/message.h>
#include <org/freedesktop/dbus/message_factory.h>
#include <org/freedesktop/dbus/message_router.h>
#include <org/freedesktop/dbus/pending_call.h>
#include <org/freedesktop/dbus/visibility.h>
#include <org/freedesktop/dbus/well_known_bus.h>

#include <org/freedesktop/dbus/types/object_path.h>

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

namespace org
{
namespace freedesktop
{
namespace dbus
{
class MatchRule;
class Object;
/**
 * @brief The Bus class constitutes a very thin wrapper and the starting
 * point to expose low-level DBus functionality for internal purposes.
 */
class ORG_FREEDESKTOP_DBUS_DLL_PUBLIC Bus
{
public:
    typedef std::shared_ptr<Bus> Ptr;

    /**
     * @brief The RequestNameFlag enum lists possible behavior when trying to acquire name on the bus.
     */
    enum class RequestNameFlag
    {
        not_set = 0,
        allow_replacement = 1 << 0, ///< Allow for later replacement by another service implementation.
        replace_existing = 1 << 1, ///< Replace any existing instance on the bus.
        do_not_queue = 1 << 2 ///< Blocking wait for service name to be acquired.
    };

    /** @brief Describes a name on the bus. */
    class Name
    {
    public:
        Name(const Name&) = delete;
        Name(Name&& rhs);

        Name& operator=(Name&& rhs);

        /**
         * @brief as_string returns a string presentation of the name.
         */
        const std::string& as_string() const;

    private:
        friend class Bus;
        Name(const std::string& name);

        std::string name;
    };

    /**
     * @brief default_request_name_flags returns defaults flags when acquiring a name on the bus.
     */
    static RequestNameFlag default_request_name_flags();

    /**
     * @brief The Errors struct summarizes exceptions that are thrown by Bus functions.
     */
    struct Errors
    {
        Errors() = delete;

        /**
         * @brief The AlreadyOwner exception if this process already owns the name on the bus.
         */
        struct AlreadyOwner : public std::runtime_error
        {
            inline AlreadyOwner()
                : std::runtime_error(
                      "This process already owns the name on the bus.")
            {
            }
        };

        /**
         * @brief The AlreadyOwned exception is thrown if this process already owns the name on the bus.
         */
        struct AlreadyOwned : public std::runtime_error
        {
            inline AlreadyOwned()
                : std::runtime_error(
                      "The name is already owned on the bus.")
            {
            }
        };

        /**
         * @brief The NoMemory exception is thrown if an operation failed due to a lack of memory.
         */
        struct NoMemory : public std::runtime_error
        {
            inline NoMemory()
                : std::runtime_error(
                      "Not enough memory to complete operation.")
            {
            }
        };

        /**
         * @brief The ObjectPathInUse exception is thrown if a path is already owned.
         */
        struct ObjectPathInUse
                : public std::runtime_error
        {
            inline ObjectPathInUse()
                : std::runtime_error(
                      "Object path is already in use.")
            {
            }
        };
    };

    /** @brief Routing of messages based on their type. */
    typedef MessageRouter<Message::Type> MessageTypeRouter;

    /** @brief Routing of signals based on a tuple of interface and member name. */
    typedef MessageRouter<types::ObjectPath> SignalRouter;

    /**
     * @brief The MessageHandlerResult enum summarizes possible replies of a MessageHandler.
     */
    enum class MessageHandlerResult
    {
        handled = DBUS_HANDLER_RESULT_HANDLED, ///< Message has had its effect - no need to run more handlers.
        not_yet_handled = DBUS_HANDLER_RESULT_NOT_YET_HANDLED, ///< Message has not had any effect - see if other handlers want it.
        need_memory = DBUS_HANDLER_RESULT_NEED_MEMORY ///< Need more memory, please try again later with more memory.
    };

    /** @brief Function signature for handling a message. */
    typedef std::function<MessageHandlerResult(const Message::Ptr& msg)> MessageHandler;

    /**
     * @brief Creates a connection to a well-known bus. The implementation takes care of setting up thread-safety flags for DBus.
     * @param bus The well-known bus the instance should connect to.
     */
    explicit Bus(WellKnownBus bus);

    // A Bus instance is not copy-able.
    Bus(const Bus&) = delete;

    /**
      * @brief Disconnects from the bus and releases the connection corresponding to this instance.
      */
    ~Bus() noexcept;

    Bus& operator=(const Bus&) = delete;
    bool operator==(const Bus&) const = delete;

    /**
     * @brief Provides access to a bus-specific message factory.
     */
    const std::shared_ptr<MessageFactory> message_factory();

    /**
     * @brief Attempts to own the given name on the bus.
     * @throw Bus::Errors::AlreadyOwner if this process already owns the name.
     * @throw Bus::Errors::AlreadyOwned if the name is already owned on the bus.
     * @return A unique instance if the ownership request completed successfully.
     */
    Name&& request_name_on_bus(
            const std::string& name,
            RequestNameFlag flags);

    /**
     * @brief Releases the previously owned name.
     * @param name The name to release.
     */
    void release_name_on_bus(Name&& name);

    /**
     * @brief Sends a raw DBus message over this DBus connection.
     * @param msg The message to send, must not be null.
     * @return A reply serial.
     * @throw std::runtime_error in case of errors.
     */
    uint32_t send(const std::shared_ptr<Message>& msg);

    /**
     * @brief Invokes a function and blocks for a specified amount of time waiting for a result.
     * @param msg The method call.
     * @param milliseconds The timeout.
     * @return The reply message or null in case of errors.
     * @throw std::runtime_error if a timeout occurs.
     */
    std::shared_ptr<Message> send_with_reply_and_block_for_at_most(
            const std::shared_ptr<Message>& msg,
            const std::chrono::milliseconds& milliseconds);

    /**
     * @brief Invokes a function, returning a waitable pending call that times out after the specified time period.
     * @param msg The method call.
     * @param timeout The timeout.
     * @return The waitable, pending call for this method invocation.
     */
    PendingCall::Ptr send_with_reply_and_timeout(
            const std::shared_ptr<Message>& msg,
            const std::chrono::milliseconds& timeout);

    /**
     * @brief Installs a match rule to the underlying DBus connection.
     * @param rule The match rule to be installed, has to be a valid match rule.
     */
    void add_match(const MatchRule& rule);

    /**
     * @brief Uninstalls a match rule to the underlying DBus connection.
     * @param rule The match rule to be uninstalled.
     */
    void remove_match(const MatchRule& rule);

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

    /**
     * @brief register_object_for_path makes the given object known for the path on the bus.
     * @throw Bus::Errors::NoMemory if not enough memory.
     * @throw Bus::Errors::ObjectPathInUse if path is already used.
     * @param path The path to make the object known for on the bus.
     * @param object The object to make known
     */
    void register_object_for_path(
            const types::ObjectPath& path,
            const std::shared_ptr<Object>& object);

    /**
     * @brief unregister_object_path removes the object known under the given name from the bus.
     * @throw Bus::Errors::NoMemory if not enough memory.
     * @param path The path to remove from the bus.
     */
    void unregister_object_path(
            const types::ObjectPath& path);

    /**
      * @brief Hands over a message to the internal message and signal routers.
      */
    MessageHandlerResult handle_message(const Message::Ptr& msg);

private:
    struct Private;
    std::unique_ptr<Private> d;
};

/** @brief Enables usage of RequestNameFlag as a bitfield. */
inline Bus::RequestNameFlag operator|(Bus::RequestNameFlag lhs, Bus::RequestNameFlag rhs)
{
    return static_cast<Bus::RequestNameFlag>(
                static_cast<unsigned int>(lhs) |
                static_cast<unsigned int>(rhs));
}
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_BUS_H_
