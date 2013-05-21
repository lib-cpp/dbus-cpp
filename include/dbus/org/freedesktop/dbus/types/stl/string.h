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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_TYPES_STL_STRING_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_TYPES_STL_STRING_H_

#include "org/freedesktop/dbus/codec.h"
#include "org/freedesktop/dbus/helper/type_mapper.h"

#include <string>

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace helper
{
template<>
struct TypeMapper<std::string>
{
    constexpr static inline ArgumentType type_value()
    {
        return ArgumentType::string;
    }
    constexpr static bool is_basic_type()
    {
        return false;
    }
    constexpr static bool requires_signature()
    {
        return true;
    }

    static std::string signature()
    {
        return DBUS_TYPE_STRING_AS_STRING;
    }
};
}
template<>
struct Codec<std::string>
{
    static void encode_argument(DBusMessageIter* out, const std::string& arg)
    {
        const char* s = arg.c_str();
        dbus_message_iter_append_basic(out, static_cast<int>(helper::TypeMapper<std::string>::type_value()), &s);
    }

    static void decode_argument(DBusMessageIter* in, std::string& arg)
    {
        if (dbus_message_iter_get_arg_type(in) != static_cast<int>(ArgumentType::string))
            throw std::runtime_error("Incompatible argument type: dbus_message_iter_get_arg_type(in) != ArgumentType::string");
        char* c = nullptr;
        dbus_message_iter_get_basic(in, std::addressof(c));

        if (c != nullptr)
            arg = c;
    }
};
}
}
}
#endif // DBUS_ORG_FREEDESKTOP_DBUS_TYPES_STL_STRING_H_
