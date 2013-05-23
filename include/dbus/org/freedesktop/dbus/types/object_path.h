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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_TYPES_OBJECT_PATH_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_TYPES_OBJECT_PATH_H_

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
class ObjectPath
{
public:
    inline static std::string root()
    {
        return std::string{"/"};
    }

    // Specifying a default argument here causes valgrind to report a "possible lost".
    // However, this is spurious and 
    //   http://stackoverflow.com/questions/10750299/if-i-specify-a-default-value-for-an-argument-of-type-stdstring-in-c-cou
    // gives some insight.
    ObjectPath(const std::string& path = ObjectPath::root()) : path(path)
    {
        DBusError error;
        dbus_error_init(std::addressof(error));
        if (!dbus_validate_path(path.c_str(), std::addressof(error)))
            throw std::runtime_error("Invalid object path: " + std::string(error.message));
    }

    bool empty() const
    {
        return path.empty();
    }

    const std::string& as_string() const
    {
        return path;
    }

    bool operator<(const ObjectPath& rhs) const
    {
        return path < rhs.path;
    }

    bool operator==(const ObjectPath& rhs) const
    {
        return path == rhs.path;
    }
private:
    friend struct Codec<types::ObjectPath>;

    std::string path;
};
inline std::ostream& operator<<(std::ostream& out, const ObjectPath& path)
{
    out << path.as_string() << std::endl;
    return out;
}
}
namespace helper
{
template<>
struct TypeMapper<org::freedesktop::dbus::types::ObjectPath>
{
    constexpr inline static ArgumentType type_value()
    {
        return ArgumentType::object_path;
    }
    constexpr inline static bool is_basic_type()
    {
        return false;
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
}
template<>
struct Codec<types::ObjectPath>
{
    inline static void encode_argument(DBusMessageIter* out, const types::ObjectPath& arg)
    {
        const char* s = arg.path.c_str();
        dbus_message_iter_append_basic(out, static_cast<int>(ArgumentType::object_path), &s);
    }

    inline static void decode_argument(DBusMessageIter* in, types::ObjectPath& arg)
    {
        if (dbus_message_iter_get_arg_type(in) != static_cast<int>(ArgumentType::object_path))
            throw std::runtime_error("Incompatible argument type: dbus_message_iter_get_arg_type(in) != ArgumentType::object_path");
        char* c = nullptr;
        dbus_message_iter_get_basic(in, std::addressof(c));

        if (c != nullptr)
            arg.path = c;
    }
};
}
}
}

namespace std
{
template<>
struct hash<org::freedesktop::dbus::types::ObjectPath>
{
    size_t operator()(const org::freedesktop::dbus::types::ObjectPath& p) const
    {
        static const std::hash<std::string> h {};
        return h(p.as_string());
    }
};
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_TYPES_OBJECT_PATH_H_
