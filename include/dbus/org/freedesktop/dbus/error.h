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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_ERROR_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_ERROR_H_

#include "org/freedesktop/dbus/visibility.h"

#include <memory>
#include <string>

struct DBusError;

namespace org
{
namespace freedesktop
{
namespace dbus
{
class ORG_FREEDESKTOP_DBUS_DLL_PUBLIC Error
{
  public:
    Error();
    ~Error();

    Error(const Error&) = delete;
    Error& operator=(const Error&) = delete;

    std::string name() const;
    std::string message() const;

    operator bool() const;

    DBusError& raw();

  private:
    struct ORG_FREEDESKTOP_DBUS_DLL_LOCAL Private;
    std::unique_ptr<Private> d;
};
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_ERROR_H_
