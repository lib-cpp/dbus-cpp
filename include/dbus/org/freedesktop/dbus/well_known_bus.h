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
/**
 * @brief The WellKnownBus enum lists all the buses known to the underlying dbus reference implementation.
 */
enum class WellKnownBus
{
    session = DBUS_BUS_SESSION, ///< The session bus
    system = DBUS_BUS_SYSTEM, ///< The system bus
    starter = DBUS_BUS_STARTER ///< The bus that started us
};
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_ORG_DBUS_H_
