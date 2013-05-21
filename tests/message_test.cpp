#include "org/freedesktop/dbus/message.h"

#include <gtest/gtest.h>

#include <chrono>
#include <memory>

TEST(Message, BuildingAMethodCallMessageSucceedsForValidArguments)
{
    const std::string destination = DBUS_SERVICE_DBUS;
    const std::string path = DBUS_PATH_DBUS;
    const std::string interface = DBUS_SERVICE_DBUS;
    const std::string member = "ListNames";

    std::shared_ptr<org::freedesktop::dbus::Message> msg;
    EXPECT_NO_THROW(msg = org::freedesktop::dbus::Message::make_method_call(destination, path, interface, member););
    EXPECT_NE(nullptr, msg.get());
}

TEST(Message, BuildingAMethodCallMessageThrowsForInvalidArguments)
{
    const std::string destination = DBUS_SERVICE_DBUS;
    const std::string path = "an:invalid:path";
    const std::string interface = DBUS_SERVICE_DBUS;
    const std::string member = "ListNames";

    std::shared_ptr<org::freedesktop::dbus::Message> msg;
    EXPECT_ANY_THROW(msg = org::freedesktop::dbus::Message::make_method_call(destination, path, interface, member););
    EXPECT_EQ(nullptr, msg.get());
}

TEST(Message, AccessingAReaderOnAnEmptyMessageThrows)
{
    const std::string destination = DBUS_SERVICE_DBUS;
    const std::string path = DBUS_PATH_DBUS;
    const std::string interface = DBUS_SERVICE_DBUS;
    const std::string member = "ListNames";

    auto msg = org::freedesktop::dbus::Message::make_method_call(destination, path, interface, member);

    EXPECT_ANY_THROW(msg->reader());
}

TEST(Message, AccessingAWriterOnAnyMessageSucceeds)
{
    const std::string destination = DBUS_SERVICE_DBUS;
    const std::string path = DBUS_PATH_DBUS;
    const std::string interface = DBUS_SERVICE_DBUS;
    const std::string member = "ListNames";

    auto msg = org::freedesktop::dbus::Message::make_method_call(destination, path, interface, member);

    EXPECT_NO_THROW(auto writer = msg->writer());

    {
        auto writer = msg->writer();
        writer << 43 << 42.;
    }

    EXPECT_NO_THROW(auto writer = msg->writer(););
}

TEST(Message, WriteAndSuccessiveReadAreIdempotent)
{
    const std::string destination = DBUS_SERVICE_DBUS;
    const std::string path = DBUS_PATH_DBUS;
    const std::string interface = DBUS_SERVICE_DBUS;
    const std::string member = "ListNames";

    auto msg = org::freedesktop::dbus::Message::make_method_call(destination, path, interface, member);

    const unsigned int expected_integer_value
    {
        43
    };
    const double expected_floating_point_value
    {
        42.
    };

    {
        auto writer = msg->writer();
        writer << expected_integer_value << expected_floating_point_value;
    }

    unsigned int i;
    double d;
    auto reader = msg->reader();
    reader >> i >> d;

    EXPECT_EQ(expected_integer_value, i);
    EXPECT_EQ(expected_floating_point_value, d);
}
