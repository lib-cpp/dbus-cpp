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
#ifndef CORE_DBUS_IMPL_SIGNAL_H_
#define CORE_DBUS_IMPL_SIGNAL_H_

#include <core/dbus/message_streaming_operators.h>

namespace core
{
namespace dbus
{
template<typename SignalDescription, typename Argument>
inline Signal<SignalDescription, Argument>::~Signal() noexcept
{
    parent->signal_router.uninstall_route(Object::SignalKey{interface, name});
    parent->remove_match(rule);
}

template<typename SignalDescription, typename Argument>
inline void
Signal<SignalDescription, Argument>::emit(void)
{
    parent->emit_signal<SignalDescription>();
}

template<typename SignalDescription, typename Argument>
inline typename Signal<SignalDescription, Argument>::SubscriptionToken
Signal<SignalDescription, Argument>::connect(const Handler& h)
{
    std::lock_guard<std::mutex> lg(handlers_guard);
    return handlers.insert(handlers.end(), h);
}

template<typename SignalDescription, typename Argument>
inline void
Signal<SignalDescription, Argument>::disconnect(
        const typename Signal<SignalDescription, Argument>::SubscriptionToken& token)
{
    std::lock_guard<std::mutex> lg(handlers_guard);
    return handlers.erase(token);
}

template<typename SignalDescription, typename Argument>
inline std::shared_ptr<Signal<SignalDescription, void>>
Signal<SignalDescription, Argument>::make_signal(
    const std::shared_ptr<Object>& parent,
    const std::string& interface,
    const std::string& name)
{
    static auto sp =
            std::shared_ptr<Signal<SignalDescription, void>>(
                new Signal<SignalDescription, void>(
                    parent,
                    interface,
                    name));
    return sp;
}

template<typename SignalDescription, typename Argument>
inline Signal<SignalDescription, Argument>::Signal(
    const std::shared_ptr<Object>& parent,
    const std::string& interface,
    const std::string& name) : parent(parent),
                               interface(interface),
                               name(name)
{
    parent->signal_router.install_route(
        Object::SignalKey {interface, name},
        std::bind(
            &Signal<SignalDescription>::operator(),
            this,
            std::placeholders::_1));
    parent->add_match(
        rule.type(Message::Type::signal).interface(interface).member(name));
}

template<typename SignalDescription, typename Argument>
inline void
Signal<SignalDescription, Argument>::operator()(const Message::Ptr&)
{
    std::lock_guard<std::mutex> lg(handlers_guard);
    for (auto handler : handlers)
        handler();
}

template<typename SignalDescription>
inline Signal<
    SignalDescription,
    typename std::enable_if<
        is_not_void<typename SignalDescription::ArgumentType>::value,
        typename SignalDescription::ArgumentType>::type
    >::~Signal() noexcept
{
    d->parent->signal_router.uninstall_route(
        Object::SignalKey{d->interface, d->name});
    d->parent->remove_match(d->rule);
}

template<typename SignalDescription>
inline void
Signal<
    SignalDescription,
    typename std::enable_if<
        is_not_void<typename SignalDescription::ArgumentType>::value,
        typename SignalDescription::ArgumentType>::type
    >::emit(const typename SignalDescription::ArgumentType&)
{
    d->parent->template emit_signal<SignalDescription, typename SignalDescription::ArgumentType>();
}

template<typename SignalDescription>
inline typename Signal<
SignalDescription,
typename std::enable_if<
    is_not_void<typename SignalDescription::ArgumentType>::value,
    typename SignalDescription::ArgumentType
>::type
>::SubscriptionToken
Signal<
    SignalDescription,
    typename std::enable_if<
        is_not_void<typename SignalDescription::ArgumentType>::value,
        typename SignalDescription::ArgumentType
    >::type
>::connect(const Handler& h)
{
    std::lock_guard<std::mutex> lg(d->handlers_guard);
    return d->handlers.insert(d->handlers.end(), h);
}

template<typename SignalDescription>
inline void
Signal<
    SignalDescription,
    typename std::enable_if<
        is_not_void<typename SignalDescription::ArgumentType>::value,
        typename SignalDescription::ArgumentType
    >::type
>::disconnect(const SubscriptionToken& token)
{
    std::lock_guard<std::mutex> lg(d.handlers_guard);
    return d.handlers.erase(token);
}

template<typename SignalDescription>
inline std::shared_ptr<Signal<SignalDescription,typename SignalDescription::ArgumentType>>
Signal<
    SignalDescription,
    typename std::enable_if<
        is_not_void<typename SignalDescription::ArgumentType>::value,
        typename SignalDescription::ArgumentType>::type
    >::make_signal(
    const std::shared_ptr<Object>& parent,
    const std::string& interface,
    const std::string& name)
{
    static auto sp =
            std::shared_ptr<Signal<SignalDescription, typename SignalDescription::ArgumentType>>(
                new Signal<SignalDescription, typename SignalDescription::ArgumentType>(
                    parent,
                    interface,
                    name));
    return sp;
}

template<typename SignalDescription>
inline Signal<
    SignalDescription,
    typename std::enable_if<
        is_not_void<typename SignalDescription::ArgumentType>::value,
        typename SignalDescription::ArgumentType>::type
    >::Signal(const std::shared_ptr<Object>& parent,
              const std::string& interface,
              const std::string& name)
        : d{new Shared{parent, interface, name}}
{
    d->parent->signal_router.install_route(
        Object::SignalKey {interface, name},
        std::bind(
            &Signal<SignalDescription, typename SignalDescription::ArgumentType>::operator(),
            this,
            std::placeholders::_1));
    d->parent->add_match(d->rule.type(Message::Type::signal).interface(interface).member(name));
}

template<typename SignalDescription>
inline void
Signal<
    SignalDescription,
    typename std::enable_if<
        is_not_void<typename SignalDescription::ArgumentType>::value,
        typename SignalDescription::ArgumentType>::type
    >::operator()(const Message::Ptr& msg) noexcept
{
    try
    {
        msg->reader() >> d->value;
        std::lock_guard<std::mutex> lg(d->handlers_guard);
        for (auto handler : d->handlers)
            handler(d->value);
    }
    catch (const std::runtime_error& e)
    {
        std::cout << e.what() << std::endl;
    }
}

template<typename SignalDescription>
inline Signal<
    SignalDescription,
    typename std::enable_if<
        is_not_void<typename SignalDescription::ArgumentType>::value,
        typename SignalDescription::ArgumentType>::type
    >::Shared::Shared(const std::shared_ptr<Object>& parent, const std::string& interface, const std::string& name)
            : parent(parent),
              interface(interface),
              name(name)
{
}
}
}

#endif // CORE_DBUS_IMPL_SIGNAL_H_


