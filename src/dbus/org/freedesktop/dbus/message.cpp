/*
 * Copyright © 2012 Canonical Ltd->
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

#include <org/freedesktop/dbus/message.h>

#include <org/freedesktop/dbus/codec.h>
#include <org/freedesktop/dbus/error.h>
#include <org/freedesktop/dbus/visibility.h>

#include <org/freedesktop/dbus/types/object_path.h>

#include <dbus/dbus.h>

#include <exception>
#include <map>
#include <memory>
#include <ostream>
#include <stdexcept>

namespace org
{
namespace freedesktop
{
namespace dbus
{
std::ostream& operator<<(std::ostream& out, const Message::Type& type)
{
    static const std::map<Message::Type, std::string> lut =
    {
        {Message::Type::invalid, "invalid"},
        {Message::Type::signal, "signal"},
        {Message::Type::method_call, "method_call"},
        {Message::Type::method_return, "method_return"},
        {Message::Type::error, "error"}
    };
    out << lut.at(type);
    return out;
}

Message::Reader::Reader(const std::shared_ptr<Message>& msg) : message(msg)
{
    if (!msg)
        throw std::runtime_error(
                "Precondition violated, cannot construct Reader for null message.");

    if (!dbus_message_iter_init(message->dbus_message.get(), std::addressof(iter)))
        throw std::runtime_error(
                "Could not initialize reader, message does not have arguments");
}

Message::Writer::Writer(const std::shared_ptr<Message>& msg) : message(msg)
{
    if (!msg)
        throw std::runtime_error(
                "Precondition violated, cannot construct Reader for null message.");

    dbus_message_iter_init_append(
                message->dbus_message.get(),
                std::addressof(iter));
}

std::shared_ptr<Message> Message::make_method_call(
        const std::string& destination,
        const std::string& path,
        const std::string& interface,
        const std::string& method)
{
    return std::shared_ptr<Message>(
                new Message(
                    dbus_message_new_method_call(
                        destination.c_str(),
                        path.c_str(),
                        interface.c_str(),
                        method.c_str())));
}

std::shared_ptr<Message> Message::make_method_return(DBusMessage* msg)
{
    return std::shared_ptr<Message>(
                new Message(
                    dbus_message_new_method_return(msg)));
}

std::shared_ptr<Message> Message::make_signal(
        const std::string& path,
        const std::string& interface,
        const std::string& signal)
{
    return std::shared_ptr<Message>(
                new Message(
                    dbus_message_new_signal(
                        path.c_str(),
                        interface.c_str(),
                        signal.c_str())));
}

std::shared_ptr<Message> Message::make_error(
        DBusMessage* in_reply_to,
        const std::string& error_name,
        const std::string& error_desc)
{
    return std::shared_ptr<Message>(
                new Message(
                    dbus_message_new_error(
                        in_reply_to,
                        error_name.c_str(),
                        error_desc.c_str())));
}

std::shared_ptr<Message> Message::from_raw_message(DBusMessage* msg)
{
    return std::shared_ptr<Message>(new Message(msg, true));
}

Message::Type Message::type() const
{
    return static_cast<Type>(dbus_message_get_type(dbus_message.get()));
}

bool Message::expects_reply() const
{
    return !dbus_message_get_no_reply(dbus_message.get());
}

types::ObjectPath Message::path() const
{
    return types::ObjectPath(dbus_message_get_path(dbus_message.get()));
}

std::string Message::member() const
{
    return dbus_message_get_member(dbus_message.get());
}

std::string Message::signature() const
{
    return dbus_message_get_signature(dbus_message.get());
}

std::string Message::interface() const
{
    return dbus_message_get_interface(dbus_message.get());
}

std::string Message::destination() const
{
    return dbus_message_get_destination(dbus_message.get());
}

std::string Message::sender() const
{
    return dbus_message_get_sender(dbus_message.get());
}

Error Message::error() const
{
    if (type() != Message::Type::error)
        throw std::runtime_error("Message does not contain error information");

    Error result;
    dbus_set_error_from_message(std::addressof(result.raw()), dbus_message.get());

    return std::move(result);
}

Message::Reader Message::reader()
{
    return Reader(shared_from_this());
}

Message::Writer Message::writer()
{
    return Writer(shared_from_this());
}

DBusMessage* Message::get() const
{
    return dbus_message.get();
}

Message::Message(
        DBusMessage* msg,
        bool ref_on_construction)
    : dbus_message(msg, [](DBusMessage* msg) {if (msg) dbus_message_unref(msg);})
{
    if (!msg)
        throw std::runtime_error(
            "Precondition violated, cannot construct Message from null DBusMessage.");

    if (ref_on_construction)
        dbus_message_ref(msg);
}
}
}
}
