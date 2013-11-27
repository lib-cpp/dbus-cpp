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
#ifndef CORE_DBUS_GENERATOR_H_
#define CORE_DBUS_GENERATOR_H_

#include <org/freedesktop/dbus/compiler.h>

#include <fstream>
#include <functional>
#include <memory>

namespace core
{
namespace dbus
{
struct GeneratorConfiguration;

/**
 * @brief Interface and implementation to generate boilerplate bindings code for interfacing with DBus services.
 */
class Generator
{
  public:
    /**
     * @brief Specifies the default generator configuration.
     * @return An instance of the default generator configuration.
     */
    static const GeneratorConfiguration& default_configuration();

    /**
     * @brief Constructs a default generator.
     */
    Generator();
    Generator(const Generator&) = delete;
    virtual ~Generator();

    bool operator==(const Generator&) const = delete;
    Generator& operator=(const Generator&) = delete;

    /**
     * @brief Invokes the generator for the supplied root element and configuration.
     * @param element The root element that the generator should start with.
     * @param raw_file_contents The raw file contents that the intermediate model has been parsed from.
     * @param config The configuration of the generator.
     * @return true if the final binding boilerplate code has been generated successfully, false otherwise.
     */
    virtual bool invoke_for_model_with_configuration(
            const std::shared_ptr<Compiler::Element>& element,
            std::istream& raw_file_contents,
            const GeneratorConfiguration& config = default_configuration());

  private:
    struct Private;
    std::unique_ptr<Private> d;
};
}
}

#endif // CORE_DBUS_GENERATOR_H_
