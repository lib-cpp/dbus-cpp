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

#include <org/freedesktop/dbus/message.h>
#include <org/freedesktop/dbus/helper/type_mapper.h>

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
     * @param out Output writer into the outgoing message.
     * @param arg The argument to encode.
     * @throw std::runtime_error in case of errors.
     */
    static void encode_argument(Message::Writer& out, const T& arg);

    /**
     * @brief Decodes an argument from a DBus message.
     * @param in Input iterator into the incoming message.
     * @param arg The argument to decode to.
     * @throw std::runtime_error in case of errors.
     */
    static void decode_argument(Message::Reader& in, T& arg);
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
    inline static void encode_argument(Message::Writer&)
    {
    }

    /**
     * @brief Does nothing
     */
    inline static void decode_argument(Message::Reader&)
    {
    }
};

/**
 * @brief Template specialization for byte argument types.
 */
template<>
struct Codec<std::int8_t>
{
    inline static void encode_argument(Message::Writer& out, const std::int8_t& value)
    {
        out.push_byte(value);
    }

    inline static void decode_argument(Message::Reader& in, std::int8_t& value)
    {
        value = in.pop_byte();
    }
};

/**
 * @brief Template specialization for boolean argument types.
 */
template<>
struct Codec<bool>
{
    inline static void encode_argument(Message::Writer& out, bool value)
    {
        out.push_boolean(value);
    }

    inline static void decode_argument(Message::Reader& in, bool& value)
    {
        value = in.pop_boolean();
    }
};

/**
 * @brief Template specialization for int16 argument types.
 */
template<>
struct Codec<std::int16_t>
{
    inline static void encode_argument(Message::Writer& out, std::int16_t value)
    {
        out.push_int16(value);
    }

    inline static void decode_argument(Message::Reader& in, std::int16_t& value)
    {
        value = in.pop_int16();
    }
};

/**
 * @brief Template specialization for uint16 argument types.
 */
template<>
struct Codec<std::uint16_t>
{
    inline static void encode_argument(Message::Writer& out, std::uint16_t value)
    {
        out.push_uint16(value);
    }

    inline static void decode_argument(Message::Reader& in, std::uint16_t& value)
    {
        value = in.pop_uint16();
    }
};

/**
 * @brief Template specialization for int32 argument types.
 */
template<>
struct Codec<std::int32_t>
{
    inline static void encode_argument(Message::Writer& out, std::int32_t value)
    {
        out.push_int32(value);
    }

    inline static void decode_argument(Message::Reader& in, std::int32_t& value)
    {
        value = in.pop_int32();
    }
};

/**
 * @brief Template specialization for uint32 argument types.
 */
template<>
struct Codec<std::uint32_t>
{
    inline static void encode_argument(Message::Writer& out, std::uint32_t value)
    {
        out.push_uint32(value);
    }

    inline static void decode_argument(Message::Reader& in, std::uint32_t& value)
    {
        value = in.pop_uint32();
    }
};

/**
 * @brief Template specialization for int64 argument types.
 */
template<>
struct Codec<std::int64_t>
{
    inline static void encode_argument(Message::Writer& out, std::int64_t value)
    {
        out.push_int64(value);
    }

    inline static void decode_argument(Message::Reader& in, std::int64_t& value)
    {
        value = in.pop_int64();
    }
};

/**
 * @brief Template specialization for uint64 argument types.
 */
template<>
struct Codec<std::uint64_t>
{
    inline static void encode_argument(Message::Writer& out, std::uint64_t value)
    {
        out.push_uint64(value);
    }

    inline static void decode_argument(Message::Reader& in, std::uint64_t& value)
    {
        value = in.pop_uint64();
    }
};

/**
 * @brief Template specialization for floating point argument types.
 */
template<>
struct Codec<double>
{
    inline static void encode_argument(Message::Writer& out, double value)
    {
        out.push_floating_point(value);
    }

    inline static void decode_argument(Message::Reader& in, double& value)
    {
        value = in.pop_floating_point();
    }
};

/**
 * @brief Template specialization for floating point argument types.
 */
template<>
struct Codec<float>
{
    inline static void encode_argument(Message::Writer& out, float value)
    {
        out.push_floating_point(value);
    }

    inline static void decode_argument(Message::Reader& in, float& value)
    {
        value = in.pop_floating_point();
    }
};

/**
 * @brief Template specialization for object path argument types.
 */
template<>
struct Codec<types::ObjectPath>
{
    inline static void encode_argument(Message::Writer& out, const types::ObjectPath& value)
    {
        out.push_object_path(value);
    }

    inline static void decode_argument(Message::Reader& in, types::ObjectPath& value)
    {
        value = in.pop_object_path();
    }
};

/**
 * @brief Template specialization for object path argument types.
 */
template<>
struct Codec<types::Signature>
{
    inline static void encode_argument(Message::Writer& out, const types::Signature& value)
    {
        out.push_signature(value);
    }

    inline static void decode_argument(Message::Reader& in, types::Signature& value)
    {
        value = in.pop_signature();
    }
};

/**
 * @brief Template specialization for object path argument types.
 */
template<>
struct Codec<types::UnixFd>
{
    inline static void encode_argument(Message::Writer& out, const types::UnixFd& value)
    {
        out.push_unix_fd(value);
    }

    inline static void decode_argument(Message::Reader& in, types::UnixFd& value)
    {
        value = in.pop_unix_fd();
    }
};

/**
 * @brief Template specialization for variant argument types.
 */
template<typename T>
struct Codec<types::Variant<T>>
{
    inline static void encode_argument(Message::Writer& out, const types::Variant<T>& value)
    {
        auto vw = out.open_variant(
                    types::Signature(helper::TypeMapper<T>::signature()));
        {
            Codec<T>::encode_argument(vw, value.get());
        }
        out.close_variant(std::move(vw));
    }

    inline static void decode_argument(Message::Reader& in, types::Variant<T>& value)
    {
        auto vr = in.pop_variant(); T inner_value;
        Codec<T>::decode_argument(vr, inner_value);
        value.set(inner_value);
    }
};

/**
 * @brief Template specialization for any argument types.
 */
template<>
struct Codec<types::Any>
{
    inline static void encode_argument(Message::Writer& out, const types::Any& value)
    {
        (void) out;
        (void) value;
    }

    inline static void decode_argument(Message::Reader& in, types::Any& value)
    {
        (void) in;
        (void) value;
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
inline void encode_argument(Message::Writer& out, const T& arg)
{
    Codec<typename std::decay<T>::type>::encode_argument(out, arg);
}

/**
 * @brief Special overload to end compile-time recursion.
 */
inline void encode_message(Message::Writer&) {}

/**
 * @brief Compile-time recursion to encode a parameter pack into a DBus message.
 * @tparam Arg The first argument to encode.
 * @tparam Args The remaining arguments to encode.
 * @throw std::runtime_error as propagated by Codec<Arg>::encode_argument.
 */
template<typename Arg, typename... Args>
inline void encode_message(Message::Writer& out, const Arg& arg, Args... params)
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
inline void decode_argument(Message::Reader& out, T& arg)
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
inline T decode_argument(Message::Reader& out)
{
    T arg;
    Codec<T>::decode_argument(out, arg);

    return arg;
}

/**
 * @brief Special overload to end compile-time recursion.
 */
inline void decode_message(Message::Reader&) {}

/**
 * @brief Compile-time recursion to decode a parameter pack from a DBus message.
 * @tparam Arg The first argument to decode.
 * @tparam Args The remaining arguments to decode.
 * @throw std::runtime_error as propagated by Codec<Arg>::decode_argument.
 */
template<typename Arg, typename... Args>
inline void decode_message(Message::Reader& out, Arg& arg, Args& ... params)
{
    decode_argument(out, arg); decode_message(out, params...);
}
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_CODEC_H_
