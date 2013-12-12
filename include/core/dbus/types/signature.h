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
#ifndef CORE_DBUS_TYPES_SIGNATURE_H_
#define CORE_DBUS_TYPES_SIGNATURE_H_

#include <stdexcept>
#include <string>

namespace core
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
    std::string signature;
};
}
}
}

#endif // CORE_DBUS_TYPES_OBJECT_PATH_H_
