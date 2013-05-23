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
            return std::chrono::seconds{1};
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
    };
};
}

#include "org/freedesktop/dbus/traits/service.h"

namespace org
{
namespace freedesktop
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
}

#endif // TEST_SERVICE_H_
