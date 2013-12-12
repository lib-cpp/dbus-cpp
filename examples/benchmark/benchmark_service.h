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

#ifndef BENCHMARK_SERVICE_H_
#define BENCHMARK_SERVICE_H_

#include <core/dbus/service.h>
#include <core/dbus/skeleton.h>
#include <core/dbus/stub.h>

namespace dbus = core::dbus;

namespace test
{
class IBenchmarkService
{
protected:
    struct MethodInt64
    {
        typedef IBenchmarkService Interface;

        inline static const std::string& name()
        {
            static const std::string s
            {
                "MethodInt64"
            };
            return s;
        }

        inline static const std::chrono::milliseconds default_timeout()
        {
            return std::chrono::seconds{1};
    }
};

    struct MethodVectorInt32
    {
        typedef IBenchmarkService Interface;

        inline static const std::string& name()
        {
            static const std::string s
            {
                "MethodVectorInt32"
            };
            return s;
        }

        inline static const std::chrono::milliseconds default_timeout()
        {
            return std::chrono::seconds{1};
    }
};

public:
    virtual ~IBenchmarkService() = default;

    virtual int64_t method_int64(int64_t arg) = 0;
    virtual std::vector<int32_t> method_vector_int32(const std::vector<int32_t>& v) = 0;
};
}

namespace core
{
namespace dbus
{
namespace traits
{
template<>
struct Service<test::IBenchmarkService>
{
    inline static const std::string& interface_name()
    {
        static const std::string s
        {
            "org.freedesktop.dbus.benchmark.Service"
        };
        return s;
    }
};
}
}
}
namespace test
{
class BenchmarkServiceStub : public dbus::Stub<IBenchmarkService>
{
public:
    typedef std::shared_ptr<BenchmarkServiceStub> Ptr;

    BenchmarkServiceStub(const dbus::Bus::Ptr& bus) : dbus::Stub<IBenchmarkService>(bus),
        object(access_service()->object_for_path(dbus::types::ObjectPath("/core/dbus/benchmark/Service")))
    {
    }

    ~BenchmarkServiceStub() noexcept = default;

    int64_t method_int64(int64_t arg)
    {
        auto result = object->invoke_method_synchronously<IBenchmarkService::MethodInt64, int64_t>(arg);
        if (result.is_error())
            throw std::runtime_error(result.error().print());

        return result.value();
    }

    std::vector<int32_t> method_vector_int32(const std::vector<int32_t>& arg)
    {
        auto result = object->invoke_method_synchronously<IBenchmarkService::MethodVectorInt32, std::vector<int32_t>>(arg);
        if (result.is_error())
            throw std::runtime_error(result.error().print());

        return result.value();
    }

private:
    dbus::Object::Ptr object;
};

class BenchmarkServiceSkeleton : public dbus::Skeleton<IBenchmarkService>
{
public:
    BenchmarkServiceSkeleton(const dbus::Bus::Ptr& bus) : dbus::Skeleton<IBenchmarkService>(bus),
        object(access_service()->add_object_for_path(dbus::types::ObjectPath("/core/dbus/benchmark/Service")))
    {
        object->install_method_handler<IBenchmarkService::MethodInt64>(
                    std::bind(&BenchmarkServiceSkeleton::handle_method_int64, this, std::placeholders::_1));
        object->install_method_handler<IBenchmarkService::MethodVectorInt32>(
                    std::bind(&BenchmarkServiceSkeleton::handle_method_vector_int32, this, std::placeholders::_1));
    }

    ~BenchmarkServiceSkeleton() noexcept = default;

private:
    void handle_method_int64(const dbus::Message::Ptr& msg)
    {
        int64_t in;
        msg->reader() >> in;
        int64_t out = method_int64(in);
        auto reply = dbus::Message::make_method_return(msg);
        reply->writer() << out;
        access_bus()->send(reply);
    }

    void handle_method_vector_int32(const dbus::Message::Ptr& msg)
    {
        std::vector<int32_t> in;
        msg->reader() >> in;
        auto out = method_vector_int32(in);
        auto reply = dbus::Message::make_method_return(msg);
        reply->writer() << out;

        access_bus()->send(reply);
    }

    dbus::Object::Ptr object;
};

class BenchmarkService : public BenchmarkServiceSkeleton
{
public:
    typedef std::shared_ptr<BenchmarkService> Ptr;

    BenchmarkService(const dbus::Bus::Ptr& bus) : BenchmarkServiceSkeleton(bus)
    {
    }

    int64_t method_int64(int64_t arg)
    {
        return arg;
    }

    std::vector<int32_t> method_vector_int32(const std::vector<int32_t>& arg)
    {
        return arg;
    }
};
}

#endif // BENCHMARK_SERVICE_H_
