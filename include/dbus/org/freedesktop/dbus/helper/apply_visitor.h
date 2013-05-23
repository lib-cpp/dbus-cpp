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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_HELPER_APPLY_VISITOR_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_HELPER_APPLY_VISITOR_H_

#include "org/freedesktop/dbus/argument_type.h"
#include "org/freedesktop/dbus/helper/is_compound_type.h"

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace helper
{
template<typename Visitor>
inline void apply_visitor(DBusMessageIter* it, Visitor visitor)
{
    ArgumentType type {ArgumentType::invalid};

    while ((type = static_cast<ArgumentType>(dbus_message_iter_get_arg_type(it))) != ArgumentType::invalid)
    {
        if (is_compound_type(type))
        {
            visitor(type);
            DBusMessageIter itt;
            dbus_message_iter_recurse(it, std::addressof(itt));

            apply_visitor(std::addressof(itt), visitor);
        }
        else
        {
            DBusBasicValue value;
            dbus_message_iter_get_basic(it, std::addressof(value));
            visitor(type, std::addressof(value));
        }

        dbus_message_iter_next(it);
    }
}
}
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_HELPER_APPLY_VISITOR_H_
