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
#ifndef DBUS_ORG_FREEDESKTOP_ORG_DBUS_H_
#define DBUS_ORG_FREEDESKTOP_ORG_DBUS_H_

#include <dbus/dbus.h>

namespace org
{
namespace freedesktop
{
namespace dbus
{
enum class WellKnownBus
{
    session = DBUS_BUS_SESSION,
    system = DBUS_BUS_SYSTEM,
    starter = DBUS_BUS_STARTER
};
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_ORG_DBUS_H_
