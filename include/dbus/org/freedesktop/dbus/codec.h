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
/**
 * @brief A templated class that allows for defining encoding/decoding of arbitrary types to the DBus type system.
 * @tparam T The type for which encoding and decoding should be specified.
 */
template<typename T>
struct Codec
{
    /**
     * @brief Encodes an argument to a DBus message
     * @param out Output iterator into the outgoing message.
     * @param arg The argument to encode.
     * @throw std::runtime_error in case of errors.
     */
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

    /**
     * @brief Decodes an argument from a DBus message.
     * @param in Input iterator into the incoming message.
     * @param arg The argument to decode to.
     * @throw std::runtime_error in case of errors.
     */
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

/**
 * @brief Template specialization for void argument types.
 */
template<>
struct Codec<void>
{
    /**
     * @brief Does nothing
     */
    inline static void encode_argument(DBusMessageIter*)
    {
    }

    /**
     * @brief Does nothing
     */
    inline static void decode_argument(DBusMessageIter*)
    {
    }
};

/**
 * @brief Template specialization for bool argument types.
 */
template<>
struct Codec<bool>
{
    /**
     * @brief Encodes the boolean argument as a DBUS_TYPE_BOOLEAN.
     * @param out Output iterator to write to.
     * @param arg The boolean argument.
     * @throw std::runtime_error if not enough memory is available to append a boolean argument to the message/iterator.
     */
    inline static void encode_argument(DBusMessageIter* out, const bool& arg)
    {
        dbus_bool_t value = arg ? TRUE : FALSE;
        if (!dbus_message_iter_append_basic(out, DBUS_TYPE_BOOLEAN, std::addressof(value)))
            throw std::runtime_error("Not enough memory when appending basic type to message");
    }

    /**
     * @brief Decodes a boolean argument from the message and stores it into the supplied bool.
     * @param in The input iterator into the message.
     * @param arg The argument to store the decoded value into.
     */
    inline static void decode_argument(DBusMessageIter* in, bool& arg)
    {
        dbus_bool_t value;
        dbus_message_iter_get_basic(in, std::addressof(value));
        arg = value == TRUE;
    }
};

/**
 * @brief Helper function that encodes the supplied argument relying on a Codec template specialization.
 * @tparam T Type of the argument that should be encoded.
 * @param out Output iterator to write to.
 * @param arg Argument to encode.
 * @throw std::runtime_error as thrown by Codec<T>::encode_argument.
 */
template<typename T>
inline void encode_argument(DBusMessageIter* out, const T& arg)
{
    Codec<typename std::decay<T>::type>::encode_argument(out, arg);
}

/**
 * @brief Special overload to end compile-time recursion.
 */
inline void encode_message(DBusMessageIter*) {}

/**
 * @brief Compile-time recursion to encode a parameter pack into a DBus message.
 * @tparam Arg The first argument to encode.
 * @tparam Args The remaining arguments to encode.
 * @throw std::runtime_error as propagated by Codec<Arg>::encode_argument.
 */
template<typename Arg, typename... Args>
inline void encode_message(DBusMessageIter* out, const Arg& arg, Args... params)
{
    encode_argument(out, arg);
    encode_message(out, params...);
}

/**
 * @brief Helper function that decodes the supplied argument relying on a Codec template specialization.
 * @tparam T Type of the argument that should be decoded.
 * @param in Input iterator to read from.
 * @param arg Argument to decode.
 * @throw std::runtime_error as thrown by Codec<T>::decode_argument.
 */
template<typename T>
inline void decode_argument(DBusMessageIter* out, T& arg)
{
    Codec<T>::decode_argument(out, arg);
}

/**
 * @brief Helper function that decodes an argument relying on a Codec template specialization, returning a copy of the argument.
 * @tparam T Type of the argument that should be decoded.
 * @param in Input iterator to read from.
 * @return An instance of T, filled with values decoded from the underlying message.
 * @throw std::runtime_error as thrown by Codec<T>::decode_argument.
 */
template<typename T>
inline T decode_argument(DBusMessageIter* out)
{
    T arg;
    Codec<T>::decode_argument(out, arg);

    return arg;
}

/**
 * @brief Special overload to end compile-time recursion.
 */
inline void decode_message(DBusMessageIter*) {}

/**
 * @brief Compile-time recursion to decode a parameter pack from a DBus message.
 * @tparam Arg The first argument to decode.
 * @tparam Args The remaining arguments to decode.
 * @throw std::runtime_error as propagated by Codec<Arg>::decode_argument.
 */
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
