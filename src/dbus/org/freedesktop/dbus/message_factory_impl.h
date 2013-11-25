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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_MESSAGE_FACTORY_IMPL_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_MESSAGE_FACTORY_IMPL_H_

#include <org/freedesktop/dbus/message_factory.h>

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace impl
{
struct MessageFactory : public org::freedesktop::dbus::MessageFactory
{
    MessageFactory() = default;

    inline std::shared_ptr<Message> make_method_call(
            const std::string& destination,
            const types::ObjectPath& path,
            const std::string& interface,
            const std::string& method)
    {
        return std::shared_ptr<Message>(
                    new Message(
                        dbus_message_new_method_call(
                            destination.c_str(),
                            path.as_string().c_str(),
                            interface.c_str(),
                            method.c_str())));
    }

    inline std::shared_ptr<Message> make_method_return(const Message::Ptr& msg)
    {
        return std::shared_ptr<Message>(
                    new Message(
                        dbus_message_new_method_return(msg->get())));
    }

    inline std::shared_ptr<Message> make_signal(
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

    inline std::shared_ptr<Message> make_error(
            const Message::Ptr& in_reply_to,
            const std::string& error_name,
            const std::string& error_desc)
    {
        return std::shared_ptr<Message>(
                    new Message(
                        dbus_message_new_error(
                            in_reply_to->get(),
                            error_name.c_str(),
                            error_desc.c_str())));
    }
};
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_MESSAGE_FACTORY_H_
