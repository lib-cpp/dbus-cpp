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
#include <org/freedesktop/dbus/object.h>

#include <org/freedesktop/dbus/traits/timeout.h>
#include <org/freedesktop/dbus/traits/watch.h>

#include "message_factory_impl.h"
#include "pending_call_impl.h"

namespace
{
struct VTable
{
    static void unregister_object_path(DBusConnection*, void* data)
    {
        delete static_cast<VTable*>(data);
    }

    static DBusHandlerResult on_new_message(
            DBusConnection*,
            DBusMessage* message,
            void* data)
    {
        auto thiz = static_cast<VTable*>(data);

        if (thiz->object->on_new_message(
                    org::freedesktop::dbus::Message::from_raw_message(
                        message)))
            return DBUS_HANDLER_RESULT_HANDLED;

        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    std::shared_ptr<org::freedesktop::dbus::Object> object;
};

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
Bus::Name::Name(Bus::Name&& rhs) : name(std::move(rhs.name))
{
}

Bus::Name& Bus::Name::operator=(Bus::Name&& rhs)
{
    name = std::move(rhs.name);
    return *this;
}

Bus::Name::Name(const std::string &name) : name(name)
{
}

const std::string& Bus::Name::as_string() const
{
    return name;
}

Bus::MessageHandlerResult Bus::handle_message(const Message::Ptr& message)
{
    message_type_router(message);
    return Bus::MessageHandlerResult::not_yet_handled;
}

Bus::Bus(WellKnownBus bus)
    : connection(nullptr),
      message_factory_impl(new impl::MessageFactory()),
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

const std::shared_ptr<MessageFactory> Bus::message_factory()
{

}

const std::shared_ptr<MessageFactory> Bus::message_factory()
{
    return message_factory_impl;
}

Bus::Name&& Bus::request_name_on_bus(
        const std::string& name,
        Bus::RequestNameFlag flags)
{
    Error error;
    auto rc = dbus_bus_request_name(
                connection.get(),
                name.c_str(),
                static_cast<unsigned int>(flags),
                std::addressof(error.raw()));

    Bus::Name result{name};

    switch (rc)
    {
    case DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER: return std::move(result);
    case DBUS_REQUEST_NAME_REPLY_IN_QUEUE: return std::move(result);
    case DBUS_REQUEST_NAME_REPLY_EXISTS: throw Bus::Errors::AlreadyOwned{}; break;
    case DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER: throw Bus::Errors::AlreadyOwner{}; break;
    case -1: throw std::runtime_error(error.print());
    }

    return std::move(result);
}

void Bus::release_name_on_bus(Bus::Name&& name)
{
    Error error;
    dbus_bus_release_name(
                connection.get(),
                name.as_string().c_str(),
                std::addressof(error.raw()));
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

PendingCall::Ptr Bus::send_with_reply_and_timeout(
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
        throw Errors::NoMemory{};

    if (!pending_call)
        throw std::runtime_error("Connection disconnected or tried to send fd's over a transport that does not support it");

    return impl::PendingCall::create(pending_call);
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

void Bus::register_object_for_path(
        const types::ObjectPath& path,
        const std::shared_ptr<Object>& object)
{
    auto vtable = new DBusObjectPathVTable
    {
        VTable::unregister_object_path,
        VTable::on_new_message,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    };

    Error e;
    auto result = dbus_connection_try_register_object_path(
                      connection.get(),
                      path.as_string().c_str(),
                      vtable,
                      new VTable{object},
                      std::addressof(e.raw()));

    if (!result || e)
    {
        delete vtable;
        throw std::runtime_error(e.print());
    }
}

void Bus::unregister_object_path(
        const types::ObjectPath& path)
{
    dbus_connection_unregister_object_path(
                connection.get(),
                path.as_string().c_str());
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
