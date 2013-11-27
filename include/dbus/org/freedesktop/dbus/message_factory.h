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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_MESSAGE_FACTORY_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_MESSAGE_FACTORY_H_

#include <org/freedesktop/dbus/message.h>
#include <org/freedesktop/dbus/visibility.h>
#include <org/freedesktop/dbus/types/object_path.h>

namespace org
{
namespace freedesktop
{
namespace dbus
{
class MessageFactory
{
public:
    MessageFactory(const MessageFactory&) = delete;
    virtual ~MessageFactory() = default;

    MessageFactory& operator=(const MessageFactory&) = delete;
    bool operator==(const MessageFactory&) const = delete;

    /**
     * @brief make_method_call creates an instance of Message with type Type::method_call.
     * @param destination The name of the remote service to send the message to.
     * @param path The name of the remote object to send the message to.
     * @param interface The interface to route the message to.
     * @param method The actual method that should be invoked
     * @return An instance of message of type Type::method_call.
     * @throw std::runtime_error if any of the parameters violates the DBus specification.
     */
    virtual MessagePtr make_method_call(
        const std::string& destination,
        const types::ObjectPath& path,
        const std::string& interface,
        const std::string& method) = 0;

    /**
     * @brief make_method_return creates a message instance in response to a raw DBus message of type method-call.
     * @param msg The message to reply to, must not be null. Must be of type Type::method_call.
     * @return An instance of message of type Type::method_return.
     */
    virtual MessagePtr make_method_return(const MessagePtr& msg) = 0;

    /**
     * @brief make_signal creates a message instance wrapping a signal emission.
     * @param path The path of the object emitting the signal.
     * @param interface The interface containing the signal.
     * @param signal The actual signal name.
     * @return An instance of message of type Type::signal.
     */
    virtual MessagePtr make_signal(
        const std::string& path,
        const std::string& interface,
        const std::string& signal) = 0;

    /**
     * @brief make_error creates an error message instance in response to a raw DBus message of type method-call.
     * @param in_reply_to The message to reply to, must not be null. Must be of type Type::method_call.
     * @param error_name The name of the error.
     * @param error_desc Human-readable description of the error.
     * @return An instance of message of type Type::error.
     */
    virtual MessagePtr make_error(
        const MessagePtr& in_reply_to,
        const std::string& error_name,
        const std::string& error_desc) = 0;

protected:
    MessageFactory() = default;
};
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_MESSAGE_FACTORY_H_
