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

#ifndef TEST_SERVICE_TINY_H_
#define TEST_SERVICE_TINY_H_

namespace test
{
struct ServiceTiny
{
    struct Interfaces
    {
        struct Foo
        {
            static const std::string& name()
            {
                static const std::string s{"this.is.unlikely.to.exist.ServiceTiny.Foo"};
                return s;
            }
        };
    };
};
}

#include <core/dbus/traits/service.h>

namespace core
{
namespace dbus
{
namespace traits
{
template<>
struct Service<test::ServiceTiny>
{
    inline static const std::string& interface_name()
    {
        static const std::string s
        {
            "this.is.unlikely.to.exist.ServiceTiny"
        };
        return s;
    }
};
}
}
}

#endif // TEST_SERVICE_TINY_H_
