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

#include <org/freedesktop/dbus/codec.h>

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
    static void encode_argument(Message::Writer& out, const std::pair<T, U>& arg)
    {
        Codec<typename std::decay<T>::type>::encode_argument(out, arg.first);
        Codec<typename std::decay<U>::type>::encode_argument(out, arg.second);
    }

    static void decode_argument(Message::Reader& in, std::pair<T, U>& arg)
    {
        Codec<T>::decode_argument(in, arg.first);
        Codec<U>::decode_argument(in, arg.second);
    }
};

template<typename T, typename U>
struct Codec<std::map<T, U>>
{
    static void encode_argument(Message::Writer& out, const std::map<T, U>& arg)
    {
        auto aw = out.open_array(
                    types::Signature(
                        helper::TypeMapper<typename std::map<T, U>::value_type>::signature()));
        {
            for (auto element : arg)
            {
                auto de = aw.open_dict_entry();
                {
                    Codec<std::pair<T,U>>::encode_argument(de, element);
                }
                aw.close_dict_entry(std::move(de));
            }
        }
        out.close_array(std::move(aw));
    }

    static void decode_argument(Message::Reader& in, std::map<T,U>& out)
    {
        auto array_reader = in.pop_array();

        while (true)
        {
            try
            {
                auto de = array_reader.pop_dict_entry();
                std::pair<T, U> v;
                Codec<std::pair<T, U>>::decode_argument(de, v);
                bool inserted = false;
                std::tie(std::ignore, inserted) = out.insert(v);
                if (!inserted)
                    throw std::runtime_error("Could not insert decoded element into map");
            } catch(...)
            {
                return;
            }
        }
    }
};
}
}
}
#endif // DBUS_ORG_FREEDESKTOP_DBUS_TYPES_STL_MAP_H_
