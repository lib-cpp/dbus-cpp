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
    struct Wrapper
    {
        std::shared_ptr<core::dbus::impl::PendingCall> pending_call;
    };

    static void on_pending_call_completed(DBusPendingCall* call,
                                          void* cookie)
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
                // We take over ownership of the wrapper and destroy on scope exit.
                delete wrapper;
                // We always unref this message, even if notify_locked or
                // from_raw_message throws.
                if (message) dbus_message_unref(message);
            }

            Wrapper* wrapper;
            // We only need this later on, when we have stolen the reply
            // from the pending call.
            DBusMessage* message;
        } scope{wrapper, nullptr};

        // We synchronize to avoid races on construction.
        std::lock_guard<std::mutex> lg{wrapper->pending_call->guard};
        // And we only steal the reply if the call actually completed.
        if (not is_pending_call_completed(call))
            return;

        scope.message = dbus_pending_call_steal_reply(call);

        if (scope.message)
            wrapper->pending_call->notify_locked(Message::from_raw_message(scope.message));
    }

    // Announce incoming reply and invoke the callback if set.
    void notify_locked(const Message::Ptr& msg)
    {
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
        if (not call) throw std::runtime_error
        {
            "core::dbus::PendingCall cannot be constructed for null object."
        };

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
                if (armed) delete wrapper;
            }

            // Disarms the scope for handling the wrapper instance.
            // Think about it like giving up ownership as we handed over
            // to libdbus.
            void disarm_for_wrapper()
            {
                armed = false;
            }

            // The raw call instance.
            DBusPendingCall* call;
            // True if the wrapper instance is still owned by the scope.
            bool armed;
            // The wrapper instance.
            Wrapper* wrapper;
        } scope
        {
            // The raw call that we got handed from the caller.
            call,
            // Yes, we still own the Wrapper instance.
            true, new Wrapper{result}
        };

        // We synchronize to avoid races on construction.
        std::lock_guard<std::mutex> lg{result->guard};

        if (FALSE == dbus_pending_call_set_notify(
                // We dispatch to the static on_pending_call_completed when
                // the call completes.
                result->pending_call, PendingCall::on_pending_call_completed,
                // We pass in our helper object and do not specify a deleter.
                // Please refer to the source-code comments in on_pending_call_completed.
                scope.wrapper, nullptr))
        {
            throw std::runtime_error("Error setting up pending call notification.");
        }

        // And here comes the beauty of libdbus, and its racy architecture:
        {            
            if (is_pending_call_completed(call))
            {
                // We took too long while setting up the pending call notification.
                // For that we now have to inject the message here.
                auto msg = dbus_pending_call_steal_reply(call);

                if (msg)
                {
                    result->message = Message::from_raw_message(msg);
                    // We decrease the reference count as Message::from_raw_message
                    // always refs the object that it is passed.
                    dbus_message_unref(msg);
                }
            }
            else
            {
                // We need the wrapper to be alive, so we disarm the scope.
                // Please note that this is the only "good" path through this
                // mess of setup and notification functions.
                scope.disarm_for_wrapper();
            }
        }

        return result;
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
