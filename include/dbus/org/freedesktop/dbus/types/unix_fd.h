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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_TYPES_UNIX_FD_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_TYPES_UNIX_FD_H_

#include <stdexcept>
#include <string>

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace types
{
class UnixFd
{
public:
    explicit UnixFd(int fd = -1) : fd(fd)
    {
    }

    const int& to_int() const
    {
        return fd;
    }

    void from_int(int fd)
    {
        this->fd = fd;
    }

    bool operator==(const UnixFd& rhs) const
    {
        return fd == rhs.fd;
    }

private:
    int fd;
};
}
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_TYPES_OBJECT_PATH_H_
