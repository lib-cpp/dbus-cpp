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

#include "org/freedesktop/dbus/error.h"

#include <dbus/dbus.h>

#include <iostream>

namespace org
{
namespace freedesktop
{
namespace dbus
{

struct Error::Private
{
    Private()
    {
        dbus_error_init(std::addressof(error));
    }

    ~Private()
    {
        dbus_error_free(std::addressof(error));
    }

    DBusError error;
};

Error::Error() : d(new Private())
{
}

Error::Error(Error&& that) : d(std::move(that.d))
{
}

Error::~Error()
{
}

Error& Error::operator=(Error&& rhs)
{
    d = std::move(rhs.d);
    return *this;
}

std::string Error::name() const
{
    return d->error.name;
}

std::string Error::message() const
{
    return d->error.message;
}

std::string Error::print() const
{
    return name() + ": " + message();
}

Error::operator bool() const
{
    return dbus_error_is_set(std::addressof(d->error));
}

DBusError& Error::raw()
{
    return d->error;
}
}
}
}

