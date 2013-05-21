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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_HELPER_IS_COMPOUND_TYPE_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_HELPER_IS_COMPOUND_TYPE_H_

#include "org/freedesktop/dbus/argument_type.h"

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace helper
{
bool is_compound_type(ArgumentType type)
{
    switch (type)
    {
    case ArgumentType::array:
    case ArgumentType::structure:
    case ArgumentType::dictionary_entry:
        return true;
        break;
    default:
        break;
    }

    return false;
}
}
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_HELPER_IS_COMPOUND_TYPE_H_
