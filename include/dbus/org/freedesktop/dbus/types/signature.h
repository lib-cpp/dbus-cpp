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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_TYPES_SIGNATURE_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_TYPES_SIGNATURE_H_

#include "org/freedesktop/dbus/codec.h"
#include "org/freedesktop/dbus/helper/type_mapper.h"

#include <stdexcept>
#include <string>

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace types
{
class Signature
{
public:
    explicit Signature(const std::string& signature = std::string()) : signature(signature)
    {
    }

    const std::string& as_string() const
    {
        return signature;
    }

    bool operator<(const Signature& rhs) const
    {
        return signature < rhs.signature;
    }

    bool operator==(const Signature& rhs) const
    {
        return signature == rhs.signature;
    }
private:
    friend struct Codec<types::Signature>;

    std::string signature;
};
}
namespace helper
{
template<>
struct TypeMapper<org::freedesktop::dbus::types::Signature>
{
    constexpr static ArgumentType type_value()
    {
        return ArgumentType::signature;
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
        return DBUS_TYPE_SIGNATURE_AS_STRING;
    }
};
}
template<>
struct Codec<types::Signature>
{
    static void encode_argument(DBusMessageIter* out, const types::Signature& arg)
    {
        const char* s = arg.signature.c_str();
        dbus_message_iter_append_basic(out, static_cast<int>(ArgumentType::signature), &s);
    }

    static void decode_argument(DBusMessageIter* in, types::Signature& arg)
    {
        if (dbus_message_iter_get_arg_type(in) != static_cast<int>(ArgumentType::signature))
            throw std::runtime_error("Incompatible argument type: dbus_message_iter_get_arg_type(in) != ArgumentType::signature");
        char* c = nullptr;
        dbus_message_iter_get_basic(in, std::addressof(c));

        if (c != nullptr)
            arg.signature = c;
    }
};
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_TYPES_OBJECT_PATH_H_
