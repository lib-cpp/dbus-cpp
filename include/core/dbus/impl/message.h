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

#ifndef CORE_DBUS_IMPL_MESSAGE_H_
#define CORE_DBUS_IMPL_MESSAGE_H_

namespace std
{
template<>
struct hash<core::dbus::Message::Type>
{
    size_t operator()(const core::dbus::Message::Type& type) const
    {
        static const hash<int> h {};
        return h(static_cast<int>(type));
    }
};
}

#endif // CORE_DBUS_IMPL_MESSAGE_H_
