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
#ifndef CORE_DBUS_TYPES_STL_VECTOR_H_
#define CORE_DBUS_TYPES_STL_VECTOR_H_

#include <org/freedesktop/dbus/codec.h>
#include <org/freedesktop/dbus/helper/type_mapper.h>

#include <algorithm>
#include <vector>

namespace core
{
namespace dbus
{
namespace helper
{
template<typename T>
struct TypeMapper<std::vector<T>>
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
struct Codec<std::vector<T>>
{
    static void encode_argument(Message::Writer& out, const std::vector<T>& arg)
    {
        auto vw = out.open_array(
                    types::Signature(helper::TypeMapper<T>::signature()));
        {
            for(auto element : arg)
                core::dbus::encode_argument(vw, element);
        }
        out.close_array(std::move(vw));
    }

    static void decode_argument(Message::Reader& in, std::vector<T>& out)
    {
        Message::Reader ar = in.pop_array();

        while (true)
        {
            try
            {
                auto value = core::dbus::decode_argument<T>(ar);
                out.push_back(value);
            } catch(...)
            {
                break;
            }
        }
    }
};
}
}
#endif // CORE_DBUS_TYPES_STL_VECTOR_H_
