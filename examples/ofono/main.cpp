/*
 * Copyright © 2013 Canonical Ltd.
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

#include "org/freedesktop/dbus/bus.h"
#include "org/freedesktop/dbus/service.h"

#include "org/freedesktop/dbus/asio/executor.h"
#include "org/freedesktop/dbus/types/stl/tuple.h"
#include "org/freedesktop/dbus/types/stl/vector.h"
#include "org/freedesktop/dbus/types/struct.h"

#include <sys/types.h>
#include <signal.h>

namespace dbus = org::freedesktop::dbus;

namespace
{
dbus::Bus::Ptr the_session_bus()
{
    static dbus::Bus::Ptr session_bus = std::make_shared<dbus::Bus>(dbus::WellKnownBus::session);
    return session_bus;
}
}

namespace org
{
namespace ofono
{
struct MessageManager
{
    struct Signals
    {
        struct IncomingMessage
        {
            static std::string name()
            {
                return "IncomingMessage";
            };
            typedef MessageManager Interface;
            typedef std::tuple<std::string, std::map<std::string, dbus::types::Variant<dbus::types::Any>>> ArgumentType;
        };
    };
};
struct VoiceCallManager
{
    struct Signals
    {
        struct CallAdded
        {
            static std::string name()
            {
                return "CallAdded";
            };
            typedef VoiceCallManager Interface;
            typedef std::tuple<dbus::types::ObjectPath, std::map<std::string, dbus::types::Variant<dbus::types::Any>>> ArgumentType;
        };
    };
};
}
}

namespace org { namespace freedesktop { namespace dbus { namespace traits {
template<>
struct Service<org::ofono::MessageManager>
{
    static std::string interface_name() { return "org.ofono.MessageManager"; }
};
template<>
struct Service<org::ofono::VoiceCallManager>
{
    static std::string interface_name() { return "org.ofono.VoiceCallManager"; }
};
}}}}

int main(int, char**)
{
    auto bus = the_session_bus();
    bus->install_executor(org::freedesktop::dbus::Executor::Ptr(new org::freedesktop::dbus::asio::Executor{bus}));
    std::thread t {std::bind(&dbus::Bus::run, bus)};

    auto ofono = dbus::Service::use_service(bus, "org.ofono");
    auto obj = ofono->object_for_path(dbus::types::ObjectPath("/org/ofono"));
    
    auto incoming_message_signal = obj->get_signal<org::ofono::MessageManager::Signals::IncomingMessage>();
    incoming_message_signal->connect([](const std::tuple<std::string, std::map<std::string, dbus::types::Variant<dbus::types::Any>>>& arg)
                    {
                        std::cout << "Incoming message: " << std::get<0>(arg) << std::endl;
                    });

    auto call_added_signal = obj->get_signal<org::ofono::VoiceCallManager::Signals::CallAdded>();
    call_added_signal->connect([](const std::tuple<dbus::types::ObjectPath, std::map<std::string, dbus::types::Variant<dbus::types::Any>>>& arg)
                    {
                        std::cout << "Call added: " << std::get<0>(arg) << std::endl;
                    });

    sigset_t signal_set;
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGINT);
    int signal;
    sigwait(&signal_set, &signal);

    if (t.joinable())
        t.join();

    return EXIT_SUCCESS;
}
