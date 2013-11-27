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

#include <org/freedesktop/dbus/message.h>

#include <gtest/gtest.h>

#include <chrono>
#include <memory>

namespace dbus = core::dbus;

TEST(Message, BuildingAMethodCallMessageSucceedsForValidArguments)
{
    const std::string destination = DBUS_SERVICE_DBUS;
    const std::string path = DBUS_PATH_DBUS;
    const std::string interface = DBUS_SERVICE_DBUS;
    const std::string member = "ListNames";

    std::shared_ptr<core::dbus::Message> msg;
    EXPECT_NO_THROW(msg = core::dbus::Message::make_method_call(destination, path, interface, member););
    EXPECT_NE(nullptr, msg.get());
}

TEST(Message, BuildingAMethodCallMessageThrowsForInvalidArguments)
{
    const std::string destination = DBUS_SERVICE_DBUS;
    const std::string path = "an:invalid:path";
    const std::string interface = DBUS_SERVICE_DBUS;
    const std::string member = "ListNames";

    std::shared_ptr<core::dbus::Message> msg;
    EXPECT_ANY_THROW(msg = core::dbus::Message::make_method_call(destination, path, interface, member););
    EXPECT_EQ(nullptr, msg.get());
}

TEST(Message, AccessingAReaderOnAnEmptyMessageThrows)
{
    const std::string destination = DBUS_SERVICE_DBUS;
    const std::string path = DBUS_PATH_DBUS;
    const std::string interface = DBUS_SERVICE_DBUS;
    const std::string member = "ListNames";

    auto msg = core::dbus::Message::make_method_call(destination, path, interface, member);

    EXPECT_ANY_THROW(msg->reader());
}

TEST(Message, AccessingAWriterOnAnyMessageSucceeds)
{
    const std::string destination = DBUS_SERVICE_DBUS;
    const std::string path = DBUS_PATH_DBUS;
    const std::string interface = DBUS_SERVICE_DBUS;
    const std::string member = "ListNames";

    auto msg = core::dbus::Message::make_method_call(
                destination,
                path,
                interface,
                member);

    EXPECT_NO_THROW(auto writer = msg->writer());

    {
        msg->writer().push_int16(43);
        msg->writer().push_int16(42);
    }

    EXPECT_NO_THROW(auto writer = msg->writer(););
}

TEST(Message, WriteAndSuccessiveReadAreIdempotent)
{
    const std::string destination = DBUS_SERVICE_DBUS;
    const std::string path = DBUS_PATH_DBUS;
    const std::string interface = DBUS_SERVICE_DBUS;
    const std::string member = "ListNames";

    auto msg = core::dbus::Message::make_method_call(
                destination,
                path,
                interface,
                member);

    const int32_t expected_integer_value
    {
        43
    };
    const double expected_floating_point_value
    {
        42.
    };

    {
        auto writer = msg->writer();
        writer.push_int32(expected_integer_value);
        writer.push_floating_point(expected_floating_point_value);
    }

    auto reader = msg->reader();
    auto i = reader.pop_int32();
    auto d = reader.pop_floating_point();

    EXPECT_EQ(expected_integer_value, i);
    EXPECT_EQ(expected_floating_point_value, d);
}

TEST(Message, WriteAndSuccessiveIterationAreIdempotent)
{
    const std::string destination = DBUS_SERVICE_DBUS;
    const std::string path = DBUS_PATH_DBUS;
    const std::string interface = DBUS_SERVICE_DBUS;
    const std::string member = "ListNames";

    auto msg = core::dbus::Message::make_method_call(
                destination,
                path,
                interface,
                member);

    const std::int32_t expected_integer_value
    {
        43
    };
    const double expected_floating_point_value
    {
        42.
    };

    {
        auto writer = msg->writer();
        writer.push_int32(expected_integer_value);
        writer.push_floating_point(expected_floating_point_value);
        auto vw = writer.open_variant(dbus::types::Signature("(id)"));
        {
            auto sw = vw.open_structure();
            {
                sw.push_int32(expected_integer_value);
                sw.push_floating_point(expected_floating_point_value);
            }
            vw.close_structure(std::move(sw));
        }
        writer.close_variant(std::move(vw));
    }

    auto reader = msg->reader();
    EXPECT_EQ(dbus::ArgumentType::int32, reader.type());
    EXPECT_NO_THROW(reader.pop());
    EXPECT_EQ(dbus::ArgumentType::floating_point, reader.type());
    EXPECT_NO_THROW(reader.pop());
    auto vr = reader.pop_variant();
    {
        auto sr = vr.pop_structure();
        {
            EXPECT_EQ(dbus::ArgumentType::int32, sr.type());
            EXPECT_EQ(expected_integer_value, sr.pop_int32());
            EXPECT_EQ(dbus::ArgumentType::floating_point, sr.type());
            EXPECT_EQ(expected_floating_point_value, sr.pop_floating_point());
        }
    }
}
