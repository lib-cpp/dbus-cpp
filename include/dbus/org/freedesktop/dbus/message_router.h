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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_MESSAGE_ROUTER_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_MESSAGE_ROUTER_H_

#include <dbus/dbus.h>

#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace org
{
namespace freedesktop
{
namespace dbus
{
template<typename Key>
class MessageRouter
{
public:
    typedef std::function<Key(DBusMessage*)> Mapper;
    typedef std::function<void(DBusMessage*)> Handler;

    explicit MessageRouter(const Mapper& m) : mapper(m)
    {
    }

    MessageRouter(const MessageRouter&) = delete;
    MessageRouter& operator=(const MessageRouter&) = delete;

    void install_route(const Key& key, Handler handler)
    {
        std::unique_lock<std::mutex> ul(guard);
        router[key] = handler;
    }

    void uninstall_route(const Key& key)
    {
        std::unique_lock<std::mutex> ul(guard);
        router.erase(key);
    }

    bool operator()(DBusMessage* msg)
    {
        std::unique_lock<std::mutex> ul(guard);
        auto it = router.find(mapper(msg));
        if (it != router.end())
            it->second(msg);

        return it != router.end();
    }

private:
    std::mutex guard;
    Mapper mapper;
    std::unordered_map<Key,Handler> router;
};
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_MESSAGE_ROUTER_H_
