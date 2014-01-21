/*
 * Copyright © 2014 Canonical Ltd.
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
 * Authored by: Pete Woods <pete.woods@canonical.com>
 */
#ifndef CORE_DBUS_SERVICE_WATCHER_H_
#define CORE_DBUS_SERVICE_WATCHER_H_

#include <core/signal.h>
#include <core/dbus/dbus.h>
#include <core/dbus/visibility.h>

#include <memory>
#include <string>

namespace core
{
namespace dbus
{
class Object;

/**
 * @brief Wraps a DBus match rule.
 */
class ORG_FREEDESKTOP_DBUS_DLL_PUBLIC ServiceWatcher
{
friend DBus;
public:
    typedef std::shared_ptr<ServiceWatcher> Ptr;

    ServiceWatcher(const ServiceWatcher& rhs) = delete;
    ServiceWatcher& operator=(const ServiceWatcher& rhs) = delete;
    bool operator==(const ServiceWatcher&) const = delete;

    core::Signal<std::string, std::string> owner_changed;

    core::Signal<void> service_registered;

    core::Signal<void> service_unregistered;

private:
    ServiceWatcher(std::shared_ptr<Object> object, const std::string& name,
                DBus::WatchMode watch_mode = DBus::WatchMode::owner_change);

    struct NameOwnerChanged;

    struct Private;
    std::shared_ptr<Private> d;
};
}
}

#endif // CORE_DBUS_SERVICE_WATCHER_H_
