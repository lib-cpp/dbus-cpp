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

#include <core/dbus/types/object_path.h>

#include <core/dbus/error.h>

#include <dbus/dbus.h>

#include <iostream>

namespace core
{
namespace dbus
{
namespace types
{
const std::string& ObjectPath::root()
{
    static const std::string s{"/"};
    return s;
}

// Specifying a default argument here causes valgrind to report a "possible lost".
// However, this is spurious and
//   http://stackoverflow.com/questions/10750299/if-i-specify-a-default-value-for-an-argument-of-type-stdstring-in-c-cou
// gives some insight.
ObjectPath::ObjectPath(const std::string& path) : path(path)
{
    Error e;
    if (!dbus_validate_path(path.c_str(), std::addressof(e.raw())))
        throw ObjectPath::Errors::InvalidObjectPathStringRepresentation{path};
}

bool ObjectPath::empty() const
{
    return path.empty();
}

const std::string& ObjectPath::as_string() const
{
    return path;
}

bool ObjectPath::operator<(const ObjectPath& rhs) const
{
    return path < rhs.path;
}

bool ObjectPath::operator==(const ObjectPath& rhs) const
{
    return path == rhs.path;
}

std::ostream& operator<<(std::ostream& out, const ObjectPath& path)
{
    out << path.as_string() << std::endl;
    return out;
}
}
}
}

size_t std::hash<core::dbus::types::ObjectPath>::operator()(
        const core::dbus::types::ObjectPath& p) const
{
    static const std::hash<std::string> h {};
    return h(p.as_string());
}
