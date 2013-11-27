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

#include <org/freedesktop/dbus/pending_call.h>

#include <org/freedesktop/dbus/message.h>

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
        std::shared_ptr<PendingCall> pending_call;
    };

    static void on_pending_call_completed(DBusPendingCall* call,
                                          void* cookie)
    {
        auto wrapper = static_cast<Wrapper*>(cookie);
        std::lock_guard<std::mutex> lg(wrapper->pending_call->callback_guard);
        if (wrapper->pending_call->callback)
        {
            auto message = dbus_pending_call_steal_reply(call);
            if (message)
                wrapper->pending_call->callback(Message::from_raw_message(message));
        }
    }

    DBusPendingCall* pending_call;
    std::mutex callback_guard;
    core::dbus::PendingCall::Notification callback;

public:
    inline static core::dbus::PendingCall::Ptr create(DBusPendingCall* call)
    {
        auto result = std::shared_ptr<core::dbus::impl::PendingCall>(
                    new core::dbus::impl::PendingCall(call));

        dbus_pending_call_set_notify(
                    result->pending_call,
                    PendingCall::on_pending_call_completed,
                    new Wrapper{result},
                    [](void* data) { delete static_cast<Wrapper*>(data); });

        return std::dynamic_pointer_cast<core::dbus::PendingCall>(result);
    }

    inline std::shared_ptr<Message> wait_for_reply()
    {
        dbus_pending_call_block(pending_call);
        auto reply = dbus_pending_call_steal_reply(pending_call);

        if (!reply)
        {
            // TODO(tvoss): Throw PendingCallYieldedEmptyReply.
        }

        return Message::from_raw_message(reply);
    }

    void cancel()
    {
        dbus_pending_call_cancel(pending_call);
    }

    void then(const core::dbus::PendingCall::Notification& notification)
    {
        std::lock_guard<std::mutex> lg{callback_guard};
        callback = notification;
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
