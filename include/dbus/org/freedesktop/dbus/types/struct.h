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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_TYPES_STRUCT_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_TYPES_STRUCT_H_

#include "org/freedesktop/dbus/codec.h"

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace types
{
template<typename T>
struct Struct
{
    T value;
};
}
namespace helper
{
template<typename T>
struct TypeMapper<org::freedesktop::dbus::types::Struct<T>>
{
    constexpr static ArgumentType type_value()
    {
        return ArgumentType::structure;
    }

    constexpr static bool is_basic_type()
    {
        return false;
    }

    constexpr static bool requires_signature()
    {
        return false;
    }

    static std::string signature()
    {
        static const std::string s =
            DBUS_STRUCT_BEGIN_CHAR_AS_STRING +
            TypeMapper<T>::signature() +
            DBUS_STRUCT_END_CHAR_AS_STRING;
        return s;
    }
};
}

template<typename T>
struct Codec<types::Struct<T>>
{
    static void encode_argument(DBusMessageIter* out, const types::Struct<T>& arg)
    {
        DBusMessageIter sub;
        if (!dbus_message_iter_open_container(
                    out,
                    DBUS_TYPE_STRUCT,
                    nullptr,
                    std::addressof(sub)))
            throw std::runtime_error("Problem opening container");

        Codec<T>::encode_argument(std::addressof(sub), arg.value);

        if (!dbus_message_iter_close_container(out, std::addressof(sub)))
            throw std::runtime_error("Problem closing container");
    }

    static void decode_argument(DBusMessageIter* in, types::Struct<T>& out)
    {
        DBusMessageIter sub;
        dbus_message_iter_recurse(in, std::addressof(sub));
        Codec<T>::decode_argument(std::addressof(sub), out.value);
    }
};
}
}
}
#endif // DBUS_ORG_FREEDESKTOP_DBUS_TYPES_STRUCT_H_
