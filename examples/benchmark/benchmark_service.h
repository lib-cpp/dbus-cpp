#ifndef BENCHMARK_SERVICE_H_
#define BENCHMARK_SERVICE_H_

#include "org/freedesktop/dbus/service.h"
#include "org/freedesktop/dbus/skeleton.h"
#include "org/freedesktop/dbus/stub.h"

namespace dbus = org::freedesktop::dbus;

namespace test
{
class IBenchmarkService
{
protected:
    struct MethodInt64
    {
        typedef IBenchmarkService Interface;

        static const std::string& name()
        {
            static const std::string s
            {
                "MethodInt64"
            };
            return s;
        }

        static const std::chrono::milliseconds default_timeout;
    };

    struct MethodVectorInt32
    {
        typedef IBenchmarkService Interface;

        static const std::string& name()
        {
            static const std::string s
            {
                "MethodVectorInt32"
            };
            return s;
        }

        static const std::chrono::milliseconds default_timeout;
    };

public:
    virtual ~IBenchmarkService() = default;

    virtual int64_t method_int64(int64_t arg) = 0;
    virtual std::vector<int32_t> method_vector_int32(const std::vector<int32_t>& v) = 0;
};
}

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace traits
{
template<>
struct Service<test::IBenchmarkService>
{
    static const std::string& interface_name()
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
}
namespace test
{
class BenchmarkServiceStub : public dbus::Stub<IBenchmarkService>
{
public:
	typedef std::shared_ptr<BenchmarkServiceStub> Ptr;
	
    BenchmarkServiceStub(const dbus::Bus::Ptr& bus) : dbus::Stub<IBenchmarkService>(bus),
        object(access_service()->object_for_path(dbus::types::ObjectPath("/org/freedesktop/dbus/benchmark/Service")))
    {
    }

    ~BenchmarkServiceStub() noexcept = default;

    int64_t method_int64(int64_t arg)
    {
        auto result = object->invoke_method_synchronously<IBenchmarkService::MethodInt64, int64_t>(arg);
        if (result.is_error())
            throw std::runtime_error(result.error());

        return result.value();
    }

    std::vector<int32_t> method_vector_int32(const std::vector<int32_t>& arg)
    {
        auto result = object->invoke_method_synchronously<IBenchmarkService::MethodVectorInt32, std::vector<int32_t>>(arg);
        if (result.is_error())
            throw std::runtime_error(result.error());

        return result.value();
    }

private:
    dbus::Object::Ptr object;
};

class BenchmarkServiceSkeleton : public dbus::Skeleton<IBenchmarkService>
{
public:
    BenchmarkServiceSkeleton(const dbus::Bus::Ptr& bus) : dbus::Skeleton<IBenchmarkService>(bus),
        object(access_service()->add_object_for_path(dbus::types::ObjectPath("/org/freedesktop/dbus/benchmark/Service")))
    {
        object->install_method_handler<IBenchmarkService::MethodInt64>(
            std::bind(&BenchmarkServiceSkeleton::handle_method_int64, this, std::placeholders::_1));
        object->install_method_handler<IBenchmarkService::MethodVectorInt32>(
            std::bind(&BenchmarkServiceSkeleton::handle_method_vector_int32, this, std::placeholders::_1));
    }

    ~BenchmarkServiceSkeleton() noexcept = default;

private:
    void handle_method_int64(DBusMessage* msg)
    {
        auto m = dbus::Message::from_raw_message(msg);

        int64_t in;
        m->reader() >> in;
        int64_t out = method_int64(in);
        auto reply = dbus::Message::make_method_return(msg);
        reply->writer() << out;
        access_bus()->send(reply->get());
    }

    void handle_method_vector_int32(DBusMessage* msg)
    {
        auto m = dbus::Message::from_raw_message(msg);

        std::vector<int32_t> in;
        m->reader() >> in;
        auto out = method_vector_int32(in);
        auto reply = dbus::Message::make_method_return(msg);
        reply->writer() << out;
        access_bus()->send(reply->get());
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

const std::chrono::milliseconds IBenchmarkService::MethodInt64::default_timeout
{
    std::chrono::seconds(10)
};
const std::chrono::milliseconds IBenchmarkService::MethodVectorInt32::default_timeout
{
    std::chrono::seconds(10)
};
}

#endif // BENCHMARK_SERVICE_H_
