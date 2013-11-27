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

#include <org/freedesktop/dbus/codec.h>

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
    bool operator==(const Struct<T>& rhs) const
    {
        return value == rhs.value;
    }

    T value;
};
}
namespace helper
{
template<typename T>
struct TypeMapper<org::freedesktop::dbus::types::Struct<T>>
{
    constexpr inline static ArgumentType type_value()
    {
        return ArgumentType::structure;
    }

    constexpr inline static bool is_basic_type()
    {
        return false;
    }

    constexpr inline static bool requires_signature()
    {
        return false;
    }

    inline static std::string signature()
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
    inline static void encode_argument(Message::Writer& out, const types::Struct<T>& arg)
    {
        auto sw = out.open_structure();
        {
            Codec<T>::encode_argument(sw, arg.value);
        }
        out.close_structure(std::move(sw));
    }

    inline static void decode_argument(Message::Reader& in, types::Struct<T>& out)
    {
        auto struct_reader = in.pop_structure();
        Codec<T>::decode_argument(struct_reader, out.value);
    }
};
}
}
}
#endif // DBUS_ORG_FREEDESKTOP_DBUS_TYPES_STRUCT_H_
