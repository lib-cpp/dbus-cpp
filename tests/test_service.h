#ifndef TEST_SERVICE_H_
#define TEST_SERVICE_H_

namespace test
{
struct Service
{
    struct Method
    {
        typedef Service Interface;

        static const std::string& name()
        {
            static const std::string s
            {
                "Method"
            };
            return s;
        }

        static const std::chrono::milliseconds default_timeout;
    };

    struct Signals
    {
        struct Dummy
        {
            static std::string name()
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
            static std::string name()
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

const std::chrono::milliseconds Service::Method::default_timeout
{
    10*1000
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
    static const std::string& interface_name()
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
