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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_TYPES_STL_MAP_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_TYPES_STL_MAP_H_

#include "org/freedesktop/dbus/codec.h"

#include <algorithm>
#include <map>

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace helper
{
template<typename T, typename U>
struct TypeMapper<std::pair<T, U>>
{
    constexpr static ArgumentType type_value()
    {
        return ArgumentType::dictionary_entry;
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
        static const std::string s = DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING + TypeMapper<typename std::decay<T>::type>::signature() + TypeMapper<typename std::decay<U>::type>::signature() + DBUS_DICT_ENTRY_END_CHAR_AS_STRING;
        return s;
    }
};

template<typename T, typename U>
struct TypeMapper<std::map<T, U>>
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
        static const std::string s = DBUS_TYPE_ARRAY_AS_STRING + TypeMapper<std::pair<T,U>>::signature();
        return s;
    }
};
}
template<typename T, typename U>
struct Codec<std::pair<T, U>>
{
    static void encode_argument(DBusMessageIter* out, const std::pair<T, U>& arg)
    {
        DBusMessageIter sub;
        if (!dbus_message_iter_open_container(
                    out,
                    DBUS_TYPE_DICT_ENTRY,
                    nullptr,
                    std::addressof(sub)))
            throw std::runtime_error("Problem opening container");

        Codec<typename std::decay<T>::type>::encode_argument(std::addressof(sub), arg.first);
        Codec<typename std::decay<U>::type>::encode_argument(std::addressof(sub), arg.second);

        if (!dbus_message_iter_close_container(out, std::addressof(sub)))
            throw std::runtime_error("Problem closing container");
    }

    static void decode_argument(DBusMessageIter* out, std::pair<T, U>& arg)
    {
        DBusMessageIter sub;
        dbus_message_iter_recurse(out, std::addressof(sub));

        Codec<T>::decode_argument(std::addressof(sub), arg.first);
        dbus_message_iter_next(std::addressof(sub));
        Codec<U>::decode_argument(std::addressof(sub), arg.second);
        dbus_message_iter_next(std::addressof(sub));
    }
};

template<typename T, typename U>
struct Codec<std::map<T, U>>
{
    static void encode_argument(DBusMessageIter* out, const std::map<T, U>& arg)
    {
        DBusMessageIter sub;
        if (!dbus_message_iter_open_container(
                    out,
                    DBUS_TYPE_ARRAY,
                    helper::TypeMapper<typename std::map<T, U>::value_type>::signature().c_str(),
                    std::addressof(sub)))
            throw std::runtime_error("Problem opening container");

        std::for_each(
            arg.begin(),
            arg.end(),
            std::bind(Codec<std::pair<T,U>>::encode_argument, std::addressof(sub), std::placeholders::_1));

        if (!dbus_message_iter_close_container(out, std::addressof(sub)))
            throw std::runtime_error("Problem closing container");
    }

    static void decode_argument(DBusMessageIter* in, std::map<T,U>& out)
    {
        if (dbus_message_iter_get_arg_type(in) != static_cast<int>(ArgumentType::array))
            throw std::runtime_error("Incompatible argument type: dbus_message_iter_get_arg_type(in) != ArgumentType::array");

        if (dbus_message_iter_get_element_type(in) != static_cast<int>(ArgumentType::dictionary_entry))
            throw std::runtime_error("Incompatible element type");

        int current_type;
        DBusMessageIter sub;
        dbus_message_iter_recurse(in, std::addressof(sub));
        while ((current_type = dbus_message_iter_get_arg_type (std::addressof(sub))) != DBUS_TYPE_INVALID)
        {
            std::pair<T, U> v;
            Codec<std::pair<T, U>>::decode_argument(std::addressof(sub), v);
            bool inserted = false;
            std::tie(std::ignore, inserted) = out.insert(v);
            if (!inserted)
                throw std::runtime_error("Could not insert decoded element into map");
            dbus_message_iter_next(std::addressof(sub));
        }
    }
};
}
}
}
#endif // DBUS_ORG_FREEDESKTOP_DBUS_TYPES_STL_MAP_H_
