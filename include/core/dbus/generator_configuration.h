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
#ifndef CORE_DBUS_GENERATOR_CONFIGURATION_H_
#define CORE_DBUS_GENERATOR_CONFIGURATION_H_

#include <functional>

namespace core
{
namespace dbus
{
/**
  * \brief Configuration options and customization points for the code generator.
  */
struct GeneratorConfiguration
{
    std::function<std::string()> include_guard_generator;
    std::function<std::string()> build_time_stamp_generator;
};
}
}

#endif // CORE_DBUS_GENERATOR_CONFIGURATION_H_
