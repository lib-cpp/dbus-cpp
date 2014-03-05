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
#ifndef CORE_DBUS_PENDING_CALL_IMPL_H_
#define CORE_DBUS_PENDING_CALL_IMPL_H_

#include <core/dbus/pending_call.h>

#include <core/dbus/message.h>

#include <dbus/dbus.h>

#include <mutex>

namespace core
{
namespace dbus
{
namespace impl
{
class PendingCall : public core::dbus::PendingCall
{
private:
    struct Wrapper
    {
        std::shared_ptr<core::dbus::impl::PendingCall> pending_call;
    };

    static void on_pending_call_completed(DBusPendingCall* call,
                                          void* cookie)
    {
        auto wrapper = static_cast<Wrapper*>(cookie);

        auto message = dbus_pending_call_steal_reply(call);

        if (message)
        {
            wrapper->pending_call->notify(Message::from_raw_message(message));
        }
    }

    void notify(const Message::Ptr& msg)
    {
        std::lock_guard<std::mutex> lg(guard);

        message = msg;

        if (callback)
            callback(message);
    }

    DBusPendingCall* pending_call;
    std::mutex guard;
    Message::Ptr message;
    PendingCall::Notification callback;

public:
    inline static core::dbus::PendingCall::Ptr create(DBusPendingCall* call)
    {
        auto result = std::shared_ptr<core::dbus::impl::PendingCall>
        {
            new core::dbus::impl::PendingCall{call}
        };

        if (FALSE == dbus_pending_call_set_notify(
                result->pending_call,
                PendingCall::on_pending_call_completed,
                new Wrapper{result},
                [](void* data)
                {
                    delete static_cast<Wrapper*>(data);
                }))
        {
            throw std::runtime_error("Error setting up pending call notification.");
        }

        // And here comes the beauty of libdbus, and its racy architecture:
        {
            std::lock_guard<std::mutex> lg(result->guard);
            if (TRUE == dbus_pending_call_get_completed(call))
            {
                // We took too long while setting up the pending call notification.
                // For that we now have to inject the message here.
                auto msg = dbus_pending_call_steal_reply(call);
                result->message = Message::from_raw_message(msg);
            }
        }

        return std::dynamic_pointer_cast<core::dbus::PendingCall>(result);
    }

    void cancel()
    {
        dbus_pending_call_cancel(pending_call);
    }

    void then(const core::dbus::PendingCall::Notification& notification)
    {
        std::lock_guard<std::mutex> lg(guard);
        callback = notification;

        // We already have a reply and invoke the callback directly.
        if (message)
            callback(message);
    }

private:
    PendingCall(DBusPendingCall* call)
        : pending_call(call)
    {
    }
};
}
}
}

#endif // CORE_DBUS_PENDING_CALL_IMPL_H_
