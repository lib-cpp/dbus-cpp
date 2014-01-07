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
#ifndef CORE_DBUS_TYPES_ANY_H_
#define CORE_DBUS_TYPES_ANY_H_

#include <core/dbus/message.h>

#include <memory>

namespace core
{
namespace dbus
{
namespace types
{
/**
 * @brief Any describes types for which no codec specialization is known at compile time.
 */
class Any
{
public:
    /**
      * @brief Constructs an empty instance.
      */
    Any(const Message::Reader& reader = Message::Reader())
        : reader_(reader)
    {
    }

    /**
     * @brief Provides non-mutable access to the contained message.
     *
     * Users of the library can rely on Any and access to the contained and cloned
     * message to postpone serialization until runtime. Usually, compile-time static
     * typing is preferred via template-specializations of Codec to ensure maximum
     * stability.
     */
    inline Message::Reader& reader() const
    {
        return reader_;
    }

private:
    mutable Message::Reader reader_;
};
}
}
}

#endif // CORE_DBUS_TYPES_ANY_H_
