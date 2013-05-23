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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_CODEC_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_CODEC_H_

#include "org/freedesktop/dbus/helper/type_mapper.h"

#include <dbus/dbus.h>

#include <iostream>
#include <string>
#include <stdexcept>

namespace org
{
namespace freedesktop
{
namespace dbus
{
template<typename T>
struct Codec
{
    inline static void encode_argument(DBusMessageIter* out, const T& arg)
    {
        static_assert(helper::TypeMapper<T>::is_basic_type(), "Default codec only defined for basic types");
        if (std::is_same<T, typename helper::DBusTypeMapper<helper::TypeMapper<T>::type_value()>::Type>::value)
        {
            if (!dbus_message_iter_append_basic(out, static_cast<int>(helper::TypeMapper<T>::type_value()), std::addressof(arg)))
                throw std::runtime_error("Not enough memory when appending basic type to message");
        }
        else
        {
            typename helper::DBusTypeMapper<helper::TypeMapper<T>::type_value()>::Type value {arg};
            if (!dbus_message_iter_append_basic(out, static_cast<int>(helper::TypeMapper<T>::type_value()), std::addressof(value)))
                throw std::runtime_error("Not enough memory when appending basic type to message");
        }
    }

    inline static void decode_argument(DBusMessageIter* in, T& arg)
    {
        static_assert(helper::TypeMapper<T>::is_basic_type(), "Default codec only defined for basic types");
        if (std::is_same<T, typename helper::DBusTypeMapper<helper::TypeMapper<T>::type_value()>::Type>::value)
        {
            dbus_message_iter_get_basic(in, std::addressof(arg));
        }
        else
        {
            typename helper::DBusTypeMapper<helper::TypeMapper<T>::type_value()>::Type value {};
            dbus_message_iter_get_basic(in, std::addressof(value));
            arg = value;
        }
    }
};

template<>
struct Codec<void>
{
    inline static void encode_argument(DBusMessageIter*)
    {
    }

    inline static void decode_argument(DBusMessageIter*)
    {
    }
};

template<>
struct Codec<bool>
{
    inline static void encode_argument(DBusMessageIter* out, const bool& arg)
    {
        dbus_bool_t value = arg ? TRUE : FALSE;
        if (!dbus_message_iter_append_basic(out, DBUS_TYPE_BOOLEAN, std::addressof(value)))
            throw std::runtime_error("Not enough memory when appending basic type to message");
    }

    inline static void decode_argument(DBusMessageIter* in, bool& arg)
    {
        dbus_bool_t value;
        dbus_message_iter_get_basic(in, std::addressof(value));
        arg = value == TRUE;
    }
};

template<typename T>
inline void encode_argument(DBusMessageIter* out, const T& arg)
{
    Codec<typename std::decay<T>::type>::encode_argument(out, arg);
}

inline void encode_message(DBusMessageIter*) {}

template<typename Arg, typename... Args>
inline void encode_message(DBusMessageIter* out, const Arg& arg, Args... params)
{
    encode_argument(out, arg);
    encode_message(out, params...);
}

template<typename T>
inline void decode_argument(DBusMessageIter* out, T& arg)
{
    Codec<T>::decode_argument(out, arg);
}

template<typename T>
inline T decode_argument(DBusMessageIter* out)
{
    T arg;
    Codec<T>::decode_argument(out, arg);

    return arg;
}

inline void decode_message(DBusMessageIter*) {}

template<typename Arg, typename... Args>
inline void decode_message(DBusMessageIter* out, Arg& arg, Args& ... params)
{
    decode_argument(out, arg);
    dbus_message_iter_next(out);
    decode_message(out, params...);
}
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_CODEC_H_
