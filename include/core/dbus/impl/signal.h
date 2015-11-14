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
#include <core/dbus/object.h>
#include <core/dbus/lifetime_constrained_cache.h>

namespace core
{
namespace dbus
{
template<typename SignalDescription, typename Argument>
inline Signal<SignalDescription, Argument>::~Signal() noexcept
{
    signal_about_to_be_destroyed();

    parent->signal_router.uninstall_route(Object::SignalKey{interface, name});
    try
    {
        parent->remove_match(rule);
    }
    catch (...)
    {
        // Intentionally left empty as we do not care about the match rule
        // not being removed at this point. There is hardly anything we can
        // do and just assume correct state.
    }
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
inline const core::Signal<void>&
Signal<SignalDescription, Argument>::about_to_be_destroyed() const
{
    return signal_about_to_be_destroyed;
}

template<typename SignalDescription, typename Argument>
inline std::shared_ptr<Signal<SignalDescription, void>>
Signal<SignalDescription, Argument>::make_signal(
    const std::shared_ptr<Object>& parent,
    const std::string& interface,
    const std::string& name)
{
    typedef std::shared_ptr<Signal<SignalDescription, void>> SharedSignalPtr;

    typedef std::tuple<types::ObjectPath, std::string, std::string> SignalCacheKey;
    typedef Signal<SignalDescription, void> SignalCacheValue;
    typedef ThreadSafeLifetimeConstrainedCache<SignalCacheKey, SignalCacheValue> SignalCache;

    static SignalCache signal_cache;

    auto key = std::make_tuple(parent->path(), interface, name);
    auto signal = signal_cache.retrieve_value_for_key(key);

    if (signal)
        return signal;

    signal = SharedSignalPtr(
                new Signal<SignalDescription, void>(
                    parent,
                    interface,
                    name));

    signal_cache.insert_value_for_key(key, signal);

    return signal;
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
    rule = rule.type(Message::Type::signal).interface(interface).member(name);
    parent->add_match(rule);
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
    d->signal_about_to_be_destroyed();

    d->parent->signal_router.uninstall_route(
        Object::SignalKey{d->interface, d->name});

    // Iterate through the unique keys in the map
    for (auto it = d->handlers.begin(); it != d->handlers.end();
            it = d->handlers.upper_bound(it->first))
    {
        const MatchRule::MatchArgs& match_args(it->first);

        try
        {
            d->parent->remove_match(d->rule.args(match_args));
        }
        catch (...)
        {
        }
    }
}

template<typename SignalDescription>
inline void
Signal<
    SignalDescription,
    typename std::enable_if<
        is_not_void<typename SignalDescription::ArgumentType>::value,
        typename SignalDescription::ArgumentType>::type
    >::emit(const typename SignalDescription::ArgumentType& args)
{
    d->parent->template emit_signal<SignalDescription, typename SignalDescription::ArgumentType>(args);
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
    return connect_with_match_args(h, MatchRule::MatchArgs());
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
>::connect_with_match_args(const Handler& h, const MatchRule::MatchArgs& match_args)
{
    std::lock_guard<std::mutex> lg(d->handlers_guard);

    bool new_entry = (d->handlers.find(match_args) == d->handlers.cend());

    SubscriptionToken token = d->handlers.insert(std::make_pair(match_args, h));

    if (new_entry)
        d->parent->add_match(d->rule.args(match_args));

    return token;
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
    std::lock_guard<std::mutex> lg(d->handlers_guard);

    MatchRule::MatchArgs match_args(token->first);
    d->handlers.erase(token);
    if (d->handlers.count(match_args) == 0)
    {
        d->parent->remove_match(d->rule.args(match_args));
    }
}

template<typename SignalDescription>
inline const core::Signal<void>&
Signal<
    SignalDescription,
    typename std::enable_if<
        is_not_void<typename SignalDescription::ArgumentType>::value,
        typename SignalDescription::ArgumentType
    >::type
>::about_to_be_destroyed() const
{
    return d->signal_about_to_be_destroyed;
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
    typedef std::shared_ptr<Signal<SignalDescription, typename SignalDescription::ArgumentType>> SharedSignalPtr;

    typedef std::tuple<types::ObjectPath, std::string, std::string> SignalCacheKey;
    typedef Signal<SignalDescription, typename SignalDescription::ArgumentType> SignalCacheValue;
    typedef ThreadSafeLifetimeConstrainedCache<SignalCacheKey, SignalCacheValue> SignalCache;

    static SignalCache signal_cache;

    auto key = std::make_tuple(parent->path(), interface, name);
    auto signal = signal_cache.retrieve_value_for_key(key);

    if (signal)
        return signal;

    signal = SharedSignalPtr(
                new Signal<SignalDescription, typename SignalDescription::ArgumentType>(
                    parent,
                    interface,
                    name));

    signal_cache.insert_value_for_key(key, signal);

    return signal;
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

    d->rule = d->rule.type(Message::Type::signal).interface(interface).member(name);
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
        typename SignalDescription::ArgumentType value;
        msg->reader() >> value;
        std::lock_guard<std::mutex> lg(d->handlers_guard);
        for (auto it : d->handlers)
        {
            const MatchRule::MatchArgs& match_args(it.first);
            const Handler &handler(it.second);

            if (!match_args.empty())
            {
                bool matched = true;
                for(const MatchRule::MatchArg& arg: match_args)
                {
                    std::size_t index = arg.first;
                    const std::string& value = arg.second;

                    // FIXME This code is probably bad, it starts reading the arguments
                    // from the beginning each time
                    auto reader = msg->reader();

                    // Wind the reader forward until we get to the desired point
                    for (std::size_t i(0); i < index && reader.type() != dbus::ArgumentType::invalid; ++i)
                        reader.pop();

                    if (value != reader.pop_string())
                    {
                        matched = false;
                        continue;
                    }
                }

                if (!matched)
                    continue;
            }

            handler(value);
        }
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


