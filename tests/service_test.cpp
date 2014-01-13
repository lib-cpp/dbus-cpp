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

#include <core/dbus/dbus.h>
#include <core/dbus/fixture.h>
#include <core/dbus/object.h>
#include <core/dbus/property.h>
#include <core/dbus/service.h>
#include <core/dbus/interfaces/properties.h>
#include <core/dbus/types/stl/tuple.h>
#include <core/dbus/types/stl/vector.h>

#include <core/dbus/asio/executor.h>

#include "test_data.h"
#include "test_service.h"

#include <core/testing/cross_process_sync.h>
#include <core/testing/fork_and_run.h>

#include <gtest/gtest.h>

#include <thread>

namespace dbus = core::dbus;

namespace
{
struct Service : public core::dbus::testing::Fixture
{
};

auto session_bus_config_file =
        core::dbus::testing::Fixture::default_session_bus_config_file() =
        core::testing::session_bus_configuration_file();

auto system_bus_config_file =
        core::dbus::testing::Fixture::default_system_bus_config_file() =
        core::testing::system_bus_configuration_file();
}

TEST_F(Service, AccessingAnExistingServiceAndItsObjectsOnTheBusWorks)
{
    auto bus = session_bus();
    auto names = dbus::DBus(bus).list_names();

    ASSERT_GT(names.size(), std::size_t{0});
}

TEST_F(Service, AddingServiceAndObjectAndCallingIntoItSucceeds)
{
        core::testing::CrossProcessSync cps1;

        const int64_t expected_value = 42;

        auto service = [this, expected_value, &cps1]()
        {
            auto bus = session_bus();
            bus->install_executor(core::dbus::asio::make_executor(bus));
            auto service = dbus::Service::add_service<test::Service>(bus);
            auto skeleton = service->add_object_for_path(dbus::types::ObjectPath("/this/is/unlikely/to/exist/Service"));
            auto signal = skeleton->get_signal<test::Service::Signals::Dummy>();
            auto writable_property = skeleton->get_property<test::Service::Properties::Dummy>();
            writable_property->set(expected_value);

            skeleton->install_method_handler<test::Service::Method>([bus, skeleton, expected_value](const dbus::Message::Ptr& msg)
            {
                auto reply = dbus::Message::make_method_return(msg);
                reply->writer() << expected_value;
                bus->send(reply);
                skeleton->emit_signal<test::Service::Signals::Dummy, int64_t>(expected_value);
            });

            std::thread t{[bus](){ bus->run(); }};
            cps1.try_signal_ready_for(std::chrono::milliseconds{500});
            if (t.joinable())
                t.join();

            return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
        };

        auto client = [this, expected_value, &cps1]()
        {
            auto bus = session_bus();
            bus->install_executor(core::dbus::asio::make_executor(bus));
            std::thread t{[bus](){ bus->run(); }};
            EXPECT_EQ(1, cps1.wait_for_signal_ready_for(std::chrono::milliseconds{500}));

            auto stub_service = dbus::Service::use_service(bus, dbus::traits::Service<test::Service>::interface_name());
            auto stub = stub_service->object_for_path(dbus::types::ObjectPath("/this/is/unlikely/to/exist/Service"));
            auto writable_property = stub->get_property<test::Service::Properties::Dummy>();
            writable_property->changed().connect([](double d)
            {
                std::cout << "Dummy property changed: " << d << std::endl;
            });
            auto signal = stub->get_signal<test::Service::Signals::Dummy>();
            int64_t received_signal_value = -1;
            signal->connect([bus, &received_signal_value](const int32_t& value)
            {
                received_signal_value = value;
                bus->stop();
            });
            auto result = stub->invoke_method_synchronously<test::Service::Method, int64_t>();
            EXPECT_FALSE(result.is_error());
            EXPECT_EQ(expected_value, result.value());
            EXPECT_EQ(expected_value, writable_property->get());
            EXPECT_NO_THROW(writable_property->set(4242));
            EXPECT_EQ(4242, writable_property->get());

            if (t.joinable())
                t.join();

            EXPECT_EQ(expected_value, received_signal_value);

            return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
        };

        EXPECT_NO_FATAL_FAILURE(core::testing::fork_and_run(service, client));
}

TEST_F(Service, AddingANonExistingServiceDoesNotThrow)
{
    const std::string service_name
    {
        "very.unlikely.that.this.name.exists"
    };
    ASSERT_NO_THROW(auto service = dbus::Service::add_service<test::Service>(session_bus()););
}

TEST_F(Service, AddingAnExistingServiceThrowsForSpecificFlags)
{
    const std::string service_name
    {
        "org.freedesktop.DBus"
    };
    dbus::Bus::RequestNameFlag flags{dbus::Bus::RequestNameFlag::not_set};
    ASSERT_ANY_THROW(auto service = dbus::Service::add_service<dbus::DBus>(session_bus(), flags););
}

TEST(VoidResult, DefaultConstructionYieldsANonErrorResult)
{
    dbus::Result<void> result;
    EXPECT_FALSE(result.is_error());
}

TEST(VoidResult, FromMethodCallYieldsException)
{
    auto msg = dbus::Message::make_method_call(
                dbus::DBus::name(),
                dbus::DBus::path(),
                dbus::DBus::interface(),
                "ListNames");
    dbus::Result<void> result;

    EXPECT_ANY_THROW(result.from_message(msg));
}

TEST(VoidResult, FromErrorYieldsError)
{
    const std::string error_name = "does.not.exist.MyError";
    const std::string error_description = "MyErrorDescription";

    auto msg = dbus::Message::make_method_call(
                dbus::DBus::name(),
                dbus::DBus::path(),
                dbus::DBus::interface(),
                "ListNames");

    msg->ensure_serial_larger_than_zero_for_testing();
    auto error_reply = dbus::Message::make_error(msg, error_name, error_description);
    dbus::Result<void> result = dbus::Result<void>::from_message(error_reply);
    EXPECT_TRUE(result.is_error());
    EXPECT_EQ(error_name + ": " + error_description, result.error().print());
}

TEST(VoidResult, FromNonEmptyMethodReturnYieldsException)
{
    auto msg = dbus::Message::make_method_call(
                dbus::DBus::name(),
                dbus::DBus::path(),
                dbus::DBus::interface(),
                "ListNames");

    msg->ensure_serial_larger_than_zero_for_testing();
    auto reply = dbus::Message::make_method_return(msg);
    reply->writer() << 42;

    dbus::Result<void> result;

    EXPECT_NO_THROW(result.from_message(reply));
}

TEST(NonVoidResult, DefaultConstructionYieldsANonErrorResult)
{
    dbus::Result<std::tuple<double, double>> result;
    EXPECT_FALSE(result.is_error());
}

TEST(NonVoidResult, FromMethodCallYieldsException)
{
    auto msg = dbus::Message::make_method_call(
                dbus::DBus::name(),
                dbus::DBus::path(),
                dbus::DBus::interface(),
                "ListNames");
    dbus::Result<int32_t> result;

    EXPECT_ANY_THROW(result.from_message(msg));
}

TEST(NonVoidResult, FromErrorYieldsError)
{
    const std::string error_name = "does.not.exist.MyError";
    const std::string error_description = "MyErrorDescription";

    auto msg = dbus::Message::make_method_call(
                dbus::DBus::name(),
                dbus::DBus::path(),
                dbus::DBus::interface(),
                "ListNames");

    msg->ensure_serial_larger_than_zero_for_testing();
    auto error_reply = dbus::Message::make_error(msg, error_name, error_description);
    auto result = dbus::Result<int32_t>::from_message(error_reply);

    EXPECT_TRUE(result.is_error());
    EXPECT_EQ(error_name + ": " + error_description, result.error().print());
}

TEST(NonVoidResult, FromEmptyMethodReturnYieldsException)
{
    auto msg = dbus::Message::make_method_call(
                dbus::DBus::name(),
                dbus::DBus::path(),
                dbus::DBus::interface(),
                "ListNames");

    msg->ensure_serial_larger_than_zero_for_testing();
    auto reply = dbus::Message::make_method_return(msg);

    dbus::Result<int32_t> result;

    EXPECT_ANY_THROW(result.from_message(reply));
}
