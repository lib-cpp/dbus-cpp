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

#ifndef TEST_SERVICE_H_
#define TEST_SERVICE_H_

namespace test
{
struct Service
{
    struct Method
    {
        typedef Service Interface;

        inline static const std::string& name()
        {
            static const std::string s
            {
                "Method"
            };
            return s;
        }

        inline static const std::chrono::milliseconds default_timeout()
        {
            return std::chrono::milliseconds{10};
        }
    };

    struct Signals
    {
        struct Dummy
        {
            inline static std::string name()
            {
                return "Dummy";
            };
            typedef Service Interface;
            typedef int64_t ArgumentType;
        };
    };

    struct Properties
    {
        struct Dummy
        {
            inline static std::string name()
            {
                return "Dummy";
            };
            typedef Service Interface;
            typedef double ValueType;
            static const bool readable = true;
            static const bool writable = true;
        };

        struct ReadOnly
        {
            inline static std::string name()
            {
                return "ReadOnly";
            };
            typedef Service Interface;
            typedef std::uint32_t ValueType;
            static const bool readable = true;
            static const bool writable = false;
        };
    };

    struct Interfaces
    {
        struct Foo
        {
            static const std::string& name()
            {
                static const std::string s{"this.is.unlikely.to.exist.Service.Foo"};
                return s;
            }

            struct Signals
            {
                struct Dummy
                {
                    inline static std::string name()
                    {
                        return "Dummy";
                    }
                    typedef Foo Interface;
                    typedef int64_t ArgumentType;
                };

                struct Bar
                {
                    inline static std::string name()
                    {
                        return "Bar";
                    }
                    typedef Foo Interface;
                    typedef int64_t ArgumentType;
                };
            };
        };
    };
};
}

#include <core/dbus/traits/service.h>

#include <iostream>

namespace core
{
namespace dbus
{
namespace traits
{
template<>
struct Service<test::Service>
{
    inline static const std::string& interface_name()
    {
        static const std::string s
        {
            "this.is.unlikely.to.exist.Service"
        };
        return s;
    }
};
}
}
}

#endif // TEST_SERVICE_H_
