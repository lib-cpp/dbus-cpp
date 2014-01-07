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
#ifndef CORE_DBUS_FIXTURE_H_
#define CORE_DBUS_FIXTURE_H_

#include <core/dbus/visibility.h>

#include <memory>
#include <string>

namespace core
{
namespace dbus
{
class Bus;

/**
 * @brief The Fixture class provides private session and system bus instances for testing purposes.
 */
class ORG_FREEDESKTOP_DBUS_DLL_PUBLIC Fixture
{
public:
    /**
     * @brief Constructs a fixture instance with the two given configuration files.
     *
     * Any test running within the scope of this fixture will access the private
     * session- and system-bus instances setup by this class.
     *
     * @param session_bus_config_file Path to the session-bus configuration file.
     * @param system_bus_config_file Path to the system-bus configuration file.
     */
    Fixture(const std::string& session_bus_config_file,
            const std::string& system_bus_config_file);

    virtual ~Fixture();

    /**
     * @brief create_connection_to_session_bus creates a new connection to the testing session bus instance.
     */
    std::shared_ptr<Bus> create_connection_to_session_bus();

    /**
     * @brief create_connection_to_system_bus creates a new connection to the testing system bus instance.
     */
    std::shared_ptr<Bus> create_connection_to_system_bus();

private:
    struct Private;
    std::unique_ptr<Private> d;
};
}
}

#endif // CORE_DBUS_FIXTURE_H_
