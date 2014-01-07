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

#include <core/dbus/fixture.h>

#include <core/dbus/bus.h>

#include <core/posix/exec.h>
#include <core/posix/this_process.h>

struct core::dbus::Fixture::Private
{
    struct Session
    {
        Session(const std::string& config_file)
        {
            std::vector<std::string> argv
            {
                "--config-file",
                config_file,
                "--print-address"
            };

            std::map<std::string, std::string> env;
            core::posix::this_process::env::for_each([&env](const std::string& key, const std::string& value)
            {
                env.insert(std::make_pair(key, value));
            });

            daemon = core::posix::exec(
                        "/bin/dbus-daemon",
                        argv,
                        env,
                        core::posix::StandardStream::stdout);

            daemon.cout() >> address;

            if (address.empty())
                throw std::runtime_error("Session: Could not read address of bus instance.");

            core::posix::this_process::env::set_or_throw("DBUS_SESSION_BUS_ADDRESS", address);
            core::posix::this_process::env::set_or_throw("DBUS_STARTER_ADDRESS", address);
            core::posix::this_process::env::set_or_throw("DBUS_STARTER_BUS_TYPE", "session");
        }

        core::posix::ChildProcess daemon = core::posix::ChildProcess::invalid();
        std::string address;
    } session;

    struct System
    {
        System(const std::string& config_file)
        {
            std::vector<std::string> argv
            {
                "--config-file",
                config_file,
                "--print-address"
            };

            std::map<std::string, std::string> env;
            core::posix::this_process::env::for_each([&env](const std::string& key, const std::string& value)
            {
                env.insert(std::make_pair(key, value));
            });

            daemon = core::posix::exec(
                        "/bin/dbus-daemon",
                        argv,
                        env,
                        core::posix::StandardStream::stdout);

            daemon.cout() >> address;

            if (address.empty())
                throw std::runtime_error("System: Could not read address of bus instance.");

            core::posix::this_process::env::set_or_throw("DBUS_SYSTEM_BUS_ADDRESS", address);
        }

        core::posix::ChildProcess daemon = core::posix::ChildProcess::invalid();
        std::string address;
    } system;
};

core::dbus::Fixture::Fixture(const std::string& session_bus_configuration_file,
                             const std::string& system_bus_configuration_file)
    : d(new Private{Private::Session{session_bus_configuration_file},
                    Private::System{system_bus_configuration_file}})
{
}

core::dbus::Fixture::~Fixture()
{
}

std::shared_ptr<core::dbus::Bus> core::dbus::Fixture::create_connection_to_session_bus()
{
    return std::shared_ptr<core::dbus::Bus>(new core::dbus::Bus{d->session.address});
}

std::shared_ptr<core::dbus::Bus> core::dbus::Fixture::create_connection_to_system_bus()
{
    return std::shared_ptr<core::dbus::Bus>(new core::dbus::Bus{d->system.address});
}
