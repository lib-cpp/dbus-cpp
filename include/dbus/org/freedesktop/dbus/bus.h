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

#include "org/freedesktop/dbus/error.h"
#include "org/freedesktop/dbus/executor.h"
#include "org/freedesktop/dbus/message_router.h"
#include "org/freedesktop/dbus/well_known_bus.h"
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
class Bus
{
  public:
    typedef std::shared_ptr<Bus> Ptr;
    typedef MessageRouter<types::ObjectPath> SignalRouter;

    explicit Bus(WellKnownBus bus);
    Bus(const Bus&) = delete;
    ~Bus() noexcept;

    Bus& operator=(const Bus&) = delete;
    bool operator==(const Bus&) const = delete;

    uint32_t send(DBusMessage* msg);
    DBusMessage* send_with_reply_and_block_for_at_most(
        DBusMessage* msg, 
        const std::chrono::milliseconds& milliseconds);
    DBusPendingCall* send_with_reply_and_timeout(
        DBusMessage* msg, 
        const std::chrono::milliseconds& timeout);

    bool install_message_filter(
        DBusHandleMessageFunction filter, 
        void* cookie) noexcept;
    void uninstall_message_filter(
        DBusHandleMessageFunction filter, 
        void* cookie) noexcept;

    void add_match(const std::string& rule);
    void remove_match(const std::string& rule);

    bool has_owner_for_name(const std::string& name);

    void install_executor(const Executor::Ptr& e);
    void stop();
    void run();

    SignalRouter& access_signal_router();

    DBusConnection* raw() const;

  private:
    static DBusHandlerResult handle_message(
        DBusConnection*, 
        DBusMessage* message, 
        void* data);

    std::shared_ptr<DBusConnection> connection;
    Executor::Ptr executor;
    MessageRouter<int> message_type_router;
    SignalRouter signal_router;
};
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_BUS_H_
