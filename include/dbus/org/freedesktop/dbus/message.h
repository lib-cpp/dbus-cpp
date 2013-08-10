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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_MESSAGE_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_MESSAGE_H_

#include "org/freedesktop/dbus/codec.h"
#include "org/freedesktop/dbus/types/object_path.h"

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

class Message : public std::enable_shared_from_this<Message>
{
public:
    typedef std::shared_ptr<Message> Ptr;

    enum class Type : int
    {
        invalid = DBUS_MESSAGE_TYPE_INVALID,
        signal = DBUS_MESSAGE_TYPE_SIGNAL,
        method_call = DBUS_MESSAGE_TYPE_METHOD_CALL,
        method_return = DBUS_MESSAGE_TYPE_METHOD_RETURN,
        error = DBUS_MESSAGE_TYPE_ERROR
    };

    class Reader
    {
    public:
        Reader(const Reader&) = default;
        Reader& operator=(const Reader&) = default;

        template<typename T>
        Reader& operator>>(T& t);

    protected:
        friend class Message;
        explicit Reader(const std::shared_ptr<Message>& msg);

    private:
        std::shared_ptr<Message> message;
        DBusMessageIter iter;
    };

    class Writer
    {
    public:
        Writer(const Writer&) = default;
        Writer& operator=(const Writer&) = default;

        template<typename T>
        Writer& operator<<(const T& t);

        template<typename... Args>
        Writer& append(const Args& ... args);

    protected:
        friend class Message;
        explicit Writer(const std::shared_ptr<Message>& msg);

    private:
        std::shared_ptr<Message> message;
        DBusMessageIter iter;
    };

    static std::shared_ptr<Message> make_method_call(
        const std::string& destination,
        const std::string& path,
        const std::string& interface,
        const std::string& method);

    static std::shared_ptr<Message> make_method_return(DBusMessage* msg);

    static std::shared_ptr<Message> make_signal(
        const std::string& path, 
        const std::string& interface, 
        const std::string& signal);

    static std::shared_ptr<Message> make_error(
        DBusMessage* in_reply_to, 
        const std::string& error_name, 
        const std::string& error_desc);

    static std::shared_ptr<Message> from_raw_message(DBusMessage* msg);

    Type type() const;
    bool expects_reply() const;
    types::ObjectPath path() const;
    std::string member() const;
    std::string signature() const;
    std::string interface() const;
    std::string destination() const;
    std::string sender() const;

    Reader reader();
    Writer writer();

    DBusMessage* get() const;

private:
    Message(
        DBusMessage* msg, 
        bool ref_on_construction = false);
    
    std::shared_ptr<DBusMessage> dbus_message;
};

std::ostream& operator<<(std::ostream& out, const Message::Type& type);
}
}
}

#include "impl/message.h"

#endif // DBUS_ORG_FREEDESKTOP_DBUS_MESSAGE_H_
