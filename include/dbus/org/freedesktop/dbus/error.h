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
#ifndef CORE_DBUS_ERROR_H_
#define CORE_DBUS_ERROR_H_

#include <org/freedesktop/dbus/visibility.h>

#include <memory>
#include <string>

struct DBusError;

namespace core
{
namespace dbus
{
/**
 * @brief Wraps a raw error instance as reported by the underlying DBus implementation.
 */
class ORG_FREEDESKTOP_DBUS_DLL_PUBLIC Error
{
  public:
    /**
     * @brief Constructs a valid but empty Error instance.
     * @post operator bool() returns false.
     */
    Error();
    Error(const Error&) = delete;
    Error(Error&&);
    ~Error();

    Error& operator=(const Error&) = delete;
    Error& operator=(Error&&);

    /**
     * @brief Queries the name of the error. Non-empty if operator bool() returns true.
     */
    std::string name() const;

    /**
     * @brief Queries the human readable description of the error. Non-empty if operator bool() returns true.
     */
    std::string message() const;

    /**
     * @brief Pretty prints name and message for consumption by humans.
     */
    std::string print() const;

    /**
     * @brief Checks if the error bit is set.
     */
    operator bool() const;

    /**
     * @brief Provides mutable access to the underlying error as defined by the underlying DBus implementation.
     */
    DBusError& raw();

  private:
    struct ORG_FREEDESKTOP_DBUS_DLL_LOCAL Private;
    std::unique_ptr<Private> d;
};
}
}

#endif // CORE_DBUS_ERROR_H_
