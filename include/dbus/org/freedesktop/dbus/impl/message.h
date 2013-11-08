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

#ifndef DBUS_ORG_FREEDESKTOP_DBUS_IMPL_MESSAGE_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_IMPL_MESSAGE_H_

namespace std
{
template<>
struct hash<org::freedesktop::dbus::Message::Type>
{
    size_t operator()(const org::freedesktop::dbus::Message::Type& type) const
    {
        static const hash<int> h {};
        return h(static_cast<int>(type));
    }
};
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_IMPL_MESSAGE_H_
