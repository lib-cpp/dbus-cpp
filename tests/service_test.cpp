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

#include "sig_term_catcher.h"
#include "test_data.h"
#include "test_service.h"

#include <core/testing/cross_process_sync.h>
#include <core/testing/fork_and_run.h>

#include <gtest/gtest.h>

#include <system_error>
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
            core::testing::SigTermCatcher sc;

            auto bus = session_bus();
            bus->install_executor(core::dbus::asio::make_executor(bus));
            auto service = dbus::Service::add_service<test::Service>(bus);
            auto skeleton = service->add_object_for_path(dbus::types::ObjectPath("/this/is/unlikely/to/exist/Service"));
            auto signal = skeleton->get_signal<test::Service::Signals::Dummy>();
            auto writable_property = skeleton->get_property<test::Service::Properties::Dummy>();
            writable_property->set(expected_value);

            auto readonly_property = skeleton->get_property<test::Service::Properties::ReadOnly>();
            readonly_property->set(7);

            skeleton->install_method_handler<test::Service::Method>([bus, skeleton, &readonly_property, expected_value](const dbus::Message::Ptr& msg)
            {
                auto reply = dbus::Message::make_method_return(msg);
                reply->writer() << expected_value;
                bus->send(reply);

                readonly_property->set(expected_value);
                auto changed_signal = skeleton->get_signal<core::dbus::interfaces::Properties::Signals::PropertiesChanged>();
                core::dbus::interfaces::Properties::Signals::PropertiesChanged::ArgumentType
                        args("this.is.unlikely.to.exist.Service",
                             {{test::Service::Properties::ReadOnly::name(),
                               core::dbus::types::TypedVariant<test::Service::Properties::ReadOnly::ValueType>(expected_value)}},
                             {});
                skeleton->emit_signal<core::dbus::interfaces::Properties::Signals::PropertiesChanged, core::dbus::interfaces::Properties::Signals::PropertiesChanged::ArgumentType>(args);
                changed_signal->emit(args);

                skeleton->emit_signal<test::Service::Signals::Dummy, int64_t>(expected_value);
            });

            std::thread t{[bus](){ bus->run(); }};
            cps1.try_signal_ready_for(std::chrono::milliseconds{500});

            EXPECT_TRUE(sc.wait_for_signal());

            bus->stop();

            if (t.joinable())
                t.join();

            return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
        };

        auto client = [this, expected_value, &cps1]()
        {
            auto bus = session_bus();
            bus->install_executor(core::dbus::asio::make_executor(bus));
            std::thread t{[bus](){ bus->run(); }};
            EXPECT_EQ(std::uint32_t(1), cps1.wait_for_signal_ready_for(std::chrono::milliseconds{500}));

            auto stub_service = dbus::Service::use_service<test::Service>(bus);
            auto stub = stub_service->object_for_path(dbus::types::ObjectPath("/this/is/unlikely/to/exist/Service"));
            EXPECT_EQ(stub->path().as_string(), "/this/is/unlikely/to/exist/Service");
            auto writable_property = stub->get_property<test::Service::Properties::Dummy>();
            writable_property->changed().connect([](double d)
            {
                std::cout << "Dummy property changed: " << d << std::endl;
            });

            auto readonly_property = stub->get_property<test::Service::Properties::ReadOnly>();
            EXPECT_EQ(readonly_property->get(), 7);
            std::uint32_t changed_value = 0;
            readonly_property->changed().connect([&changed_value](std::uint32_t value){
                changed_value = value;
            });

            auto signal = stub->get_signal<test::Service::Signals::Dummy>();
            int64_t received_signal_value = -1;
            signal->connect([bus, &received_signal_value](const int32_t& value)
            {
                received_signal_value = value;
                bus->stop();
            });

            try
            {
                auto result = stub->invoke_method_synchronously<test::Service::Method, int64_t>();
                EXPECT_FALSE(result.is_error());
                EXPECT_EQ(expected_value, result.value());
            } catch(const std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }
            
            EXPECT_EQ(expected_value, writable_property->get());
            EXPECT_NO_THROW(writable_property->set(4242));
            EXPECT_EQ(4242, writable_property->get());

            if (t.joinable())
                t.join();

            EXPECT_EQ(expected_value, received_signal_value);
            EXPECT_EQ(expected_value, readonly_property->get());
            EXPECT_EQ(changed_value, expected_value);

            return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
        };

        EXPECT_EQ(core::testing::ForkAndRunResult::empty, core::testing::fork_and_run(service, client));
}

TEST_F(Service, AddingANonExistingServiceDoesNotThrow)
{
    ASSERT_NO_THROW(auto service = dbus::Service::add_service<test::Service>(session_bus()););
}

namespace
{
struct Dummy
{
    static const std::string& name()
    {
        static std::string s{"A.Dummy.Service"};
        return s;
    }
};
}

// We cache service allocations in-process. For that, we rely on two child processes for testing purposes and
// to check that trying to own a service name for the second name actually throws.
TEST_F(Service, AddingAServiceTwiceThrows)
{
    // p1 --| Done accessing the service |--> p2
    core::testing::CrossProcessSync first_process_acquired_name;
    // p2 --| Done trying to access the service |--> p1
    core::testing::CrossProcessSync second_process_acquired_name;

    // This is the child process that owns the dummy service. It waits for the second process to
    // come up and trying to own the same service on the same bus. Otherwise, the service object
    // would go out of scope in P1 and clean up the owned name.
    auto p1 = core::posix::fork([this, &first_process_acquired_name, &second_process_acquired_name]
    {
        core::dbus::Service::Ptr service;
        EXPECT_NO_THROW(service = dbus::Service::add_service<Dummy>(session_bus()));
        first_process_acquired_name.try_signal_ready_for(std::chrono::seconds{2});
        second_process_acquired_name.wait_for_signal_ready_for(std::chrono::seconds{2});

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    }, core::posix::StandardStream::empty);

    auto p2 = core::posix::fork([this, &first_process_acquired_name, &second_process_acquired_name]
    {
        first_process_acquired_name.wait_for_signal_ready_for(std::chrono::seconds{2});

        core::dbus::Service::Ptr service;
        EXPECT_ANY_THROW(service = dbus::Service::add_service<Dummy>(session_bus()););
        second_process_acquired_name.try_signal_ready_for(std::chrono::seconds{2});

        return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
    }, core::posix::StandardStream::empty);

    core::posix::wait::Result r1 = p1.wait_for(core::posix::wait::Flags::untraced);
    core::posix::wait::Result r2 = p2.wait_for(core::posix::wait::Flags::untraced);

    EXPECT_EQ(core::posix::wait::Result::Status::exited, r1.status);
    EXPECT_EQ(core::posix::exit::Status::success, r1.detail.if_exited.status);

    EXPECT_EQ(core::posix::wait::Result::Status::exited, r2.status);
    EXPECT_EQ(core::posix::exit::Status::success, r2.detail.if_exited.status);
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
