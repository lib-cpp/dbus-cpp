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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_HELPER_SIGNATURE_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_HELPER_SIGNATURE_H_

#include "org/freedesktop/dbus/helper/type_mapper.h"

#include <string>

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace helper
{
template<typename T>
std::string atomic_signature()
{
    return TypeMapper<T>::signature();
}

std::string signature()
{
    static const std::string s;
    return s;
}

template<typename Arg, typename... Args>
std::string signature(const Arg&, const Args& ... remainder)
{
    static const std::string s = atomic_signature<Arg>() + signature(remainder...);
    return s;
}
}
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_HELPER_SIGNATURE_H_
