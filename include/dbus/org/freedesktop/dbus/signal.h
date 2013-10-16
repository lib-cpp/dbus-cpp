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

#include <boost/signals2.hpp>

#include <functional>
#include <memory>
#include <string>

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace signals
{
typedef boost::signals2::connection Connection;
typedef boost::signals2::scoped_connection ScopedConnection;
}

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

template<typename SignalDescription, typename Argument = void>
class Signal
{
public:
    typedef std::shared_ptr<Signal<SignalDescription, void>> Ptr;
    typedef std::function<void()> Handler;

    ~Signal() noexcept;

    void emit(void);

    signals::Connection connect(const Handler& h);

protected:
    friend class Object;

    static std::shared_ptr<Signal<SignalDescription, void>> make_signal(
        const std::shared_ptr<Object>& parent,
        const std::string& interface,
        const std::string& name);

private:
    Signal(const std::shared_ptr<Object>& parent,
           const std::string& interface,
           const std::string& name);

    void operator()(const DBusMessage*);

    std::shared_ptr<Object> parent;
    std::string interface;
    std::string name;
    MatchRule rule;
    boost::signals2::signal<void()> signal;
};

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

    ~Signal() noexcept;

    void emit(const typename SignalDescription::ArgumentType&);

    signals::Connection connect(const Handler& h);

protected:
    friend class Object;

    static std::shared_ptr<Signal<SignalDescription,typename SignalDescription::ArgumentType>>
    make_signal(
        const std::shared_ptr<Object>& parent,
        const std::string& interface,
        const std::string& name);

private:
    Signal(
        const std::shared_ptr<Object>& parent,
        const std::string& interface,
        const std::string& name);

    void operator()(DBusMessage* msg) noexcept;

    struct Shared
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
        boost::signals2::signal<void(const typename SignalDescription::ArgumentType&)> signal;
    };
    std::shared_ptr<Shared> d;
};
}
}
}

#include "impl/signal.h"

#endif // DBUS_ORG_FREEDESKTOP_DBUS_SIGNAL_H_
