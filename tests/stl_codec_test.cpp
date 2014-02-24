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
#include <core/dbus/message_streaming_operators.h>

#include <core/dbus/types/variant.h>

#include <core/dbus/types/stl/map.h>
#include <core/dbus/types/stl/string.h>
#include <core/dbus/types/stl/tuple.h>

#include <gtest/gtest.h>

#include <memory>

namespace dbus = core::dbus;

namespace
{
std::shared_ptr<core::dbus::Message> a_method_call()
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
    core::dbus::Codec<TupleType>::encode_argument(writer, t1);
    EXPECT_EQ(msg->signature(),
              core::dbus::helper::TypeMapper<TupleType>::signature());
    TupleType t2;
    auto reader = msg->reader();
    core::dbus::Codec<TupleType>::decode_argument(reader, t2);

    EXPECT_EQ(std::get<0>(t1), std::get<0>(t2));
    EXPECT_EQ(std::get<1>(t1), std::get<1>(t2));
}

TEST(CodecForMaps, DictionaryMappingToVariantsIsEncodedAndDecodedCorrectly)
{
    namespace dbus = core::dbus;

    auto msg = a_method_call();

    {
        auto writer = msg->writer();
        auto array = writer.open_array(dbus::types::Signature{"{sv}"});
        for(unsigned int i = 0; i < 5; i++)
        {
            auto entry = array.open_dict_entry();
            {
                auto key = std::to_string(i);
                entry.push_stringn(key.c_str(), key.size());
                auto variant = entry.open_variant(dbus::types::Signature{dbus::helper::TypeMapper<std::uint32_t>::signature()});
                {
                    variant.push_uint32(i);

                } entry.close_variant(std::move(variant));
            } array.close_dict_entry(std::move(entry));
        } writer.close_array(std::move(array));
    }

    unsigned int counter = 0;

    std::map<std::string, dbus::types::Variant> result;
    msg->reader() >> result;

    EXPECT_EQ(std::uint32_t(5), result.size());

    for (const auto& element : result)
    {
        EXPECT_EQ(std::to_string(counter), element.first);
        EXPECT_EQ(counter, element.second.as<std::uint32_t>());

        counter++;
    }
}

