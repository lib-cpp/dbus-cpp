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

#include "org/freedesktop/dbus/codec.h"
#include "org/freedesktop/dbus/helper/type_mapper.h"

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
    friend struct Codec<types::UnixFd>;

    int fd;
};
}
namespace helper
{
template<>
struct TypeMapper<org::freedesktop::dbus::types::UnixFd>
{
    constexpr inline static ArgumentType type_value()
    {
        return ArgumentType::unix_fd;
    }
    constexpr inline static bool is_basic_type()
    {
        return false;
    }
    constexpr inline static bool requires_signature()
    {
        return true;
    }

    inline static std::string signature()
    {
        return DBUS_TYPE_UNIX_FD_AS_STRING;
    }
};
}
template<>
struct Codec<types::UnixFd>
{
    inline static void encode_argument(DBusMessageIter* out, const types::UnixFd& arg)
    {
        dbus_message_iter_append_basic(out, static_cast<int>(ArgumentType::unix_fd), std::addressof(arg.fd));
    }

    inline static void decode_argument(DBusMessageIter* in, types::UnixFd& arg)
    {
        if (dbus_message_iter_get_arg_type(in) != static_cast<int>(ArgumentType::unix_fd))
            throw std::runtime_error("Incompatible argument type: dbus_message_iter_get_arg_type(in) != ArgumentType::unix_fd");

        dbus_message_iter_get_basic(in, std::addressof(arg.fd));
    }
};
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_TYPES_OBJECT_PATH_H_
