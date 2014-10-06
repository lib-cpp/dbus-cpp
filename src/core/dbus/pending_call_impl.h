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

namespace
{
// Better save than sorry. We wrap common error handling here.
bool is_pending_call_completed(DBusPendingCall* pending_call)
{
    if (not pending_call)
        return false;

    return dbus_pending_call_get_completed(pending_call) == TRUE;
}
}
namespace core
{
namespace dbus
{
namespace impl
{
class PendingCall : public core::dbus::PendingCall
{
private:
    // We explicitly maintain the state of the pending call
    // to avoid races during initialization and callbacks happening.
    enum class State
    {
        // We haven't received a result yet.
        pending,
        // We have received a result and either called out
        // to application code or stored the answer to
        // our request in a message for later processing
        completed
    };

    // We keep an instance alive by passing the pending call
    // wrapped in an ordinary heap-allocated wrapper to the
    // notification callback.
    struct Wrapper
    {
        std::shared_ptr<core::dbus::impl::PendingCall> pending_call;
    };

    // The callback that is passed to libdbus.
    static void on_pending_call_completed(DBusPendingCall* call, void* cookie)
    {
        auto wrapper = static_cast<Wrapper*>(cookie);

        if (not wrapper)
            return;

        // We tie cleanup of the wrapper to the scope of the callback.
        // With that, even an exception being thrown would _not_ result
        // in an instance being leaked.
        struct Scope
        {
            ~Scope()
            {
                // We always unref this message, even if notify_locked or
                // from_raw_message throws.
                if (message) dbus_message_unref(message);
            }

            // We only need this later on, when we have stolen the reply
            // from the pending call.
            DBusMessage* message;
        } scope{nullptr};

        // We synchronize to avoid races on construction.
        std::lock_guard<std::mutex> lg{wrapper->pending_call->guard};
        // And we only steal the reply if the call actually completed.
        if (is_pending_call_completed(call))
            if (nullptr != (scope.message = dbus_pending_call_steal_reply(call)))
                wrapper->pending_call->notify_locked(Message::from_raw_message(scope.message));
    }

    // Announce incoming reply and invoke the callback if set.
    // Atomically maintains the state of this call instance.
    void notify_locked(const Message::Ptr& msg)
    {
        if (state.exchange(State::completed) == State::completed)
            return;

        message = msg;

        if (callback)
            callback(message);
    }

public:
    // Creates a new PendingCall instance given the opaque call instance
    // handed out by libdbus. Throws in case of errors.
    inline static core::dbus::PendingCall::Ptr create(DBusPendingCall* call)
    {
        auto result = std::shared_ptr<core::dbus::impl::PendingCall>
        {
            new core::dbus::impl::PendingCall{call}
        };

        // Our scope contains two objects that are dynamically created:
        //   * The actual PendingCall implementation (managed by a shared pointer)
        //   * A helper object of type Wrapper for passing around the pending call instance.
        // The latter one needs some manual memory mgmt. until we are sure that it got handed
        // over to libdbus correctly.
        struct Scope
        {
            // Whenever we go out of scope, we unref the call (we do not need it anymore)
            // and we potentially (if armed) delete the wrapper.
            ~Scope()
            {
                dbus_pending_call_unref(call);
                if (message) dbus_message_unref(message);
            }  

            // The raw call instance.
            DBusPendingCall* call;
            // Non-null if we managed to steal the reply.
            DBusMessage* message;
        } scope
        {
            // The raw call that we got handed from the caller.
            call,
            // Initialize the message to null to make sure that we
            // do not unref garbage.
            nullptr
        };

        // We synchronize to avoid races on construction.
        std::lock_guard<std::mutex> lg{result->guard};

        // We dispatch to the static on_pending_call_completed when
        // the call completes.
        // Please refer to the source-code comments in on_pending_call_completed.
        if (FALSE == dbus_pending_call_set_notify(
                result->pending_call, PendingCall::on_pending_call_completed,
                new Wrapper{result}, [](void* p){ delete static_cast<Wrapper*>(p); }))
        {
            throw std::runtime_error("Error setting up pending call notification.");
        }

        // And here comes the beauty of libdbus, and its racy architecture:
        if (is_pending_call_completed(call))
        {
            // We took too long while setting up the pending call notification.
            // For that we now have to inject the message here.
            if (nullptr != (scope.message = dbus_pending_call_steal_reply(call)))
                result->notify_locked(Message::from_raw_message(scope.message));
        }

        return result;
    }

    // Cancels the outstanding call.
    void cancel() override
    {
        dbus_pending_call_cancel(pending_call);
    }

    // Installs a continuation and invokes it if the call already completed.
    void then(const core::dbus::PendingCall::Notification& notification) override
    {
        std::lock_guard<std::mutex> lg(guard);
        callback = notification;

        // We already have a reply and invoke the callback directly.
        if (message)
            callback(message);
    }

private:
    PendingCall(DBusPendingCall* call)
        : state(State::pending), pending_call(call)
    {
        if (not call) throw std::runtime_error
        {
            "core::dbus::PendingCall cannot be constructed for null object."
        };
    }

    // Our internal state, initialized to State::pending.
    std::atomic<State> state;
    // Our pending call instance.
    DBusPendingCall* pending_call;
    // We synchronize access to the following two members.
    std::mutex guard;
    // The reply message.
    Message::Ptr message;
    // The callback, invoked when either the call completed or
    // if the callback is installed when the call already completed.
    PendingCall::Notification callback;
};
}
}
}

#endif // CORE_DBUS_PENDING_CALL_IMPL_H_
