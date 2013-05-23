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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_BUS_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_BUS_H_

#include "org/freedesktop/dbus/executor.h"
#include "org/freedesktop/dbus/message_router.h"
#include "org/freedesktop/dbus/well_known_bus.h"
#include "org/freedesktop/dbus/traits/timeout.h"
#include "org/freedesktop/dbus/traits/watch.h"
#include "org/freedesktop/dbus/types/object_path.h"

#include <cstring>

#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <map>
#include <mutex>
#include <stdexcept>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>

#include <dbus/dbus.h>

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace traits
{
template<>
struct Timeout<DBusTimeout>
{
    typedef int DurationType;

    static inline bool is_timeout_enabled(DBusTimeout* timeout)
    {
        return dbus_timeout_get_enabled(timeout);
    }

    static inline int get_timeout_interval(DBusTimeout* timeout)
    {
        return DurationType(dbus_timeout_get_interval(timeout));
    }

    static inline void invoke_timeout_handler(DBusTimeout* timeout)
    {
        dbus_timeout_handle(timeout);
    }
};

template<>
struct Watch<DBusWatch>
{
    inline static int readable_event() { return DBUS_WATCH_READABLE; }
    inline static int writeable_event() { return DBUS_WATCH_WRITABLE; }
    inline static int error_event() { return DBUS_WATCH_ERROR; }
    inline static int hangup_event() { return DBUS_WATCH_HANGUP; }

    static inline bool is_watch_enabled(DBusWatch* watch)
    {
        return dbus_watch_get_enabled(watch);
    }

    static inline int get_watch_unix_fd(DBusWatch* watch)
    {
        return dbus_watch_get_unix_fd(watch);
    }

    static inline bool is_watch_monitoring_fd_for_readable(DBusWatch* watch)
    {
        return dbus_watch_get_flags(watch) & DBUS_WATCH_READABLE;
    }

    static bool is_watch_monitoring_fd_for_writable(DBusWatch* watch)
    {
        return dbus_watch_get_flags(watch) & DBUS_WATCH_WRITABLE;
    }

    static bool invoke_watch_handler_for_event(DBusWatch* watch, int event)
    {
        return dbus_watch_handle(watch, event);
    }
};
}

class Bus
{
  public:
    typedef std::shared_ptr<Bus> Ptr;

    struct ScopedError
    {
        ScopedError()
        {
            dbus_error_init(std::addressof(error));
        }
        
        ~ScopedError()
        {
            dbus_error_free(std::addressof(error));
        }
        
        ScopedError(const ScopedError&) = delete;
        ScopedError& operator=(const ScopedError&) = delete;

        DBusError error;
    };

    static DBusHandlerResult handle_message(DBusConnection*, DBusMessage* message, void* data)
    {
        auto thiz = static_cast<Bus*>(data);
        thiz->message_type_router(message);

        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    typedef MessageRouter<types::ObjectPath> SignalRouter;

    explicit Bus(WellKnownBus bus)
            : connection(nullptr),
              message_type_router([](DBusMessage* msg) { return dbus_message_get_type(msg); }),
              signal_router([](DBusMessage* msg){ return types::ObjectPath {dbus_message_get_path(msg)}; })
    {
        static std::once_flag once;
        std::call_once(once, []() { dbus_threads_init_default(); std::atexit(dbus_shutdown); });
        
        ScopedError se;

        connection.reset(
            dbus_bus_get_private(static_cast<DBusBusType>(bus), std::addressof(se.error)),
            [](DBusConnection*){}
        );

        if (!connection)
            throw std::runtime_error(se.error.message);

        message_type_router.install_route(DBUS_MESSAGE_TYPE_SIGNAL, std::bind(&SignalRouter::operator(), std::ref(signal_router), std::placeholders::_1));
        install_message_filter(handle_message, this);
    }

    ~Bus() noexcept
    {
        uninstall_message_filter(handle_message, this);
        dbus_connection_close(connection.get());
        dbus_connection_unref(connection.get());        
    }

    uint32_t send(DBusMessage* msg)
    {
        dbus_uint32_t serial;
        if (!dbus_connection_send(connection.get(), msg, std::addressof(serial)))
            throw std::runtime_error("Problem sending message");

        return serial;
    }

    DBusMessage* send_with_reply_and_block_for_at_most(DBusMessage* msg, const std::chrono::milliseconds& milliseconds)
    {
        ScopedError se;

        auto result = dbus_connection_send_with_reply_and_block(connection.get(), msg, milliseconds.count(), std::addressof(se.error));

        if (!result)
            throw std::runtime_error(se.error.message);

        return result;
    }

    DBusPendingCall* send_with_reply_and_timeout(DBusMessage* msg, const std::chrono::milliseconds& timeout)
    {
        DBusPendingCall* pending_call;
        auto result = dbus_connection_send_with_reply(connection.get(), msg, std::addressof(pending_call), timeout.count());

        if (!result)
            throw std::runtime_error("Could not send message, not enough memory");

        if (!pending_call)
            throw std::runtime_error("Connection disconnected or tried to send fd's over a transport that does not support it");

        return pending_call;
    }

    bool install_message_filter(DBusHandleMessageFunction filter, void* cookie) noexcept
    {
        return dbus_connection_add_filter(connection.get(), filter, cookie, nullptr);
    }

    void uninstall_message_filter(DBusHandleMessageFunction filter, void* cookie) noexcept
    {
        dbus_connection_remove_filter(connection.get(), filter, cookie);
    }

    void add_match(const std::string& rule)
    {
        ScopedError se;
        dbus_bus_add_match(connection.get(), rule.c_str(), std::addressof(se.error));
        if (dbus_error_is_set(std::addressof(se.error)))
            throw std::runtime_error(se.error.message);
    }

    void remove_match(const std::string& rule)
    {
        ScopedError se;
        dbus_bus_remove_match(connection.get(), rule.c_str(), std::addressof(se.error));
        if (dbus_error_is_set(std::addressof(se.error)))
            throw std::runtime_error(se.error.message);
    }

    bool has_owner_for_name(const std::string& name)
    {
        return dbus_bus_name_has_owner(connection.get(), name.c_str(), nullptr);
    }

    void install_executor(const Executor::Ptr& e)
    {
        executor = e;
    }

    void stop()
    {
        if (!executor)
            throw std::runtime_error("Missing executor, cannot stop.");
        executor->stop();
    }

    void run()
    {
        if (!executor)
            throw std::runtime_error("Missing executor, cannot run.");
        executor->run();
    }

    SignalRouter& access_signal_router()
    {
        return signal_router;
    }

    DBusConnection* raw() const
    {
        return connection.get();
    }
    
  private:
    std::shared_ptr<DBusConnection> connection;
    Executor::Ptr executor;
    MessageRouter<int> message_type_router;
    SignalRouter signal_router;
};
}
}
}
#endif // DBUS_ORG_FREEDESKTOP_DBUS_BUS_H_
