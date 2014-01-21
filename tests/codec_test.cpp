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

#include <core/dbus/codec.h>
#include <core/dbus/dbus.h>
#include <core/dbus/message_streaming_operators.h>
#include <core/dbus/helper/signature.h>

#include <core/dbus/types/any.h>
#include <core/dbus/types/object_path.h>
#include <core/dbus/types/signature.h>
#include <core/dbus/types/struct.h>
#include <core/dbus/types/unix_fd.h>
#include <core/dbus/types/variant.h>

// STL includes
#include <core/dbus/types/stl/list.h>
#include <core/dbus/types/stl/map.h>
#include <core/dbus/types/stl/string.h>
#include <core/dbus/types/stl/tuple.h>
#include <core/dbus/types/stl/vector.h>

#include <gtest/gtest.h>

#include <memory>

namespace dbus = core::dbus;

TEST(Codec, BasicTypesMatchSizeAndAlignOfDBusTypes)
{
    ::testing::StaticAssertTypeEq<dbus_bool_t, typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::boolean>::Type>();
    ::testing::StaticAssertTypeEq<int8_t, typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::byte>::Type>();
    ::testing::StaticAssertTypeEq<int16_t, typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::int16>::Type>();
    ::testing::StaticAssertTypeEq<uint16_t, typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::uint16>::Type>();
    ::testing::StaticAssertTypeEq<int32_t, typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::int32>::Type>();
    ::testing::StaticAssertTypeEq<uint32_t, typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::uint32>::Type>();
    ::testing::StaticAssertTypeEq<int64_t, typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::int64>::Type>();
    ::testing::StaticAssertTypeEq<uint64_t, typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::uint64>::Type>();
    ::testing::StaticAssertTypeEq<double, typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::floating_point>::Type>();

    ASSERT_EQ(sizeof(dbus_bool_t), sizeof(typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::boolean>::Type));
    ASSERT_EQ(sizeof(unsigned char), sizeof(typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::byte>::Type));
    ASSERT_EQ(sizeof(dbus_int16_t), sizeof(typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::int16>::Type));
    ASSERT_EQ(sizeof(dbus_uint16_t), sizeof(typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::uint16>::Type));
    ASSERT_EQ(sizeof(dbus_int32_t), sizeof(typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::int32>::Type));
    ASSERT_EQ(sizeof(dbus_uint32_t), sizeof(typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::uint32>::Type));
    ASSERT_EQ(sizeof(dbus_int64_t), sizeof(typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::int64>::Type));
    ASSERT_EQ(sizeof(dbus_uint64_t), sizeof(typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::uint64>::Type));
    ASSERT_EQ(sizeof(double), sizeof(typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::floating_point>::Type));

    ASSERT_EQ(alignof(dbus_bool_t), alignof(typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::boolean>::Type));
    ASSERT_EQ(alignof(unsigned char), alignof(typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::byte>::Type));
    ASSERT_EQ(alignof(dbus_int16_t), alignof(typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::int16>::Type));
    ASSERT_EQ(alignof(dbus_uint16_t), alignof(typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::uint16>::Type));
    ASSERT_EQ(alignof(dbus_int32_t), alignof(typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::int32>::Type));
    ASSERT_EQ(alignof(dbus_uint32_t), alignof(typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::uint32>::Type));
    ASSERT_EQ(alignof(dbus_int64_t), alignof(typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::int64>::Type));
    ASSERT_EQ(alignof(dbus_uint64_t), alignof(typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::uint64>::Type));
    ASSERT_EQ(alignof(double), alignof(typename core::dbus::helper::DBusTypeMapper<core::dbus::ArgumentType::floating_point>::Type));
}

TEST(Codec, BasicTypesMapToCorrectDBusTypes)
{
    ASSERT_EQ(core::dbus::ArgumentType::boolean, core::dbus::helper::TypeMapper<bool>::type_value());
    ASSERT_EQ(core::dbus::ArgumentType::byte, core::dbus::helper::TypeMapper<int8_t>::type_value());
    ASSERT_EQ(core::dbus::ArgumentType::int16, core::dbus::helper::TypeMapper<int16_t>::type_value());
    ASSERT_EQ(core::dbus::ArgumentType::uint16, core::dbus::helper::TypeMapper<uint16_t>::type_value());
    ASSERT_EQ(core::dbus::ArgumentType::int32, core::dbus::helper::TypeMapper<int32_t>::type_value());
    ASSERT_EQ(core::dbus::ArgumentType::uint32, core::dbus::helper::TypeMapper<uint32_t>::type_value());
    ASSERT_EQ(core::dbus::ArgumentType::floating_point, core::dbus::helper::TypeMapper<float>::type_value());
    ASSERT_EQ(core::dbus::ArgumentType::floating_point, core::dbus::helper::TypeMapper<double>::type_value());
}

TEST(Codec, StlCompoundTypesMapToCorrectDBusTypes)
{
    ASSERT_EQ(core::dbus::ArgumentType::string, core::dbus::helper::TypeMapper<std::string>::type_value());
    ASSERT_EQ(core::dbus::ArgumentType::array, core::dbus::helper::TypeMapper<std::vector<std::string>>::type_value());
    ASSERT_EQ(core::dbus::ArgumentType::array, core::dbus::helper::TypeMapper<std::list<std::string>>::type_value());
    typedef std::map<std::string, std::string> Map; // Need to circumvent a macro restriction with this.
    ASSERT_EQ(core::dbus::ArgumentType::array, core::dbus::helper::TypeMapper< Map >::type_value());
    typedef std::pair<std::string,std::string> Pair; // Need to circumvent a macro restriction with this.
    ASSERT_EQ(core::dbus::ArgumentType::dictionary_entry, core::dbus::helper::TypeMapper<Pair>::type_value());
}

TEST(Codec, BasicTypeSignaturesMapToCorrectDBusSignatures)
{
    ASSERT_STREQ(DBUS_TYPE_BOOLEAN_AS_STRING, core::dbus::helper::TypeMapper<bool>::signature().c_str());
    ASSERT_STREQ(DBUS_TYPE_BYTE_AS_STRING, core::dbus::helper::TypeMapper<int8_t>::signature().c_str());
    ASSERT_STREQ(DBUS_TYPE_INT16_AS_STRING, core::dbus::helper::TypeMapper<int16_t>::signature().c_str());
    ASSERT_STREQ(DBUS_TYPE_UINT16_AS_STRING, core::dbus::helper::TypeMapper<uint16_t>::signature().c_str());
    ASSERT_STREQ(DBUS_TYPE_INT32_AS_STRING, core::dbus::helper::TypeMapper<int32_t>::signature().c_str());
    ASSERT_STREQ(DBUS_TYPE_UINT32_AS_STRING, core::dbus::helper::TypeMapper<uint32_t>::signature().c_str());
    ASSERT_STREQ(DBUS_TYPE_DOUBLE_AS_STRING, core::dbus::helper::TypeMapper<float>::signature().c_str());
    ASSERT_STREQ(DBUS_TYPE_DOUBLE_AS_STRING, core::dbus::helper::TypeMapper<double>::signature().c_str());
}

TEST(Codec, CompoundTypeSignaturesMapToCorrectDBusSignatures)
{
    ASSERT_STREQ(
        DBUS_TYPE_ARRAY_AS_STRING DBUS_TYPE_STRING_AS_STRING,
        core::dbus::helper::TypeMapper<std::vector<std::string>>::signature().c_str());

    ASSERT_STREQ(
        DBUS_TYPE_ARRAY_AS_STRING DBUS_TYPE_STRING_AS_STRING,
        core::dbus::helper::TypeMapper<std::list<std::string>>::signature().c_str());

    typedef std::map<std::string, std::string> Map;

    ASSERT_STREQ(
        DBUS_TYPE_ARRAY_AS_STRING DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_STRING_AS_STRING DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
        core::dbus::helper::TypeMapper<Map>::signature().c_str());
}

namespace
{

const bool default_bool
{
    true
};
const int8_t default_int8
{
    8
};
const int16_t default_int16
{
    -16
};
const uint16_t default_uint16
{
    16
};
const int32_t default_int32
{
    -32
};
const uint32_t default_uint32
{
    32
};
const int64_t default_int64
{
    -64
};
const uint64_t default_uint64
{
    64
};
const float default_float
{
    std::numeric_limits<float>::min()
};
const double default_double
{
    std::numeric_limits<double>::min()
};

core::dbus::Message::Ptr a_method_call()
{
    return core::dbus::Message::make_method_call(
                dbus::DBus::name(),
                dbus::DBus::path(),
                dbus::DBus::interface(),
                "ListNames");
}

std::tuple<
    core::dbus::Message::Ptr,
    std::function<const char*()>
    > a_method_call_with_basic_types_as_arguments()
{
    auto msg = a_method_call();
    auto writer = msg->writer();
    EXPECT_NO_THROW(core::dbus::Codec<bool>::encode_argument(writer, default_bool););
    EXPECT_NO_THROW(core::dbus::Codec<int8_t>::encode_argument(writer, default_int8););
    EXPECT_NO_THROW(core::dbus::Codec<int16_t>::encode_argument(writer, default_int16););
    EXPECT_NO_THROW(core::dbus::Codec<uint16_t>::encode_argument(writer, default_uint16););
    EXPECT_NO_THROW(core::dbus::Codec<int32_t>::encode_argument(writer, default_int32););
    EXPECT_NO_THROW(core::dbus::Codec<uint32_t>::encode_argument(writer, default_uint32););
    EXPECT_NO_THROW(core::dbus::Codec<int64_t>::encode_argument(writer, default_int64););
    EXPECT_NO_THROW(core::dbus::Codec<uint64_t>::encode_argument(writer, default_uint64););
    EXPECT_NO_THROW(core::dbus::Codec<float>::encode_argument(writer, default_float););
    EXPECT_NO_THROW(core::dbus::Codec<double>::encode_argument(writer, default_double););

    std::function<const char*()> signature = []()
    {
        return DBUS_TYPE_BOOLEAN_AS_STRING DBUS_TYPE_BYTE_AS_STRING DBUS_TYPE_INT16_AS_STRING DBUS_TYPE_UINT16_AS_STRING DBUS_TYPE_INT32_AS_STRING DBUS_TYPE_UINT32_AS_STRING DBUS_TYPE_INT64_AS_STRING DBUS_TYPE_UINT64_AS_STRING DBUS_TYPE_DOUBLE_AS_STRING DBUS_TYPE_DOUBLE_AS_STRING;
    };

    return std::make_tuple(msg, signature);
}

template<typename T>
::testing::AssertionResult check_value(core::dbus::Message::Reader& reader, const T& expected_value)
{
    return expected_value == core::dbus::decode_argument<T>(reader) ?
                ::testing::AssertionSuccess() :
                ::testing::AssertionFailure();
}
}

TEST(Codec, EncodingOfBasicTypeYieldsCorrectMessageSignatures)
{
    auto tuple = a_method_call_with_basic_types_as_arguments();
    EXPECT_EQ(std::get<0>(tuple)->signature(), std::get<1>(tuple)());
}

TEST(Codec, DecodingAMessageOfBasicTypesYieldsCorrectValues)
{
    auto tuple = a_method_call_with_basic_types_as_arguments();
    auto reader = std::get<0>(tuple)->reader();

    EXPECT_TRUE(check_value(reader, default_bool));
    EXPECT_TRUE(check_value(reader, default_int8));
    EXPECT_TRUE(check_value(reader, default_int16));
    EXPECT_TRUE(check_value(reader, default_uint16));
    EXPECT_TRUE(check_value(reader, default_int32));
    EXPECT_TRUE(check_value(reader, default_uint32));
    EXPECT_TRUE(check_value(reader, default_int64));
    EXPECT_TRUE(check_value(reader, default_uint64));
    EXPECT_TRUE(check_value(reader, default_float));
    EXPECT_TRUE(check_value(reader, default_double));
}

TEST(ObjectPath, TypeMapperSpecializationReturnsCorrectValues)
{
    namespace dbus = core::dbus;
    ASSERT_EQ(dbus::ArgumentType::object_path, dbus::helper::TypeMapper<dbus::types::ObjectPath>::type_value());
    ASSERT_TRUE(dbus::helper::TypeMapper<dbus::types::ObjectPath>::is_basic_type());
    ASSERT_TRUE(dbus::helper::TypeMapper<dbus::types::ObjectPath>::requires_signature());
    ASSERT_STREQ(DBUS_TYPE_OBJECT_PATH_AS_STRING, dbus::helper::TypeMapper<dbus::types::ObjectPath>::signature().c_str());
}

TEST(ObjectPath, EncodingAndDecodingWorksCorrectly)
{
    namespace dbus = core::dbus;
    const dbus::types::ObjectPath expected_value
    {
        dbus::DBus::path()
    };
    auto msg = a_method_call();
    auto writer = msg->writer();
    ASSERT_NO_THROW(dbus::encode_argument(writer, expected_value););

    auto reader = msg->reader();
    ASSERT_EQ(expected_value, dbus::decode_argument<dbus::types::ObjectPath>(reader));
}

TEST(Unixfd, TypeMapperSpecializationReturnsCorrectValues)
{
    namespace dbus = core::dbus;
    ASSERT_EQ(dbus::ArgumentType::unix_fd, dbus::helper::TypeMapper<dbus::types::UnixFd>::type_value());
    ASSERT_TRUE(dbus::helper::TypeMapper<dbus::types::UnixFd>::is_basic_type());
    ASSERT_TRUE(dbus::helper::TypeMapper<dbus::types::UnixFd>::requires_signature());
    ASSERT_STREQ(DBUS_TYPE_UNIX_FD_AS_STRING, dbus::helper::TypeMapper<dbus::types::UnixFd>::signature().c_str());
}

#include <sys/eventfd.h>

TEST(UnixFd, EncodingAndDecodingWorksCorrectly)
{
    namespace dbus = core::dbus;
    const dbus::types::UnixFd expected_value
    {
        eventfd(0,0)
    };
    auto msg = a_method_call();
    auto writer = msg->writer();

    ASSERT_NO_THROW(dbus::encode_argument(writer, expected_value););

    auto reader = msg->reader();
    auto fd = dbus::decode_argument<dbus::types::UnixFd>(reader);

    // We cannot just check for equality of fd and expected_value as libdbus duplicates the fd.
    static const uint64_t magic_value
    {
        42
    };
    EXPECT_TRUE(write(expected_value.to_int(), std::addressof(magic_value), sizeof(magic_value)) >= 0);
    uint64_t result {0};
    EXPECT_TRUE(read(fd.to_int(), std::addressof(result), sizeof(result)) >= 0);
    ASSERT_EQ(magic_value, result);
}

TEST(Variant, TypeMapperSpecializationReturnsCorrectValues)
{
    namespace dbus = core::dbus;
    ASSERT_EQ(dbus::ArgumentType::variant, dbus::helper::TypeMapper<dbus::types::Variant>::type_value());
    ASSERT_TRUE(dbus::helper::TypeMapper<dbus::types::ObjectPath>::is_basic_type());
    ASSERT_TRUE(dbus::helper::TypeMapper<dbus::types::ObjectPath>::requires_signature());
    ASSERT_STREQ(DBUS_TYPE_VARIANT_AS_STRING, dbus::helper::TypeMapper<dbus::types::Variant>::signature().c_str());
}

TEST(Variant, EncodingAndDecodingWorksCorrectly)
{
    namespace dbus = core::dbus;

    typedef dbus::types::Struct<std::tuple<double, double, std::int32_t>> Struct;

    const Struct expected_value = Struct{std::make_tuple(42., 42., 56)};
    auto msg = a_method_call();
    {
        auto writer = msg->writer();
        ASSERT_NO_THROW(dbus::encode_argument(writer, dbus::types::Variant::encode(expected_value)););
    }
    auto reader = msg->reader();
    Struct my_struct;
    auto variant = std::move(dbus::types::Variant::decode(my_struct));
    dbus::decode_argument<dbus::types::Variant>(
        reader,
        variant);
    ASSERT_EQ(expected_value, my_struct);
}

TEST(Signature, TypeMapperSpecializationReturnsCorrectValues)
{
    namespace dbus = core::dbus;
    ASSERT_EQ(dbus::ArgumentType::signature, dbus::helper::TypeMapper<dbus::types::Signature>::type_value());
    ASSERT_TRUE(dbus::helper::TypeMapper<dbus::types::Signature>::is_basic_type());
    ASSERT_TRUE(dbus::helper::TypeMapper<dbus::types::Signature>::requires_signature());
    ASSERT_STREQ(DBUS_TYPE_SIGNATURE_AS_STRING, dbus::helper::TypeMapper<dbus::types::Signature>::signature().c_str());
}

TEST(Signature, EncodingAndDecodingWorksCorrectly)
{
    namespace dbus = core::dbus;
    const dbus::types::Signature expected_value
    {
        "(ii)"
    };
    auto msg = a_method_call();
    auto writer = msg->writer();
    {
        ASSERT_NO_THROW(dbus::encode_argument(writer, expected_value););
    }
    auto reader = msg->reader();
    auto signature = dbus::decode_argument<dbus::types::Signature>(reader);
    ASSERT_EQ(expected_value, signature);
}

TEST(Properties, DictionaryMappingToVariantsIsEncodedCorrectly)
{
    namespace dbus = core::dbus;

    const std::string key{"key"};

    auto msg = a_method_call();

    {
        auto writer = msg->writer();
        auto array = writer.open_array(dbus::types::Signature{"{sv}"});
        for(unsigned int i = 0; i < 5; i++)
        {
            auto entry = array.open_dict_entry();
            {
                entry.push_stringn(key.c_str(), key.size());
                auto variant = entry.open_variant(dbus::types::Signature{dbus::helper::TypeMapper<std::uint32_t>::signature()});
                {
                    variant.push_uint32(i);

                } entry.close_variant(std::move(variant));
            } array.close_dict_entry(std::move(entry));
        } writer.close_array(std::move(array));
    }
    unsigned int counter = 0;

    auto reader = msg->reader();
    auto array = reader.pop_array();

    while (array.type() != dbus::ArgumentType::invalid)
    {
        auto entry = array.pop_dict_entry();
        {
            EXPECT_EQ(key, std::string{entry.pop_string()});
            auto variant = entry.pop_variant();
            {
                EXPECT_EQ(counter, variant.pop_uint32());
            }
        }
        counter++;
    }

    EXPECT_EQ(5, counter);
}

TEST(Properties, DictionaryMappingToVariantsIsEncodedCorrectlyWithMap)
{
    namespace dbus = core::dbus;

    auto msg = a_method_call();

    {
        std::map<std::string, dbus::types::Variant> map;

        map["key1"] = dbus::types::Variant::encode<std::uint32_t>(1);
        map["key2"] = dbus::types::Variant::encode<std::uint32_t>(2);
        map["key3"] = dbus::types::Variant::encode<std::uint32_t>(3);
        map["key4"] = dbus::types::Variant::encode<std::uint32_t>(4);
        map["key5"] = dbus::types::Variant::encode<std::uint32_t>(5);

        msg->writer() << map;
    }

    auto reader = msg->reader();
    std::map<std::string, dbus::types::Variant> map;
    reader >> map;

    EXPECT_EQ(5, map.size());
    EXPECT_EQ(1, map.at("key1").as<std::uint32_t>());
    EXPECT_EQ(2, map.at("key2").as<std::uint32_t>());
    EXPECT_EQ(3, map.at("key3").as<std::uint32_t>());
    EXPECT_EQ(4, map.at("key4").as<std::uint32_t>());
    EXPECT_EQ(5, map.at("key5").as<std::uint32_t>());
}
