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
#include "org/freedesktop/dbus/types/stl/tuple.h"

#include <gtest/gtest.h>

#include <memory>

namespace dbus = org::freedesktop::dbus;

namespace
{
std::shared_ptr<org::freedesktop::dbus::Message> a_method_call()
{
    return dbus::Message::make_method_call(
                dbus::DBus::name(),
                dbus::DBus::path(),
                dbus::DBus::interface(),
                "ListNames");
}
}

TEST(CodecForTuple, encoding_of_tuples_works)
{
    typedef std::tuple<int, int> TupleType;
    auto msg = a_method_call();
    auto writer = msg->writer();
    TupleType t1(42, 42);
    org::freedesktop::dbus::Codec<TupleType>::encode_argument(writer, t1);
    EXPECT_EQ(msg->signature(),
              org::freedesktop::dbus::helper::TypeMapper<TupleType>::signature());
    TupleType t2;
    auto reader = msg->reader();
    org::freedesktop::dbus::Codec<TupleType>::decode_argument(reader, t2);

    EXPECT_EQ(std::get<0>(t1), std::get<0>(t2));
    EXPECT_EQ(std::get<1>(t1), std::get<1>(t2));
}
