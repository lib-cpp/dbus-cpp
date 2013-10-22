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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_TYPES_VARIANT_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_TYPES_VARIANT_H_

#include "org/freedesktop/dbus/argument_type.h"
#include "org/freedesktop/dbus/codec.h"
#include "org/freedesktop/dbus/helper/signature.h"
#include "org/freedesktop/dbus/types/any.h"

#include <cstring>

#include <memory>
#include <stdexcept>

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace types
{
template<typename T = org::freedesktop::dbus::types::Any>
class Variant
{
public:
    explicit Variant(const T& value = T()) : value(value)
    {
    }

    const T& get() const
    {
        return value;
    }

    void set(const T& new_value)
    {
        value = new_value;
    }

    bool operator==(const Variant<T>& rhs) const
    {
        return value == rhs.value;
    }
private:
    friend struct Codec<types::Variant<T>>;
    T value;
};
}
namespace helper
{
template<typename T>
struct TypeMapper<org::freedesktop::dbus::types::Variant<T>>
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
template<typename T>
struct Codec<types::Variant<T>>
{
    inline static void encode_argument(DBusMessageIter* out, const types::Variant<T>& arg)
    {
        DBusMessageIter sub;
        if (!dbus_message_iter_open_container(
                    out,
                    static_cast<int>(ArgumentType::variant),
                    helper::TypeMapper<T>::signature().c_str(),
                    std::addressof(sub)))
            throw std::runtime_error("No memory available to open container");

        Codec<T>::encode_argument(std::addressof(sub), arg.value);

        if (!dbus_message_iter_close_container(out, std::addressof(sub)))
            throw std::runtime_error("Problem closing container");
    }

    inline static void decode_argument(DBusMessageIter* in, types::Variant<T>& arg)
    {
        if (dbus_message_iter_get_arg_type(in) != static_cast<int>(ArgumentType::variant))
            throw std::runtime_error("Incompatible argument type: dbus_message_iter_get_arg_type(in) != ArgumentType::variant");
        DBusMessageIter sub;
        dbus_message_iter_recurse(in, std::addressof(sub));
        char* signature = dbus_message_iter_get_signature(std::addressof(sub));
        std::shared_ptr<char> holder(signature, [](char* p)
        {
            dbus_free(p);
        });
        if (std::strncmp(signature, helper::TypeMapper<T>::signature().c_str(), helper::TypeMapper<T>::signature().size()) != 0)
            throw std::runtime_error("Mismatching signatures while decoding variant");

        Codec<T>::decode_argument(std::addressof(sub), arg.value);
    }
};
}
}
}
#endif // DBUS_ORG_FREEDESKTOP_DBUS_TYPES_VARIANT_H_
