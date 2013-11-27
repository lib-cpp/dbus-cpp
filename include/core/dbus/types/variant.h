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
#ifndef CORE_DBUS_TYPES_VARIANT_H_
#define CORE_DBUS_TYPES_VARIANT_H_

#include <core/dbus/types/any.h>

#include <cstring>

#include <memory>
#include <stdexcept>

namespace core
{
namespace dbus
{
namespace types
{
template<typename T = core::dbus::types::Any>
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
    T value;
};
}
}
}
#endif // CORE_DBUS_TYPES_VARIANT_H_
