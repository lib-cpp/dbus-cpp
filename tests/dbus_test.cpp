#include "org/freedesktop/dbus/dbus.h"

#include "org/freedesktop/dbus/asio/executor.h"

#include "test_service.h"
#include "cross_process_sync.h"
#include "fork_and_run.h"

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

TEST(DBus, QueryingUnixProcessIdReturnsCorrectResult)
{
    const std::string path{"/this/is/just/a/test/service"};

    auto pid = getpid();
    auto uid = getuid();

    test::CrossProcessSync barrier;

    auto child = [path, pid, uid, &barrier]()
    {
        auto bus = the_session_bus();
        bus->install_executor(
            org::freedesktop::dbus::Executor::Ptr(new org::freedesktop::dbus::asio::Executor{bus}));
        dbus::DBus daemon{bus};

        auto service = dbus::Service::add_service<test::Service>(bus);
        auto object = service->add_object_for_path(dbus::types::ObjectPath{path});

        uint32_t sender_pid = 0, sender_uid = 0;

        auto handler = [&daemon, &sender_pid, &sender_uid, bus](DBusMessage* msg)
        {
            auto tmp = dbus::Message::from_raw_message(msg); 
            auto sender = tmp->sender();
            sender_pid = daemon.get_connection_unix_process_id(sender);
            sender_uid = daemon.get_connection_unix_user(sender);

            auto reply = dbus::Message::make_method_return(msg);
            bus->send(reply->get());
            bus->stop();
        };

        object->install_method_handler<test::Service::Method>(handler);
        barrier.signal_ready();
        bus->run();

        EXPECT_EQ(pid, sender_pid);
        EXPECT_EQ(uid, sender_uid);
    };

    auto parent = [path, &barrier]()
    {
        auto bus = the_session_bus();
        
        auto service = dbus::Service::use_service<test::Service>(bus);
        auto object = service->object_for_path(dbus::types::ObjectPath{path});

        barrier.wait_for_signal_ready();

        object->invoke_method_synchronously<test::Service::Method, void>();        
    };

    ASSERT_NO_FATAL_FAILURE(test::fork_and_run(child, parent));
}
