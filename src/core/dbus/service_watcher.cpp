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

#include <core/dbus/dbus.h>
#include <core/dbus/match_rule.h>
#include <core/dbus/object.h>
#include <core/dbus/service_watcher.h>
#include <core/dbus/signal.h>

namespace core
{
namespace dbus
{

namespace {
struct NameOwnerChanged
{
    static const std::string& name()
    {
        static const std::string s { "NameOwnerChanged" };
        return s;
    }

    typedef DBus Interface;
    typedef std::tuple<std::string, std::string, std::string> ArgumentType;
};
}

struct dbus::ServiceWatcher::Private
{
    void signal_handler(const NameOwnerChanged::ArgumentType& args)
    {
        const std::string& old_owner(std::get<1>(args));
        const std::string& new_owner(std::get<2>(args));

        owner_changed(old_owner, new_owner);

        if (new_owner.empty())
        {
            service_unregistered();
        }
        else
        {
            service_registered();
        }
    }

    core::Signal<std::string, std::string> owner_changed;
    core::Signal<void> service_registered;
    core::Signal<void> service_unregistered;
    std::shared_ptr<Object> object;
    std::shared_ptr<
            core::dbus::Signal<NameOwnerChanged, NameOwnerChanged::ArgumentType>> signal;
};

dbus::ServiceWatcher::ServiceWatcher(std::shared_ptr<Object> object,
        const std::string& name, DBus::WatchMode watch_mode) :
        d(new Private())
{
    d->object = object;

    MatchRule::MatchArgs match_args{ { 0, name } };

    switch (watch_mode)
    {
    case DBus::WatchMode::owner_change:
        break;
    case DBus::WatchMode::registration:
        match_args.push_back({ 1, std::string() });
        break;
    case DBus::WatchMode::unregistration:
        match_args.push_back({ 2, std::string() });
        break;
    }

    d->signal = d->object->get_signal<NameOwnerChanged>();

    d->signal->connect_with_match_args(
            std::bind(&Private::signal_handler, d, std::placeholders::_1), match_args);
}

const core::Signal<std::string, std::string>& dbus::ServiceWatcher::owner_changed() const
{
    return d->owner_changed;
}

const core::Signal<void>& dbus::ServiceWatcher::service_registered() const
{
    return d->service_registered;
}

const core::Signal<void>& dbus::ServiceWatcher::service_unregistered() const
{
    return d->service_unregistered;
}

}
}
