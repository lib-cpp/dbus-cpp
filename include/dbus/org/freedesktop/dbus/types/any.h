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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_TYPES_ANY_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_TYPES_ANY_H_

#include "org/freedesktop/dbus/argument_type.h"
#include "org/freedesktop/dbus/codec.h"
#include "org/freedesktop/dbus/helper/apply_visitor.h"

#include <cstring>
#include <functional>
#include <ostream>
#include <tuple>

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace types
{
struct Any
{
    static void clone_value_if_string(DBusBasicValue& out, const DBusBasicValue& in, const ArgumentType& type)
    {
        out = in;
        if (type == ArgumentType::string)
            out.str = ::strdup(in.str); // FIXME(tvoss): This should be a strcpy
    }

    Any() : type(ArgumentType::invalid)
    {
    }

    Any(const Any& other) : type(other.type)
    {
        clone_value_if_string(value, other.value, other.type);
    }

    Any& operator=(const Any& other)
    {
        type = other.type;
        clone_value_if_string(value, other.value, other.type);

        return *this;
    }

    ~Any() noexcept
    {
        if (type == ArgumentType::string)
            free(value.str);
    }

    void operator()(org::freedesktop::dbus::ArgumentType t)
    {
        this->type = t;
    }

    void operator()(org::freedesktop::dbus::ArgumentType t, DBusBasicValue* v)
    {
        type = t;
        clone_value_if_string(value, *v, t);
    }

    ArgumentType type;
    DBusBasicValue value;
};
std::ostream& operator<<(std::ostream& out, const Any& any)
{
    switch (any.type)
    {
    case ArgumentType::invalid:
        out << "invalid";
        break;

    case ArgumentType::floating_point:
        out << any.value.dbl;
        break;
    case ArgumentType::byte:
        out << any.value.byt;
        break;
    case ArgumentType::boolean:
        out << std::boolalpha << static_cast<bool>(any.value.bool_val);
        break;
    case ArgumentType::int16:
        out << any.value.i16;
        break;
    case ArgumentType::uint16:
        out << any.value.u16;
        break;
    case ArgumentType::int32:
        out << any.value.i32;
        break;
    case ArgumentType::uint32:
        out << any.value.u32;
        break;
    case ArgumentType::int64:
        out << any.value.i64;
        break;
    case ArgumentType::uint64:
        out << any.value.u64;
        break;
    case ArgumentType::string:
    case ArgumentType::object_path:
    case ArgumentType::signature:
        out << any.value.str;
        break;
    default:
        break;
    }

    return out;
}
}
namespace helper
{
template<>
struct TypeMapper<org::freedesktop::dbus::types::Any>
{
    constexpr static ArgumentType type_value()
    {
        return ArgumentType::invalid;
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
        return "";
    }
};
}
template<>
struct Codec<types::Any>
{
    static void encode_argument(DBusMessageIter*, const types::Any&)
    {
    }

    static void decode_argument(DBusMessageIter* in, types::Any& any)
    {
        helper::apply_visitor(in, std::ref(any));
    };
};

}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_TYPES_ANY_H_
