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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_MESSAGE_P_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_MESSAGE_P_H_

#include <org/freedesktop/dbus/message.h>

#include <sstream>

namespace org
{
namespace freedesktop
{
namespace dbus
{
struct Message::Reader::Private
{
    Private(const std::shared_ptr<Message>& msg) : msg(msg)
    {
        ::memset(std::addressof(iter), 0, sizeof(iter));
    }

    ~Private()
    {
    }

    void ensure_argument_type_or_throw(ArgumentType expected_type)
    {
        auto actual_type = static_cast<ArgumentType>(dbus_message_iter_get_arg_type(std::addressof(iter)));
        if (actual_type != expected_type)
        {
            std::stringstream ss;
            ss << "Mismatch between expected and actual type reported by iterator: " << std::endl
               << "\t Expected: " << expected_type << std::endl
               << "\t Actual: " << actual_type;
            throw std::runtime_error(ss.str());
        }
    }

    const char* pop_string_unchecked()
    {
        char* result = nullptr;
        dbus_message_iter_get_basic(
                    std::addressof(iter),
                    std::addressof(result));
        dbus_message_iter_next(std::addressof(iter));
        return result;
    }

    std::shared_ptr<Message> msg;
    DBusMessageIter iter;
};

struct Message::Writer::Private
{
    std::shared_ptr<Message> msg;
    DBusMessageIter iter;
};

struct Message::Private
{
    Private(DBusMessage* msg, bool ref_on_construction = false)
        : dbus_message(
              msg,
              [](DBusMessage* msg) {if (msg) dbus_message_unref(msg);})
    {
        if (ref_on_construction)
            dbus_message_ref(msg);
    }

    std::unique_ptr<Private> clone()
    {
        return std::unique_ptr<Message::Private>(
                    new Private(
                        dbus_message_copy(dbus_message.get())));
    }

    std::shared_ptr<DBusMessage> dbus_message;
};
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_MESSAGE_P_H_
