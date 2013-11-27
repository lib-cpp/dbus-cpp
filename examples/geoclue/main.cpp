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

#include "geoclue.h"

#include <org/freedesktop/dbus/bus.h>

#include <org/freedesktop/dbus/asio/executor.h>
#include <org/freedesktop/dbus/interfaces/properties.h>
#include <org/freedesktop/dbus/types/stl/tuple.h>
#include <org/freedesktop/dbus/types/stl/vector.h>
#include <org/freedesktop/dbus/types/struct.h>

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

int main(int, char**)
{
    auto bus = the_session_bus();
    bus->install_executor(org::freedesktop::dbus::asio::make_executor(bus));
    std::thread t {std::bind(&dbus::Bus::run, bus)};
    auto ubuntu_geoip = dbus::Service::use_service(bus, "org.freedesktop.Geoclue.Providers.UbuntuGeoIP");
    auto ubuntu_geoip_obj = ubuntu_geoip->object_for_path(dbus::types::ObjectPath("/org/freedesktop/Geoclue/Providers/UbuntuGeoIP"));

    // Connect to signal
    auto position_changed_signal = ubuntu_geoip_obj->get_signal<org::freedesktop::Geoclue::Position::Signals::PositionChanged>();
    position_changed_signal->connect([](const org::freedesktop::Geoclue::Position::Signals::PositionChanged::ArgumentType&)
    {
        std::cout << "org::freedesktop::Geoclue::Position::Signals::PositionChanged" << std::endl;
    });

    // Demonstrates tying tuple values to fields of a custom struct.
    struct Position
    {
        int32_t fields;
        int32_t timestamp;
        double latitude;
        double longitude;
        double altitude;
    } p;
    std::tie(p.fields, p.timestamp, p.latitude, p.longitude, p.altitude, std::ignore)
        = ubuntu_geoip_obj->invoke_method_synchronously<
          org::freedesktop::Geoclue::Position::GetPosition,
          std::tuple<int32_t, int32_t, double, double, double, dbus::types::Struct<std::tuple<int32_t, double, double>>>
          >().value();
    std::cout 	<< p.fields << ", "
                << p.timestamp << ", "
                << p.latitude << ", "
                << p.longitude << ", "
                << p.altitude << std::endl;

    // Illustrates std::get-based access to.
    auto address = ubuntu_geoip_obj->invoke_method_synchronously<
                   org::freedesktop::Geoclue::Address::GetAddress,
                   std::tuple<int32_t, std::map<std::string, std::string>, dbus::types::Struct<std::tuple<int32_t, double, double>>>
                   >().value();
    std::cout << std::get<0>(address) << std::endl;
    std::for_each(std::get<1>(address).begin(), std::get<1>(address).end(), [](const std::pair<std::string, std::string>& p)
    {
        std::cout << p.first << " -> " << p.second << std::endl;
    });
    auto geoclue = dbus::Service::use_service(bus, dbus::traits::Service<org::freedesktop::Geoclue::Master>::interface_name());
    auto geoclue_obj = geoclue->object_for_path(dbus::types::ObjectPath("/org/freedesktop/Geoclue/Master"));
    auto session_path = geoclue_obj->invoke_method_synchronously<org::freedesktop::Geoclue::Master::Create, dbus::types::ObjectPath>().value();
    auto geoclue_client = geoclue->object_for_path(session_path);

    try
    {
        geoclue_client->invoke_method_synchronously<
        org::freedesktop::Geoclue::MasterClient::SetRequirements,
            void,
            int32_t, int32_t, bool, int32_t
            >(1, 100, false, (1 << 10)-1);
    }
    catch (const std::runtime_error& e)
    {
        std::cout << e.what() << std::endl;
    }

    sigset_t signal_set;
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGINT);
    int signal;
    sigwait(&signal_set, &signal);

    if (t.joinable())
        t.join();

    return EXIT_SUCCESS;
}
