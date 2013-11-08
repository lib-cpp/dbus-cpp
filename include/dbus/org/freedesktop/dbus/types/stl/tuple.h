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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_TYPES_STL_TUPLE_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_TYPES_STL_TUPLE_H_

#include "org/freedesktop/dbus/codec.h"
#include "org/freedesktop/dbus/helper/type_mapper.h"

#include <tuple>

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace helper
{
namespace detail
{
template<typename Tuple, size_t n, size_t size>
struct StaticRecursiveSignature
{
    static std::string signature()
    {
        Tuple t;
        typedef decltype(std::get<size-n-1>(t)) ElementType;
        static const std::string s = TypeMapper<typename std::decay<ElementType>::type>::signature() + StaticRecursiveSignature<Tuple, n-1, size>::signature();
        return s;
    }
};

template<typename Tuple, size_t size>
struct StaticRecursiveSignature<Tuple, 0, size>
{
    static std::string signature()
    {
        Tuple t;
        typedef decltype(std::get<size-1>(t)) ElementType;
        static const std::string s = TypeMapper<typename std::decay<ElementType>::type>::signature();
        return s;
    }
};
}
template<typename... Args>
struct TypeMapper<std::tuple<Args...>>
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
            detail::StaticRecursiveSignature<std::tuple<Args...>, sizeof...(Args)-1, sizeof...(Args)>::signature();
        return s;
    }
};
}
namespace detail
{

template<typename Tuple, size_t n, size_t size>
struct CodecApply
{
    static void decode_argument(Message::Reader& in, Tuple& t)
    {
        typedef decltype(std::get<size-n-1>(t)) ElementType;
        Codec<typename std::decay<ElementType>::type>::decode_argument(in, std::get<size-n-1>(t));
        CodecApply<Tuple, n-1, size>::decode_argument(in, t);
    }

    static void encode_argument(Message::Writer& out, const Tuple& t)
    {
        typedef decltype(std::get<size-n-1>(t)) ElementType;
        Codec<typename std::decay<ElementType>::type>::encode_argument(out, std::get<size-n-1>(t));
        CodecApply<Tuple, n-1, size>::encode_argument(out, t);
    }
};

template<typename Tuple, size_t size>
struct CodecApply<Tuple, 0, size>
{
    static void decode_argument(Message::Reader& in, Tuple& t)
    {
        typedef decltype(std::get<size-1>(t)) ElementType;
        Codec<typename std::decay<ElementType>::type>::decode_argument(in, std::get<size-1>(t));
    }

    static void encode_argument(Message::Writer& out, const Tuple& t)
    {
        typedef decltype(std::get<size-1>(t)) ElementType;
        Codec<typename std::decay<ElementType>::type>::encode_argument(out, std::get<size-1>(t));
    }
};
}

template<typename... Args>
struct Codec<std::tuple<Args...>>
{
    static void encode_argument(Message::Writer& out, const std::tuple<Args...>& arg)
    {
        detail::CodecApply<std::tuple<Args...>, sizeof...(Args)-1, sizeof...(Args)>::encode_argument(out, arg);
    }

    static void decode_argument(Message::Reader& in, std::tuple<Args...>& out)
    {
        detail::CodecApply<std::tuple<Args...>, sizeof...(Args)-1, sizeof...(Args)>::decode_argument(in, out);
    }
};
}
}
}
#endif // DBUS_ORG_FREEDESKTOP_DBUS_TYPES_STL_TUPLE_H_
