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

#include <org/freedesktop/dbus/bus.h>

#include <org/freedesktop/dbus/match_rule.h>

#include <org/freedesktop/dbus/traits/timeout.h>
#include <org/freedesktop/dbus/traits/watch.h>

namespace
{
DBusHandlerResult static_handle_message(
        DBusConnection* connection,
        DBusMessage* message,
        void* user_data)
{
    (void) connection;
    auto thiz =
            static_cast<org::freedesktop::dbus::Bus*>(user_data);
    return static_cast<DBusHandlerResult>(
                thiz->handle_message(
                    org::freedesktop::dbus::Message::from_raw_message(
                        message)));
}
}

namespace org
{
namespace freedesktop
{
namespace dbus
{
Bus::MessageHandlerResult Bus::handle_message(const Message::Ptr& message)
{
    message_type_router(message);
    return Bus::MessageHandlerResult::not_yet_handled;
}

Bus::Bus(WellKnownBus bus)
    : connection(nullptr),
      message_type_router([](const Message::Ptr& msg) { return msg->type(); }),
      signal_router([](const Message::Ptr& msg){ return msg->path(); })
{
    static std::once_flag once;
    std::call_once(once, []() { dbus_threads_init_default(); std::atexit(dbus_shutdown); });

    Error se;

    connection.reset(
                dbus_bus_get_private(static_cast<DBusBusType>(bus), std::addressof(se.raw())),
                [](DBusConnection*){}
    );

    if (!connection)
        throw std::runtime_error(se.print());

    message_type_router.install_route(
                Message::Type::signal,
                std::bind(
                    &Bus::SignalRouter::operator(),
                    std::ref(signal_router),
                    std::placeholders::_1));

    dbus_connection_add_filter(
                    connection.get(),
                    static_handle_message,
                    this,
                    nullptr);
}

Bus::~Bus() noexcept
{
    dbus_connection_remove_filter(connection.get(), static_handle_message, this);
    dbus_connection_close(connection.get());
    dbus_connection_unref(connection.get());
}

uint32_t Bus::send(const std::shared_ptr<Message>& msg)
{
    dbus_uint32_t serial;
    if (!dbus_connection_send(connection.get(), msg->get(), std::addressof(serial)))
        throw std::runtime_error("Problem sending message");

    return serial;
}

std::shared_ptr<Message> Bus::send_with_reply_and_block_for_at_most(
        const std::shared_ptr<Message>& msg,
        const std::chrono::milliseconds& milliseconds)
{
    Error se;

    auto result = dbus_connection_send_with_reply_and_block(
                connection.get(),
                msg->get(),
                milliseconds.count(),
                std::addressof(se.raw()));

    if (!result)
        throw std::runtime_error(se.print());

    return Message::from_raw_message(result);
}

DBusPendingCall* Bus::send_with_reply_and_timeout(
        const std::shared_ptr<Message>& msg,
        const std::chrono::milliseconds& timeout)
{
    DBusPendingCall* pending_call;
    auto result = dbus_connection_send_with_reply(
                connection.get(),
                msg->get(),
                std::addressof(pending_call),
                timeout.count());

    if (!result)
        throw std::runtime_error("Could not send message, not enough memory");

    if (!pending_call)
        throw std::runtime_error("Connection disconnected or tried to send fd's over a transport that does not support it");

    return pending_call;
}

void Bus::add_match(const MatchRule& rule)
{
    Error se;
    dbus_bus_add_match(connection.get(), rule.as_string().c_str(), std::addressof(se.raw()));
    if (se)
        throw std::runtime_error(se.print());
}

void Bus::remove_match(const MatchRule& rule)
{
    Error se;
    dbus_bus_remove_match(connection.get(), rule.as_string().c_str(), std::addressof(se.raw()));
    if (se)
        throw std::runtime_error(se.print());
}

bool Bus::has_owner_for_name(const std::string& name)
{
    return dbus_bus_name_has_owner(connection.get(), name.c_str(), nullptr);
}

void Bus::install_executor(const Executor::Ptr& e)
{
    executor = e;
}

void Bus::stop()
{
    if (!executor)
        throw std::runtime_error("Missing executor, cannot stop.");
    executor->stop();
}

void Bus::run()
{
    if (!executor)
        throw std::runtime_error("Missing executor, cannot run.");
    executor->run();
}

Bus::SignalRouter& Bus::access_signal_router()
{
    return signal_router;
}

DBusConnection* Bus::raw() const
{
    return connection.get();
}
}
}
}
