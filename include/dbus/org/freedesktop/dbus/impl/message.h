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

#ifndef DBUS_ORG_FREEDESKTOP_DBUS_IMPL_MESSAGE_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_IMPL_MESSAGE_H_

#include <org/freedesktop/dbus/codec.h>

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

template<typename T>
inline Message::Reader& Message::Reader::operator>>(T& t)
{
    return pop(t);
}

template<typename T>
void Message::Reader::peek(T& t)
{
    decode_argument(std::addressof(iter), t);
}

template<typename T>
inline Message::Reader& Message::Reader::pop(T& t)
{
    peek(t);
    dbus_message_iter_next(std::addressof(iter));

    return *this;
}

template<typename T>
inline Message::Writer& Message::Writer::operator<<(const T& t)
{
    encode_argument(std::addressof(iter), t);
    return *this;
}

template<typename... Args>
inline Message::Writer& Message::Writer::append(const Args& ... args)
{
    encode_message(std::addressof(iter), args...);
    return *this;
}    
}
}
}

namespace std
{
template<>
struct hash<org::freedesktop::dbus::Message::Type>
{
    size_t operator()(const org::freedesktop::dbus::Message::Type& type) const
    {
        static const hash<int> h {};
        return h(static_cast<int>(type));
    }
};
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_IMPL_MESSAGE_H_
