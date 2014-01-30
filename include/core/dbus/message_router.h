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
#ifndef CORE_DBUS_MESSAGE_ROUTER_H_
#define CORE_DBUS_MESSAGE_ROUTER_H_

#include <core/dbus/message.h>

#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace core
{
namespace dbus
{
/**
 * @brief Takes a raw DBus message and routes it to a handler.
 */
template<typename Key>
class MessageRouter
{
public:
    /**
     * @brief Mapper takes a raw DBus Message and maps it to the Key type of the router.
     */
    typedef std::function<Key(const Message::Ptr&)> Mapper;

    /**
     * @brief Handler is a function type that handles raw DBus messages.
     */
    typedef std::function<void(const Message::Ptr&)> Handler;

    /**
     * @brief Constructs an empty router with the specified mapper instance.
     * @param m An object of type Mapper.
     */
    inline explicit MessageRouter(const Mapper& m) : mapper(m)
    {
    }

    MessageRouter(const MessageRouter&) = delete;
    MessageRouter& operator=(const MessageRouter&) = delete;

    /**
     * @brief Installs a route for a specific key in a thread-safe manner, replacing any previously installed route.
     * @param key The key to install the route for.
     * @param handler The handler to install, must not be empty.
     */
    inline void install_route(const Key& key, Handler handler)
    {
        std::unique_lock<std::mutex> ul(guard);
        router[key] = handler;
    }

    /**
     * @brief Uninstalls a route for a specific key in a thread-safe manner.
     * @param key The key to uninstall the route for.
     */
    inline void uninstall_route(const Key& key)
    {
        std::unique_lock<std::mutex> ul(guard);
        router.erase(key);
    }

    /**
     * @brief Maps and routes a raw DBus message in a thread-safe manner.
     * @param msg The message to map and route, must not be null.
     * @return true if the message has been routes successfully, false otherwise.
     */
    inline bool operator()(const Message::Ptr& msg)
    {
        std::unique_lock<std::mutex> ul(guard);
        auto it = router.find(mapper(msg));
        if (it != router.end()) {
            // release the lock so that Handler can modify the Router
            ul.unlock();
            it->second(msg);
            return true;
        } else {
            return false;
        }
    }

private:
    std::mutex guard;
    Mapper mapper;
    std::unordered_map<Key,Handler> router;
};
}
}

#endif // CORE_DBUS_MESSAGE_ROUTER_H_
