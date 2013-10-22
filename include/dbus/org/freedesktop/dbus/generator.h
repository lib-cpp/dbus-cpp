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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_GENERATOR_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_GENERATOR_H_

#include <org/freedesktop/dbus/compiler.h>

#include <fstream>
#include <functional>
#include <memory>

namespace org
{
namespace freedesktop
{
namespace dbus
{
class GeneratorConfiguration;

class Generator
{
  public:
    static const GeneratorConfiguration& default_configuration();

    Generator();
    Generator(const Generator&) = delete;
    virtual ~Generator();

    bool operator==(const Generator&) const = delete;
    Generator& operator=(const Generator&) = delete;

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
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_GENERATOR_H_
