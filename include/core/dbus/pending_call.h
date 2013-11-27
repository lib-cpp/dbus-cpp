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
#ifndef CORE_DBUS_PENDING_CALL_H_
#define CORE_DBUS_PENDING_CALL_H_

#include <core/dbus/visibility.h>

#include <cstdint>

#include <functional>
#include <limits>
#include <memory>

namespace core
{
namespace dbus
{
class Message;

/**
 * @brief The PendingCall class wraps an active call to a remote peer.
 */
class ORG_FREEDESKTOP_DBUS_DLL_PUBLIC PendingCall
{
public:
    /**
     * @brief Timeout constant for a call that never times out.
     */
    inline static const std::chrono::milliseconds& inifinite_timeout()
    {
        static const std::chrono::milliseconds ms
        {
            std::numeric_limits<std::int32_t>::max()
        };

        return ms;
    }

    typedef std::shared_ptr<PendingCall> Ptr;

    /** @brief Function signature callback for call completion notification. */
    typedef std::function<void(const std::shared_ptr<Message>&)> Notification;

    PendingCall(const PendingCall&) = delete;
    virtual ~PendingCall() = default;

    PendingCall& operator=(const PendingCall&) = delete;
    bool operator==(const PendingCall&) const = delete;

    /**
     * @brief Suspends the current thread until a reply has arrived.
     * @return Pointer to a message.
     */
    virtual std::shared_ptr<Message> wait_for_reply() = 0;

    /**
     * @brief Cancels the outstanding call.
     */
    virtual void cancel() = 0;

    /**
     * @brief Sets up notification as the callback when the call eventually completes.
     * @param notification The function to be called when the call completes.
     */
    virtual void then(const Notification& notification) = 0;

protected:
    PendingCall() = default;
};
}
}

#endif // CORE_DBUS_PENDING_CALL_H_
