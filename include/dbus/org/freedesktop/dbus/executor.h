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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_EXECUTOR_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_EXECUTOR_H_

#include "org/freedesktop/dbus/visibility.h"

#include <memory>

namespace org
{
namespace freedesktop
{
namespace dbus
{
/**
 * @brief Abstracts an event loop that a bus instance should be running upon.
 */
class ORG_FREEDESKTOP_DBUS_DLL_PUBLIC Executor
{
public:
    typedef std::shared_ptr<Executor> Ptr;
    
    virtual ~Executor() = default;

protected:
    friend class Bus;

    Executor() = default;
    Executor(const Executor&) = delete;
    Executor& operator=(const Executor&) = delete;

    /**
     * @brief Start the event loop and block until stop is called.
     */
    virtual void run() = 0;

    /**
     * @brief Stop the event loop.
     */
    virtual void stop() = 0;
};
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_EXECUTOR_H_
