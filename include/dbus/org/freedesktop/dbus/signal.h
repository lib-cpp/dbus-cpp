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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_SIGNAL_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_SIGNAL_H_

#include <org/freedesktop/dbus/message.h>
#include <org/freedesktop/dbus/visibility.h>

#include <functional>
#include <memory>
#include <string>

namespace org
{
namespace freedesktop
{
namespace dbus
{
template<typename T>
struct is_not_void
{
    static const bool value = true;
};

template<>
struct is_not_void<void>
{
    static const bool value = false;
};

/**
 * @brief Template class Signal models a type-safe DBus signal.
 * @tparam SignalDescription Needs to be a model of concept SignalDescription.
 * @tparam Argument The type of the argument that is emitted by this signal.
 */
template<typename SignalDescription, typename Argument = void>
class Signal
{
public:
    typedef std::shared_ptr<Signal<SignalDescription, void>> Ptr;
    typedef std::function<void()> Handler;
    typedef typename std::list<Handler>::iterator SubscriptionToken;

    inline ~Signal() noexcept;

    inline void emit(void);

    inline SubscriptionToken connect(const Handler& h);
    inline void disconnect(const SubscriptionToken& token);
protected:
    friend class Object;

    inline static std::shared_ptr<Signal<SignalDescription, void>> make_signal(
                                                                       const std::shared_ptr<Object>& parent,
                                                                       const std::string& interface,
                                                                       const std::string& name);

private:
    inline Signal(const std::shared_ptr<Object>& parent,
                  const std::string& interface,
                  const std::string& name);

    void operator()(const Message::Ptr&);

    std::shared_ptr<Object> parent;
    std::string interface;
    std::string name;
    MatchRule rule;
    std::mutex handlers_guard;
    std::list<Handler> handlers;
};

/**
 * @brief Template class Signal models a type-safe DBus signal.
 * @tparam SignalDescription Needs to be a model of concept SignalDescription.
 * @tparam Argument The type of the argument that is emitted by this signal.
 */
template<typename SignalDescription>
class Signal<
    SignalDescription,
    typename std::enable_if<
    is_not_void<typename SignalDescription::ArgumentType>::value,
    typename SignalDescription::ArgumentType>::type
    >
{
public:    
    typedef std::shared_ptr<Signal<SignalDescription, typename SignalDescription::ArgumentType>> Ptr;
    typedef std::function<void(const typename SignalDescription::ArgumentType&)> Handler;
    typedef typename std::list<Handler>::iterator SubscriptionToken;

    inline ~Signal() noexcept;

    inline void emit(const typename SignalDescription::ArgumentType&);

    inline SubscriptionToken connect(const Handler& h);
    inline void disconnect(const SubscriptionToken& token);

protected:
    friend class Object;

    inline static std::shared_ptr<Signal<SignalDescription,typename SignalDescription::ArgumentType>>
    make_signal(
        const std::shared_ptr<Object>& parent,
        const std::string& interface,
        const std::string& name);

private:
    inline Signal(
        const std::shared_ptr<Object>& parent,
        const std::string& interface,
        const std::string& name);

    inline void operator()(const Message::Ptr&) noexcept;

    struct ORG_FREEDESKTOP_DBUS_DLL_LOCAL Shared
    {
        Shared(
            const std::shared_ptr<Object>& parent, 
            const std::string& interface, 
            const std::string& name);

        typename SignalDescription::ArgumentType value;
        std::shared_ptr<Object> parent;
        std::string interface;
        std::string name;
        MatchRule rule;
        std::mutex handlers_guard;
        std::list<Handler> handlers;
    };
    std::shared_ptr<Shared> d;
};
}
}
}

#include "impl/signal.h"

#endif // DBUS_ORG_FREEDESKTOP_DBUS_SIGNAL_H_
