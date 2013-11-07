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

#include "org/freedesktop/dbus/dbus.h"
#include "org/freedesktop/dbus/object.h"
#include "org/freedesktop/dbus/property.h"
#include "org/freedesktop/dbus/service.h"
#include "org/freedesktop/dbus/interfaces/properties.h"
#include "org/freedesktop/dbus/types/stl/tuple.h"
#include "org/freedesktop/dbus/types/stl/vector.h"

#include "org/freedesktop/dbus/asio/executor.h"

#include "cross_process_sync.h"
#include "fork_and_run.h"
#include "test_service.h"

#include <gtest/gtest.h>

#include <thread>

namespace dbus = org::freedesktop::dbus;

namespace
{
dbus::Bus::Ptr the_session_bus()
{
    dbus::Bus::Ptr session_bus = std::make_shared<dbus::Bus>(dbus::WellKnownBus::session);
    return session_bus;
}
}

TEST(Service, AccessingAnExistingServiceAndItsObjectsOnTheBusWorks)
{
    auto bus = the_session_bus();
    auto dbus = dbus::Service::use_service<dbus::DBus>(bus);
    auto dbus_object = dbus->object_for_path(dbus::types::ObjectPath(DBUS_PATH_DBUS));

    auto names = dbus_object->invoke_method_synchronously<dbus::DBus::ListNames, std::vector<std::string>>();

    ASSERT_GT(names.value().size(), 0);
}

TEST(Service, AddingServiceAndObjectAndCallingIntoItSucceeds)
{
    test::CrossProcessSync cross_process_sync;

    const int64_t expected_value = 42;
    auto child = [expected_value, &cross_process_sync]()
    {
        auto bus = the_session_bus();
        bus->install_executor(org::freedesktop::dbus::asio::make_executor(bus));
        auto service = dbus::Service::add_service<test::Service>(bus);
        auto skeleton = service->add_object_for_path(dbus::types::ObjectPath("/this/is/unlikely/to/exist/Service"));
        auto signal = skeleton->get_signal<test::Service::Signals::Dummy>();
        auto writable_property = skeleton->get_property<test::Service::Properties::Dummy>();
        writable_property->value(expected_value);
        skeleton->install_method_handler<test::Service::Method>([bus, skeleton, expected_value](DBusMessage* msg)
        {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
            auto reply = dbus::Message::make_method_return(msg);
            reply->writer() << expected_value;
            bus->send(reply->get());
            skeleton->emit_signal<test::Service::Signals::Dummy, int64_t>(expected_value);
        });
        std::thread t{[bus](){ bus->run(); }};
        cross_process_sync.signal_ready();
        if (t.joinable())
            t.join();
    };
    auto parent = [expected_value, cross_process_sync]()
    {
        auto bus = the_session_bus();
        bus->install_executor(org::freedesktop::dbus::asio::make_executor(bus));
        std::thread t{[bus](){ bus->run(); }};
        cross_process_sync.wait_for_signal_ready();

        auto stub_service = dbus::Service::use_service(bus, dbus::traits::Service<test::Service>::interface_name());
        auto stub = stub_service->object_for_path(dbus::types::ObjectPath("/this/is/unlikely/to/exist/Service"));
        auto writable_property = stub->get_property<test::Service::Properties::Dummy>();
        auto signal = stub->get_signal<test::Service::Signals::Dummy>();
        int64_t received_signal_value = -1;
        signal->connect([bus, &received_signal_value](const int32_t& value)
        {
            std::cout << value << std::endl;
            received_signal_value = value;
            bus->stop();
        });
        auto result = stub->invoke_method_synchronously<test::Service::Method, int32_t>();
        ASSERT_FALSE(result.is_error());
        ASSERT_EQ(expected_value, result.value());
        ASSERT_EQ(expected_value, writable_property->value());
        ASSERT_NO_THROW(writable_property->value(4242));
        ASSERT_EQ(4242, writable_property->value());

        if (t.joinable())
            t.join();

        EXPECT_EQ(expected_value, received_signal_value);
    };

    EXPECT_NO_FATAL_FAILURE(test::fork_and_run(child, parent));
}

TEST(Service, DefaultRequestNameFlagsEnforceReplacingExistingService)
{
    auto flags = dbus::Service::default_request_name_flags();
    EXPECT_FALSE(flags.test(dbus::Service::allow_replacement));
    EXPECT_TRUE(flags.test(dbus::Service::replace_existing));
    EXPECT_FALSE(flags.test(dbus::Service::do_not_queue));
}

TEST(Service, AddingANonExistingServiceDoesNotThrow)
{
    auto bus = the_session_bus();
    const std::string service_name
    {
        "very.unlikely.that.this.name.exists"
    };
    ASSERT_NO_THROW(auto service = dbus::Service::add_service<test::Service>(bus););
}

TEST(Service, AddingAnExistingServiceThrowsForSpecificFlags)
{
    auto bus = the_session_bus();
    const std::string service_name
    {
        "org.freedesktop.DBus"
    };
    dbus::Service::RequestNameFlags flags;
    flags.set(dbus::Service::replace_existing, false);
    ASSERT_ANY_THROW(auto service = dbus::Service::add_service<dbus::DBus>(bus, flags););
}

TEST(VoidResult, DefaultConstructionYieldsANonErrorResult)
{
    dbus::Result<void> result;
    EXPECT_FALSE(result.is_error());
}

TEST(VoidResult, FromMethodCallYieldsException)
{
    auto msg = dbus::Message::make_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_SERVICE_DBUS, "ListNames");
    dbus::Result<void> result;

    EXPECT_ANY_THROW(result.from_message(msg->get()));
}

TEST(VoidResult, FromErrorYieldsError)
{
    const std::string error_name = "does.not.exist.MyError";
    const std::string error_description = "MyErrorDescription";

    auto msg = dbus::Message::make_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_SERVICE_DBUS, "ListNames");
    dbus_message_set_serial(msg->get(), 1);
    auto error_reply = dbus::Message::make_error(msg->get(), error_name, error_description);
    dbus::Result<void> result = dbus::Result<void>::from_message(error_reply->get());
    EXPECT_TRUE(result.is_error());
    EXPECT_EQ(error_name + ": " + error_description, result.error().print());
}

TEST(VoidResult, FromNonEmptyMethodReturnYieldsException)
{
    auto msg = dbus::Message::make_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_SERVICE_DBUS, "ListNames");
    dbus_message_set_serial(msg->get(), 1);
    auto reply = dbus::Message::make_method_return(msg->get());
    reply->writer() << 42;

    dbus::Result<void> result;

    EXPECT_NO_THROW(result.from_message(reply->get()));
}

TEST(NonVoidResult, DefaultConstructionYieldsANonErrorResult)
{
    dbus::Result<std::tuple<double, double>> result;
    EXPECT_FALSE(result.is_error());
}

TEST(NonVoidResult, FromMethodCallYieldsException)
{
    auto msg = dbus::Message::make_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_SERVICE_DBUS, "ListNames");
    dbus::Result<int32_t> result;

    EXPECT_ANY_THROW(result.from_message(msg->get()));
}

TEST(NonVoidResult, FromErrorYieldsError)
{
    const std::string error_name = "does.not.exist.MyError";
    const std::string error_description = "MyErrorDescription";

    auto msg = dbus::Message::make_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_SERVICE_DBUS, "ListNames");
    dbus_message_set_serial(msg->get(), 1);
    auto error_reply = dbus::Message::make_error(msg->get(), error_name, error_description);
    auto result = dbus::Result<int32_t>::from_message(error_reply->get());

    EXPECT_TRUE(result.is_error());
    EXPECT_EQ(error_name + ": " + error_description, result.error().print());
}

TEST(NonVoidResult, FromEmptyMethodReturnYieldsException)
{
    auto msg = dbus::Message::make_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_SERVICE_DBUS, "ListNames");
    dbus_message_set_serial(msg->get(), 1);
    auto reply = dbus::Message::make_method_return(msg->get());

    dbus::Result<int32_t> result;

    EXPECT_ANY_THROW(result.from_message(reply->get()));
}
