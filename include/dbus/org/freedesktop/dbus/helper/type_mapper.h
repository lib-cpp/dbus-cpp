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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_HELPER_TYPE_MAPPER_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_HELPER_TYPE_MAPPER_H_

#include <org/freedesktop/dbus/argument_type.h>

#include <org/freedesktop/dbus/types/object_path.h>
#include <org/freedesktop/dbus/types/signature.h>
#include <org/freedesktop/dbus/types/unix_fd.h>
#include <org/freedesktop/dbus/types/variant.h>

#include <cstdint>

#include <list>
#include <map>
#include <string>
#include <tuple>
#include <vector>

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace helper
{

template<ArgumentType Type>
struct DBusTypeMapper
{
};

template<>
struct DBusTypeMapper<ArgumentType::boolean>
{
    typedef dbus_bool_t Type;
};

template<>
struct DBusTypeMapper<ArgumentType::byte>
{
    typedef int8_t Type;
};

template<>
struct DBusTypeMapper<ArgumentType::int16>
{
    typedef int16_t Type;
};

template<>
struct DBusTypeMapper<ArgumentType::uint16>
{
    typedef uint16_t Type;
};

template<>
struct DBusTypeMapper<ArgumentType::int32>
{
    typedef int32_t Type;
};

template<>
struct DBusTypeMapper<ArgumentType::uint32>
{
    typedef uint32_t Type;
};

template<>
struct DBusTypeMapper<ArgumentType::int64>
{
    typedef int64_t Type;
};

template<>
struct DBusTypeMapper<ArgumentType::uint64>
{
    typedef uint64_t Type;
};

template<>
struct DBusTypeMapper<ArgumentType::floating_point>
{
    typedef double Type;
};

template<typename T>
struct TypeMapper
{
    constexpr inline static ArgumentType type_value();
    constexpr inline static bool is_basic_type();
    constexpr inline static bool requires_signature();

    static inline std::string signature();
};

template<>
struct TypeMapper<bool>
{
    constexpr inline static ArgumentType type_value()
    {
        return ArgumentType::boolean;
    }
    constexpr inline static bool is_basic_type()
    {
        return true;
    }
    constexpr inline static bool requires_signature()
    {
        return true;
    }

    inline static std::string signature()
    {
        return DBUS_TYPE_BOOLEAN_AS_STRING;
    }
};

template<>
struct TypeMapper<std::int8_t>
{
    constexpr inline static ArgumentType type_value()
    {
        return ArgumentType::byte;
    }
    constexpr inline static bool is_basic_type()
    {
        return true;
    }
    constexpr inline static bool requires_signature()
    {
        return true;
    }

    inline static std::string signature()
    {
        return DBUS_TYPE_BYTE_AS_STRING;
    }
};

template<>
struct TypeMapper<std::int16_t>
{
    constexpr inline static ArgumentType type_value()
    {
        return ArgumentType::int16;
    }
    constexpr inline static bool is_basic_type()
    {
        return true;
    }
    constexpr inline static bool requires_signature()
    {
        return true;
    }

    inline static std::string signature()
    {
        return DBUS_TYPE_INT16_AS_STRING;
    }
};

template<>
struct TypeMapper<std::uint16_t>
{
    constexpr inline static ArgumentType type_value()
    {
        return ArgumentType::uint16;
    }
    constexpr inline static bool is_basic_type()
    {
        return true;
    }
    constexpr inline static bool requires_signature()
    {
        return true;
    }

    inline static std::string signature()
    {
        return DBUS_TYPE_UINT16_AS_STRING;
    }
};

template<>
struct TypeMapper<std::int32_t>
{
    constexpr inline static ArgumentType type_value()
    {
        return ArgumentType::int32;
    }
    constexpr inline static bool is_basic_type()
    {
        return true;
    }
    constexpr inline static bool requires_signature()
    {
        return true;
    }

    inline static std::string signature()
    {
        return DBUS_TYPE_INT32_AS_STRING;
    }
};

template<>
struct TypeMapper<std::uint32_t>
{
    constexpr inline static ArgumentType type_value()
    {
        return ArgumentType::uint32;
    }
    constexpr inline static bool is_basic_type()
    {
        return true;
    }
    constexpr inline static bool requires_signature()
    {
        return true;
    }

    inline static std::string signature()
    {
        return DBUS_TYPE_UINT32_AS_STRING;
    }
};

template<>
struct TypeMapper<std::int64_t>
{
    constexpr inline static ArgumentType type_value()
    {
        return ArgumentType::int64;
    }
    constexpr inline static bool is_basic_type()
    {
        return true;
    }
    constexpr inline static bool requires_signature()
    {
        return true;
    }

    inline static std::string signature()
    {
        return DBUS_TYPE_INT32_AS_STRING;
    }
};

template<>
struct TypeMapper<std::uint64_t>
{
    constexpr inline static ArgumentType type_value()
    {
        return ArgumentType::uint64;
    }
    constexpr inline static bool is_basic_type()
    {
        return true;
    }
    constexpr inline static bool requires_signature()
    {
        return true;
    }

    inline static std::string signature()
    {
        return DBUS_TYPE_UINT64_AS_STRING;
    }
};

template<>
struct TypeMapper<float>
{
    constexpr inline static ArgumentType type_value()
    {
        return ArgumentType::floating_point;
    }
    constexpr inline static bool is_basic_type()
    {
        return true;
    }
    constexpr inline static bool requires_signature()
    {
        return true;
    }

    inline static std::string signature()
    {
        return DBUS_TYPE_DOUBLE_AS_STRING;
    }
};

template<>
struct TypeMapper<double>
{
    constexpr inline static ArgumentType type_value()
    {
        return ArgumentType::floating_point;
    }
    constexpr inline static bool is_basic_type()
    {
        return true;
    }
    constexpr inline static bool requires_signature()
    {
        return true;
    }

    inline static std::string signature()
    {
        return DBUS_TYPE_DOUBLE_AS_STRING;
    }
};

template<>
struct TypeMapper<types::ObjectPath>
{
    constexpr inline static ArgumentType type_value()
    {
        return ArgumentType::object_path;
    }
    constexpr inline static bool is_basic_type()
    {
        return true;
    }
    constexpr inline static bool requires_signature()
    {
        return true;
    }

    inline static std::string signature()
    {
        return DBUS_TYPE_OBJECT_PATH_AS_STRING;
    }
};

template<>
struct TypeMapper<types::Signature>
{
    constexpr inline static ArgumentType type_value()
    {
        return ArgumentType::signature;
    }
    constexpr inline static bool is_basic_type()
    {
        return true;
    }
    constexpr inline static bool requires_signature()
    {
        return true;
    }

    inline static std::string signature()
    {
        return DBUS_TYPE_SIGNATURE_AS_STRING;
    }
};

template<>
struct TypeMapper<types::UnixFd>
{
    constexpr inline static ArgumentType type_value()
    {
        return ArgumentType::unix_fd;
    }
    constexpr inline static bool is_basic_type()
    {
        return true;
    }
    constexpr inline static bool requires_signature()
    {
        return true;
    }

    inline static std::string signature()
    {
        return DBUS_TYPE_UNIX_FD_AS_STRING;
    }
};

template<typename T>
struct TypeMapper<types::Variant<T>>
{
    constexpr inline static ArgumentType type_value()
    {
        return ArgumentType::variant;
    }
    constexpr inline static bool is_basic_type()
    {
        return true;
    }
    constexpr inline static bool requires_signature()
    {
        return true;
    }

    inline static std::string signature()
    {
        return DBUS_TYPE_VARIANT_AS_STRING + TypeMapper<T>::signature();
    }
};
}
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_HELPER_TYPE_MAPPER_H_
