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

#include <core/dbus/dbus.h>
#include <core/dbus/bus.h>
#include <core/dbus/object.h>
#include <core/dbus/service.h>
#include <core/dbus/service_watcher.h>
#include <core/dbus/signal.h>

#include <core/dbus/asio/executor.h>
#include <core/dbus/types/stl/tuple.h>
#include <core/dbus/types/stl/vector.h>
#include <core/dbus/types/struct.h>

#include <sys/types.h>
#include <signal.h>

namespace dbus = core::dbus;

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

namespace core { namespace dbus { namespace traits {
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
}}}

int main(int, char**)
{
    auto bus = the_session_bus();
    bus->install_executor(core::dbus::asio::make_executor(bus));
    std::thread t {std::bind(&dbus::Bus::run, bus)};

    auto ofono = dbus::Service::use_service(bus, "org.ofono");
    auto obj = ofono->object_for_path(dbus::types::ObjectPath("/org/ofono"));

    dbus::DBus daemon(bus);
    dbus::ServiceWatcher::Ptr watcher_one(
        daemon.make_service_watcher("com.canonical.Unity.WindowStack",
                dbus::DBus::WatchMode::registration));
    watcher_one->owner_changed.connect(
        [](const std::string& old_owner, const std::string& new_owner)
        {
            std::cout << "first name_owner_changed  |" << old_owner << "|"
                    << new_owner << "|" << std::endl;
        });
    watcher_one->service_registered.connect([]()
        {
            std::cout << "first service registered" << std::endl;
        });
    watcher_one->service_unregistered.connect([]()
        {
            std::cout << "first service unregistered" << std::endl;
        });
    dbus::ServiceWatcher::Ptr watcher_two(
        daemon.make_service_watcher("com.canonical.Unity.WindowStack",
                dbus::DBus::WatchMode::unregistration));
    watcher_two->owner_changed.connect(
        [](const std::string& old_owner, const std::string& new_owner)
        {
            std::cout << "second name_owner_changed  |" << old_owner << "|"
            << new_owner << "|" << std::endl;
        });
    
    /*auto incoming_message_signal = obj->get_signal<org::ofono::MessageManager::Signals::IncomingMessage>();
    incoming_message_signal->connect([](const std::tuple<std::string, std::map<std::string, dbus::types::Variant<dbus::types::Any>>>& arg)
                    {
                        std::cout << "Incoming message: " << std::get<0>(arg) << std::endl;
                    });

    auto call_added_signal = obj->get_signal<org::ofono::VoiceCallManager::Signals::CallAdded>();
    call_added_signal->connect([](const std::tuple<dbus::types::ObjectPath, std::map<std::string, dbus::types::Variant<dbus::types::Any>>>& arg)
                    {
                        std::cout << "Call added: " << std::get<0>(arg) << std::endl;
                    });*/

    sigset_t signal_set;
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGINT);
    int signal;
    sigwait(&signal_set, &signal);

    if (t.joinable())
        t.join();

    return EXIT_SUCCESS;
}
