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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_TYPES_STL_LIST_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_TYPES_STL_LIST_H_

#include <org/freedesktop/dbus/codec.h>

#include <algorithm>
#include <list>

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace helper
{
template<typename T>
struct TypeMapper<std::list<T>>
{
    constexpr static ArgumentType type_value()
    {
        return ArgumentType::array;
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
        static const std::string s = DBUS_TYPE_ARRAY_AS_STRING + TypeMapper<typename std::decay<T>::type>::signature();
        return s;
    }
};
}
template<typename T>
struct Codec<std::list<T>>
{
    static void encode_argument(DBusMessageIter* out, const std::list<T>& arg)
    {
        DBusMessageIter sub;
        if (!dbus_message_iter_open_container(
                    out,
                    DBUS_TYPE_ARRAY,
                    helper::TypeMapper<T>::requires_signature() ? helper::signature<T>(T()).c_str() : NULL,
                    std::addressof(sub)))
            throw std::runtime_error("Problem opening container");

        std::for_each(
            arg.begin(),
            arg.end(),
            std::bind(Codec<T>::encode_argument, std::addressof(sub), std::placeholders::_1));

        if (!dbus_message_iter_close_container(out, std::addressof(sub)))
            throw std::runtime_error("Problem closing container");
    }

    static void decode_argument(DBusMessageIter* in, std::list<T>& out)
    {
        if (dbus_message_iter_get_arg_type(in) != static_cast<int>(ArgumentType::array))
            throw std::runtime_error("Incompatible argument type: dbus_message_iter_get_arg_type(in) != ArgumentType::array");

        if (dbus_message_iter_get_element_type(in) != static_cast<int>(helper::TypeMapper<T>::type_value()))
            throw std::runtime_error("Incompatible element type");

        int current_type;
        DBusMessageIter sub;
        dbus_message_iter_recurse(in, std::addressof(sub));
        while ((current_type = dbus_message_iter_get_arg_type (std::addressof(sub))) != DBUS_TYPE_INVALID)
        {
            out.emplace_back();
            Codec<T>::decode_argument(std::addressof(sub), out.back());

            dbus_message_iter_next(std::addressof(sub));
        }
    }
};
}
}
}
#endif // DBUS_ORG_FREEDESKTOP_DBUS_TYPES_STL_LIST_H_

