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

#include <dbus/dbus.h>

#include <iostream>
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
/**
 * @brief The ObjectPath class encapsulates a DBus object path.
 */
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
    inline ObjectPath(const std::string& path = ObjectPath::root()) : path(path)
    {
        DBusError error;
        dbus_error_init(std::addressof(error));
        if (!dbus_validate_path(path.c_str(), std::addressof(error)))
            throw std::runtime_error("Invalid object path: " + std::string(error.message));
    }

    inline bool empty() const
    {
        return path.empty();
    }

    inline const std::string& as_string() const
    {
        return path;
    }

    inline bool operator<(const ObjectPath& rhs) const
    {
        return path < rhs.path;
    }

    inline bool operator==(const ObjectPath& rhs) const
    {
        return path == rhs.path;
    }

private:
    std::string path;
};

inline std::ostream& operator<<(std::ostream& out, const ObjectPath& path)
{
    out << path.as_string() << std::endl;
    return out;
}

}
}
}
}

namespace std
{
template<>
struct hash<org::freedesktop::dbus::types::ObjectPath>
{
    inline size_t operator()(const org::freedesktop::dbus::types::ObjectPath& p) const
    {
        static const std::hash<std::string> h {};
        return h(p.as_string());
    }
};
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_TYPES_OBJECT_PATH_H_
