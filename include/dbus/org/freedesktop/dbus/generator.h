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

#include <memory>

namespace org
{
namespace freedesktop
{
namespace dbus
{
class Generator
{
  public:
    Generator();
    Generator(const Generator&) = delete;
    virtual ~Generator();

    bool operator==(const Generator&) const = delete;
    Generator& operator=(const Generator&) = delete;

    virtual bool invoke_for_model(const std::shared_ptr<Compiler::Element>& element);

  private:
    struct Private;
    std::unique_ptr<Private> d;
};
}
}
}

#include "impl/generator.h"

#endif // DBUS_ORG_FREEDESKTOP_DBUS_GENERATOR_H_
