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

#include "org/freedesktop/dbus/types/stl/tuple.h"

#include <gtest/gtest.h>

#include <memory>

namespace
{
std::shared_ptr<DBusMessage> a_method_call()
{
    return std::shared_ptr<DBusMessage>(
               dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_SERVICE_DBUS, "ListNames"),
               [](DBusMessage* msg)
    {
        dbus_message_unref(msg);
    });
}
}

TEST(CodecForTuple, encoding_of_tuples_works)
{
    typedef std::tuple<int, int> TupleType;
    auto msg = a_method_call();
    DBusMessageIter iter;
    dbus_message_iter_init_append(msg.get(), std::addressof(iter));
    TupleType t1(42, 42);
    org::freedesktop::dbus::Codec<TupleType>::encode_argument(std::addressof(iter), t1);
    EXPECT_STREQ(dbus_message_get_signature(msg.get()), org::freedesktop::dbus::helper::TypeMapper<TupleType>::signature().c_str());
    TupleType t2;
    dbus_message_iter_init(msg.get(), std::addressof(iter));
    org::freedesktop::dbus::Codec<TupleType>::decode_argument(std::addressof(iter), t2);

    EXPECT_EQ(std::get<0>(t1), std::get<0>(t2));
    EXPECT_EQ(std::get<1>(t1), std::get<1>(t2));
}
