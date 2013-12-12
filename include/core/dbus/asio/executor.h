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
#ifndef CORE_DBUS_ASIO_EXECUTOR_H_
#define CORE_DBUS_ASIO_EXECUTOR_H_

#include <core/dbus/bus.h>
#include <core/dbus/executor.h>
#include <core/dbus/visibility.h>

namespace core
{
namespace dbus
{
namespace asio
{
ORG_FREEDESKTOP_DBUS_DLL_PUBLIC Executor::Ptr make_executor(const Bus::Ptr& bus);
}
}
}

#endif // CORE_DBUS_ASIO_EXECUTOR_H_
