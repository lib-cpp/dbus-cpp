#include "org/freedesktop/dbus/asio/executor.h"

#include "org/freedesktop/dbus/dbus.h"
#include "org/freedesktop/dbus/service.h"

#include "cross_process_sync.h"
#include "fork_and_run.h"
#include "test_service.h"

#include <gtest/gtest.h>

namespace dbus = org::freedesktop::dbus;

namespace
{
dbus::Bus::Ptr the_session_bus()
{
    dbus::Bus::Ptr session_bus = std::make_shared<dbus::Bus>(dbus::WellKnownBus::session);
    return session_bus;
}
}

TEST(Executor, ThrowsOnConstructionFromNullBus)
{
    EXPECT_ANY_THROW(org::freedesktop::dbus::asio::Executor executor{org::freedesktop::dbus::Bus::Ptr{}});
}

TEST(Executor, DoesNotThrowForExistingBus)
{
    org::freedesktop::dbus::Bus::Ptr bus{new org::freedesktop::dbus::Bus{org::freedesktop::dbus::WellKnownBus::session}};
    EXPECT_NO_THROW(bus->install_executor(org::freedesktop::dbus::Executor::Ptr(new org::freedesktop::dbus::asio::Executor{bus})));
}

TEST(Executor, ABusRunByAnExecutorReceivesSignals)
{
    test::CrossProcessSync cross_process_sync;
    
    const int64_t expected_value = 42;
    auto child = [expected_value, &cross_process_sync]()
    {
        auto bus = the_session_bus();        
        bus->install_executor(dbus::Executor::Ptr{new dbus::asio::Executor{bus}});
        auto service = dbus::Service::add_service<test::Service>(bus);
        auto skeleton = service->add_object_for_path(dbus::types::ObjectPath("/this/is/unlikely/to/exist/Service"));
        skeleton->install_method_handler<test::Service::Method>(
            [bus, skeleton, expected_value](DBusMessage* msg)
        {
            auto reply = dbus::Message::make_method_return(msg);
            reply->writer() << expected_value;
            bus->send(reply->get());
            skeleton->emit_signal<test::Service::Signals::Dummy, int64_t>(expected_value);
        });
        cross_process_sync.signal_ready();
        bus->run();
    };
    auto parent = [expected_value, cross_process_sync]()
    {
        auto bus = the_session_bus();
        bus->install_executor(dbus::Executor::Ptr{new dbus::asio::Executor{bus}});
        std::thread t{[bus](){bus->run();}};
        
        cross_process_sync.wait_for_signal_ready();

        auto stub_service = dbus::Service::use_service(bus, dbus::traits::Service<test::Service>::interface_name());
        auto stub = stub_service->object_for_path(dbus::types::ObjectPath("/this/is/unlikely/to/exist/Service"));
        auto signal = stub->get_signal<test::Service::Signals::Dummy>();
        int64_t received_signal_value = -1;
        signal->connect([bus, &received_signal_value](const int32_t& value)
        {
            received_signal_value = value;
            bus->stop();
        });
        auto result = stub->invoke_method_synchronously<test::Service::Method, int32_t>();

        if (t.joinable())
            t.join();

        ASSERT_FALSE(result.is_error());
        ASSERT_EQ(expected_value, result.value());
        EXPECT_EQ(expected_value, received_signal_value);
    };

    EXPECT_NO_FATAL_FAILURE(test::fork_and_run(child, parent));
}

TEST(Bus, TimeoutThrowsForNullDBusWatch)
{
    boost::asio::io_service io_service;
    EXPECT_ANY_THROW(org::freedesktop::dbus::asio::Executor::Timeout<> timeout(io_service, nullptr););
}

namespace
{
struct TimeoutHelper
{
    TimeoutHelper(const std::chrono::milliseconds& timeout, bool enabled = true)
        : timeout(timeout),
          work(reactor),
          invoked(false),
          enabled(enabled)
    {
    }

    void notify_invocation()
    {
        invoked = true;
        reactor.stop();
    }

    bool has_been_invoked() const
    {
        return invoked;
    }

    std::chrono::milliseconds timeout;
    boost::asio::io_service reactor;
    boost::asio::io_service::work work;
    bool invoked;
    bool enabled;
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
struct Timeout<TimeoutHelper>
{
    typedef int DurationType;

    static inline bool is_timeout_enabled(TimeoutHelper* helper)
    {
        return helper->enabled;
    }

    static inline DurationType get_timeout_interval(TimeoutHelper* helper)
    {
        return helper->timeout.count();
    }

    static inline void invoke_timeout_handler(TimeoutHelper* helper)
    {
        helper->notify_invocation();
    }
};
}
}
}
}

TEST(Executor, TimeoutHandlerIsInvokedForEnabledDBusTimeout)
{
    static const std::chrono::seconds test_timeout
    {
        2
    };
    static const std::chrono::milliseconds expected_timeout
    {
        100
    };

    ASSERT_GT(test_timeout, expected_timeout);

    TimeoutHelper helper {expected_timeout};

    boost::asio::deadline_timer test_timer(helper.reactor);
    test_timer.expires_from_now(boost::posix_time::seconds(test_timeout.count()));
    test_timer.async_wait([&](const boost::system::error_code&)
    {
        helper.reactor.stop();
    });
    auto to = std::make_shared<org::freedesktop::dbus::asio::Executor::Timeout<TimeoutHelper>>(helper.reactor, std::addressof(helper));
    to->start();

    helper.reactor.run();
    EXPECT_TRUE(helper.has_been_invoked());
}

TEST(Bus, TimeoutHandlerIsNotInvokedForDisabledDBusTimeout)
{
    static const std::chrono::milliseconds test_timeout
    {
        200
    };
    static const std::chrono::milliseconds expected_timeout
    {
        100
    };

    ASSERT_GT(test_timeout, expected_timeout);

    static const bool disabled
    {
        false
    };
    TimeoutHelper helper {expected_timeout, disabled};

    boost::asio::deadline_timer test_timer(helper.reactor);
    test_timer.expires_from_now(boost::posix_time::milliseconds(test_timeout.count()));
    test_timer.async_wait([&](const boost::system::error_code&)
    {
        helper.reactor.stop();
    });
    auto to = std::make_shared<org::freedesktop::dbus::asio::Executor::Timeout<TimeoutHelper>>(helper.reactor, std::addressof(helper));
    to->start();
    helper.reactor.run();

    EXPECT_FALSE(helper.has_been_invoked());
}

TEST(Bus, TimeoutHandlerIsNotInvokedForEnabledButCancelledDBusTimeout)
{
    static const std::chrono::milliseconds test_timeout
    {
        200
    };
    static const std::chrono::milliseconds expected_timeout
    {
        100
    };

    ASSERT_GT(test_timeout, expected_timeout);

    TimeoutHelper helper {expected_timeout};

    boost::asio::deadline_timer test_timer(helper.reactor);
    test_timer.expires_from_now(boost::posix_time::milliseconds(test_timeout.count()));
    test_timer.async_wait([&](const boost::system::error_code&)
    {
        helper.reactor.stop();
    });
    auto to = std::make_shared<org::freedesktop::dbus::asio::Executor::Timeout<TimeoutHelper>>(helper.reactor, std::addressof(helper));
    to->start();
    helper.reactor.post([&]()
    {
        to->cancel();
    });
    helper.reactor.run();

    EXPECT_FALSE(helper.has_been_invoked());
}

#include <sys/eventfd.h>

namespace
{
struct WatchHelper
{
    inline static int readable_event() { return 0; };
    inline static int writeable_event() { return 1; };
    inline static int error_event() { return 2; };
    inline static int hangup_event() { return 3; };

    static const unsigned int initial_value = 0;
    static const int flags = 0;

    WatchHelper(std::function<void(int, WatchHelper&)> handler)
        : fd(eventfd(initial_value, flags)),
          invocation_handler(handler),
          invocation_cache(4, false),
          work(reactor)
    {
    }

    ~WatchHelper()
    {
        close(fd);
    }

    bool invoke_for_event(int event)
    {
        invocation_cache[event] = true;
        if (has_been_invoked_for(readable_event()) &&
            has_been_invoked_for(writeable_event()))
            reactor.stop();
        invocation_handler(event, *this);
        return true;
    }

    bool has_been_invoked_for(int event)
    {
        return invocation_cache[event];
    }

    int fd;
    std::function<void(int, WatchHelper&)> invocation_handler;
    std::vector<bool> invocation_cache;
    boost::asio::io_service reactor;
    boost::asio::io_service::work work;
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
struct Watch<WatchHelper>
{
    inline static int readable_event() { return WatchHelper::readable_event(); };
    inline static int writeable_event() { return WatchHelper::writeable_event(); };
    inline static int error_event() { return WatchHelper::error_event(); };
    inline static int hangup_event() { return WatchHelper::hangup_event(); };

    static int get_watch_unix_fd(WatchHelper* watch)
    {
        return watch->fd;
    }

    static bool is_watch_monitoring_fd_for_readable(WatchHelper*)
    {
        return true;
    }
    static bool is_watch_monitoring_fd_for_writable(WatchHelper*)
    {
        return true;
    }

    static bool invoke_watch_handler_for_event(WatchHelper* watch, int event)
    {
        return watch->invoke_for_event(event);
    }
};
}
}
}
}

TEST(Executor, WatchHandlerIsInvokedForReadableAndWritableEvents)
{
    WatchHelper helper([&](int, WatchHelper& wh)
    {
        static const int64_t make_readable = std::numeric_limits<int64_t>::max();
        write(wh.fd, std::addressof(make_readable), sizeof(make_readable));
    });

    auto watch = std::make_shared<org::freedesktop::dbus::asio::Executor::Watch<WatchHelper>>(helper.reactor, std::addressof(helper));

    helper.reactor.post([&]()
    {
        int64_t value = 0;
        read(helper.fd, std::addressof(value), sizeof(value));
    });
    watch->start();
    helper.reactor.run();

    EXPECT_TRUE(helper.has_been_invoked_for(WatchHelper::readable_event()));
    EXPECT_TRUE(helper.has_been_invoked_for(WatchHelper::writeable_event()));
}
