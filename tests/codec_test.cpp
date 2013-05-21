#include "org/freedesktop/dbus/codec.h"
#include "org/freedesktop/dbus/helper/signature.h"

#include "org/freedesktop/dbus/types/any.h"
#include "org/freedesktop/dbus/types/object_path.h"
#include "org/freedesktop/dbus/types/signature.h"
#include "org/freedesktop/dbus/types/unix_fd.h"
#include "org/freedesktop/dbus/types/variant.h"

// STL includes
#include "org/freedesktop/dbus/types/stl/list.h"
#include "org/freedesktop/dbus/types/stl/map.h"
#include "org/freedesktop/dbus/types/stl/string.h"
#include "org/freedesktop/dbus/types/stl/vector.h"

#include <gtest/gtest.h>

#include <memory>

TEST(Codec, BasicTypesMatchSizeAndAlignOfDBusTypes)
{
    ::testing::StaticAssertTypeEq<dbus_bool_t, typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::boolean>::Type>();
    ::testing::StaticAssertTypeEq<int8_t, typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::byte>::Type>();
    ::testing::StaticAssertTypeEq<int16_t, typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::int16>::Type>();
    ::testing::StaticAssertTypeEq<uint16_t, typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::uint16>::Type>();
    ::testing::StaticAssertTypeEq<int32_t, typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::int32>::Type>();
    ::testing::StaticAssertTypeEq<uint32_t, typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::uint32>::Type>();
    ::testing::StaticAssertTypeEq<int64_t, typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::int64>::Type>();
    ::testing::StaticAssertTypeEq<uint64_t, typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::uint64>::Type>();
    ::testing::StaticAssertTypeEq<double, typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::floating_point>::Type>();

    ASSERT_EQ(sizeof(dbus_bool_t), sizeof(typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::boolean>::Type));
    ASSERT_EQ(sizeof(unsigned char), sizeof(typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::byte>::Type));
    ASSERT_EQ(sizeof(dbus_int16_t), sizeof(typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::int16>::Type));
    ASSERT_EQ(sizeof(dbus_uint16_t), sizeof(typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::uint16>::Type));
    ASSERT_EQ(sizeof(dbus_int32_t), sizeof(typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::int32>::Type));
    ASSERT_EQ(sizeof(dbus_uint32_t), sizeof(typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::uint32>::Type));
    ASSERT_EQ(sizeof(dbus_int64_t), sizeof(typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::int64>::Type));
    ASSERT_EQ(sizeof(dbus_uint64_t), sizeof(typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::uint64>::Type));
    ASSERT_EQ(sizeof(double), sizeof(typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::floating_point>::Type));

    ASSERT_EQ(alignof(dbus_bool_t), alignof(typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::boolean>::Type));
    ASSERT_EQ(alignof(unsigned char), alignof(typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::byte>::Type));
    ASSERT_EQ(alignof(dbus_int16_t), alignof(typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::int16>::Type));
    ASSERT_EQ(alignof(dbus_uint16_t), alignof(typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::uint16>::Type));
    ASSERT_EQ(alignof(dbus_int32_t), alignof(typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::int32>::Type));
    ASSERT_EQ(alignof(dbus_uint32_t), alignof(typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::uint32>::Type));
    ASSERT_EQ(alignof(dbus_int64_t), alignof(typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::int64>::Type));
    ASSERT_EQ(alignof(dbus_uint64_t), alignof(typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::uint64>::Type));
    ASSERT_EQ(alignof(double), alignof(typename org::freedesktop::dbus::helper::DBusTypeMapper<org::freedesktop::dbus::ArgumentType::floating_point>::Type));
}

TEST(Codec, BasicTypesMapToCorrectDBusTypes)
{
    ASSERT_EQ(org::freedesktop::dbus::ArgumentType::boolean, org::freedesktop::dbus::helper::TypeMapper<bool>::type_value());
    ASSERT_EQ(org::freedesktop::dbus::ArgumentType::byte, org::freedesktop::dbus::helper::TypeMapper<int8_t>::type_value());
    ASSERT_EQ(org::freedesktop::dbus::ArgumentType::int16, org::freedesktop::dbus::helper::TypeMapper<int16_t>::type_value());
    ASSERT_EQ(org::freedesktop::dbus::ArgumentType::uint16, org::freedesktop::dbus::helper::TypeMapper<uint16_t>::type_value());
    ASSERT_EQ(org::freedesktop::dbus::ArgumentType::int32, org::freedesktop::dbus::helper::TypeMapper<int32_t>::type_value());
    ASSERT_EQ(org::freedesktop::dbus::ArgumentType::uint32, org::freedesktop::dbus::helper::TypeMapper<uint32_t>::type_value());
    ASSERT_EQ(org::freedesktop::dbus::ArgumentType::floating_point, org::freedesktop::dbus::helper::TypeMapper<float>::type_value());
    ASSERT_EQ(org::freedesktop::dbus::ArgumentType::floating_point, org::freedesktop::dbus::helper::TypeMapper<double>::type_value());
}

TEST(Codec, StlCompoundTypesMapToCorrectDBusTypes)
{
    ASSERT_EQ(org::freedesktop::dbus::ArgumentType::string, org::freedesktop::dbus::helper::TypeMapper<std::string>::type_value());
    ASSERT_EQ(org::freedesktop::dbus::ArgumentType::array, org::freedesktop::dbus::helper::TypeMapper<std::vector<std::string>>::type_value());
    ASSERT_EQ(org::freedesktop::dbus::ArgumentType::array, org::freedesktop::dbus::helper::TypeMapper<std::list<std::string>>::type_value());
    typedef std::map<std::string, std::string> Map; // Need to circumvent a macro restriction with this.
    ASSERT_EQ(org::freedesktop::dbus::ArgumentType::array, org::freedesktop::dbus::helper::TypeMapper< Map >::type_value());
    typedef std::pair<std::string,std::string> Pair; // Need to circumvent a macro restriction with this.
    ASSERT_EQ(org::freedesktop::dbus::ArgumentType::dictionary_entry, org::freedesktop::dbus::helper::TypeMapper<Pair>::type_value());
}

TEST(Codec, BasicTypeSignaturesMapToCorrectDBusSignatures)
{
    ASSERT_STREQ(DBUS_TYPE_BOOLEAN_AS_STRING, org::freedesktop::dbus::helper::TypeMapper<bool>::signature().c_str());
    ASSERT_STREQ(DBUS_TYPE_BYTE_AS_STRING, org::freedesktop::dbus::helper::TypeMapper<int8_t>::signature().c_str());
    ASSERT_STREQ(DBUS_TYPE_INT16_AS_STRING, org::freedesktop::dbus::helper::TypeMapper<int16_t>::signature().c_str());
    ASSERT_STREQ(DBUS_TYPE_UINT16_AS_STRING, org::freedesktop::dbus::helper::TypeMapper<uint16_t>::signature().c_str());
    ASSERT_STREQ(DBUS_TYPE_INT32_AS_STRING, org::freedesktop::dbus::helper::TypeMapper<int32_t>::signature().c_str());
    ASSERT_STREQ(DBUS_TYPE_UINT32_AS_STRING, org::freedesktop::dbus::helper::TypeMapper<uint32_t>::signature().c_str());
    ASSERT_STREQ(DBUS_TYPE_DOUBLE_AS_STRING, org::freedesktop::dbus::helper::TypeMapper<float>::signature().c_str());
    ASSERT_STREQ(DBUS_TYPE_DOUBLE_AS_STRING, org::freedesktop::dbus::helper::TypeMapper<double>::signature().c_str());
}

TEST(Codec, CompoundTypeSignaturesMapToCorrectDBusSignatures)
{
    ASSERT_STREQ(
        DBUS_TYPE_ARRAY_AS_STRING DBUS_TYPE_STRING_AS_STRING,
        org::freedesktop::dbus::helper::TypeMapper<std::vector<std::string>>::signature().c_str());

    ASSERT_STREQ(
        DBUS_TYPE_ARRAY_AS_STRING DBUS_TYPE_STRING_AS_STRING,
        org::freedesktop::dbus::helper::TypeMapper<std::list<std::string>>::signature().c_str());

    typedef std::map<std::string, std::string> Map;

    ASSERT_STREQ(
        DBUS_TYPE_ARRAY_AS_STRING DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_STRING_AS_STRING DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
        org::freedesktop::dbus::helper::TypeMapper<Map>::signature().c_str());
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

std::shared_ptr<DBusMessage> a_method_call()
{
    return std::shared_ptr<DBusMessage>(
               dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_SERVICE_DBUS, "ListNames"),
               [](DBusMessage* msg)
    {
        dbus_message_unref(msg);
    });
}

std::tuple<
std::shared_ptr<DBusMessage>,
    std::function<const char*()>
    > a_method_call_with_basic_types_as_arguments()
{
    auto msg = a_method_call();

    DBusMessageIter iter;
    dbus_message_iter_init_append(msg.get(), std::addressof(iter));
    {
        EXPECT_NO_THROW(org::freedesktop::dbus::Codec<bool>::encode_argument(std::addressof(iter), default_bool););
        EXPECT_NO_THROW(org::freedesktop::dbus::Codec<int8_t>::encode_argument(std::addressof(iter), default_int8););
        EXPECT_NO_THROW(org::freedesktop::dbus::Codec<int16_t>::encode_argument(std::addressof(iter), default_int16););
        EXPECT_NO_THROW(org::freedesktop::dbus::Codec<uint16_t>::encode_argument(std::addressof(iter), default_uint16););
        EXPECT_NO_THROW(org::freedesktop::dbus::Codec<int32_t>::encode_argument(std::addressof(iter), default_int32););
        EXPECT_NO_THROW(org::freedesktop::dbus::Codec<uint32_t>::encode_argument(std::addressof(iter), default_uint32););
        EXPECT_NO_THROW(org::freedesktop::dbus::Codec<int64_t>::encode_argument(std::addressof(iter), default_int64););
        EXPECT_NO_THROW(org::freedesktop::dbus::Codec<uint64_t>::encode_argument(std::addressof(iter), default_uint64););
        EXPECT_NO_THROW(org::freedesktop::dbus::Codec<float>::encode_argument(std::addressof(iter), default_float););
        EXPECT_NO_THROW(org::freedesktop::dbus::Codec<double>::encode_argument(std::addressof(iter), default_double););
    }

    auto signature = []()
    {
        return DBUS_TYPE_BOOLEAN_AS_STRING DBUS_TYPE_BYTE_AS_STRING DBUS_TYPE_INT16_AS_STRING DBUS_TYPE_UINT16_AS_STRING DBUS_TYPE_INT32_AS_STRING DBUS_TYPE_UINT32_AS_STRING DBUS_TYPE_INT64_AS_STRING DBUS_TYPE_UINT64_AS_STRING DBUS_TYPE_DOUBLE_AS_STRING DBUS_TYPE_DOUBLE_AS_STRING;
    };

    return std::make_tuple(msg, signature);
}
template<typename T>
void check_value_and_advance(DBusMessageIter* iter, const T& expected_value)
{
    ASSERT_EQ(expected_value, org::freedesktop::dbus::decode_argument<T>(iter));
    dbus_message_iter_next(iter);
}
}

TEST(Codec, EncodingOfBasicTypeYieldsCorrectMessageSignatures)
{
    auto tuple = a_method_call_with_basic_types_as_arguments();
    EXPECT_STREQ(dbus_message_get_signature(std::get<0>(tuple).get()), std::get<1>(tuple)());
}

TEST(Codec, DecodingAMessageOfBasicTypesYieldsCorrectValues)
{
    auto tuple = a_method_call_with_basic_types_as_arguments();

    DBusMessageIter iter;
    EXPECT_EQ(TRUE, dbus_message_iter_init(std::get<0>(tuple).get(), std::addressof(iter)));

    ASSERT_NO_FATAL_FAILURE(check_value_and_advance(std::addressof(iter), default_bool););
    ASSERT_NO_FATAL_FAILURE(check_value_and_advance(std::addressof(iter), default_int8););
    ASSERT_NO_FATAL_FAILURE(check_value_and_advance(std::addressof(iter), default_int16););
    ASSERT_NO_FATAL_FAILURE(check_value_and_advance(std::addressof(iter), default_uint16););
    ASSERT_NO_FATAL_FAILURE(check_value_and_advance(std::addressof(iter), default_int32););
    ASSERT_NO_FATAL_FAILURE(check_value_and_advance(std::addressof(iter), default_uint32););
    ASSERT_NO_FATAL_FAILURE(check_value_and_advance(std::addressof(iter), default_int64););
    ASSERT_NO_FATAL_FAILURE(check_value_and_advance(std::addressof(iter), default_uint64););
    ASSERT_NO_FATAL_FAILURE(check_value_and_advance(std::addressof(iter), default_float););
    ASSERT_NO_FATAL_FAILURE(check_value_and_advance(std::addressof(iter), default_double););
}

TEST(ObjectPath, TypeMapperSpecializationReturnsCorrectValues)
{
    namespace dbus = org::freedesktop::dbus;
    ASSERT_EQ(dbus::ArgumentType::object_path, dbus::helper::TypeMapper<dbus::types::ObjectPath>::type_value());
    ASSERT_FALSE(dbus::helper::TypeMapper<dbus::types::ObjectPath>::is_basic_type());
    ASSERT_TRUE(dbus::helper::TypeMapper<dbus::types::ObjectPath>::requires_signature());
    ASSERT_STREQ(DBUS_TYPE_OBJECT_PATH_AS_STRING, dbus::helper::TypeMapper<dbus::types::ObjectPath>::signature().c_str());
}

TEST(ObjectPath, EncodingAndDecodingWorksCorrectly)
{
    namespace dbus = org::freedesktop::dbus;
    const dbus::types::ObjectPath expected_value
    {
        DBUS_PATH_DBUS
    };
    auto msg = a_method_call();
    DBusMessageIter iter;
    dbus_message_iter_init_append(msg.get(), std::addressof(iter));

    {
        ASSERT_NO_THROW(dbus::encode_argument(std::addressof(iter), expected_value););
    }

    dbus_message_iter_init(msg.get(), std::addressof(iter));
    ASSERT_EQ(expected_value, dbus::decode_argument<dbus::types::ObjectPath>(std::addressof(iter)));
}

TEST(Unixfd, TypeMapperSpecializationReturnsCorrectValues)
{
    namespace dbus = org::freedesktop::dbus;
    ASSERT_EQ(dbus::ArgumentType::unix_fd, dbus::helper::TypeMapper<dbus::types::UnixFd>::type_value());
    ASSERT_FALSE(dbus::helper::TypeMapper<dbus::types::UnixFd>::is_basic_type());
    ASSERT_TRUE(dbus::helper::TypeMapper<dbus::types::UnixFd>::requires_signature());
    ASSERT_STREQ(DBUS_TYPE_UNIX_FD_AS_STRING, dbus::helper::TypeMapper<dbus::types::UnixFd>::signature().c_str());
}

#include <sys/eventfd.h>

TEST(UnixFd, EncodingAndDecodingWorksCorrectly)
{
    namespace dbus = org::freedesktop::dbus;
    const dbus::types::UnixFd expected_value
    {
        eventfd(0,0)
    };
    auto msg = a_method_call();
    DBusMessageIter iter;
    dbus_message_iter_init_append(msg.get(), std::addressof(iter));

    {
        ASSERT_NO_THROW(dbus::encode_argument(std::addressof(iter), expected_value););
    }

    dbus_message_iter_init(msg.get(), std::addressof(iter));
    auto fd = dbus::decode_argument<dbus::types::UnixFd>(std::addressof(iter));

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
    namespace dbus = org::freedesktop::dbus;
    ASSERT_EQ(dbus::ArgumentType::variant, dbus::helper::TypeMapper<dbus::types::Variant<double>>::type_value());
    ASSERT_FALSE(dbus::helper::TypeMapper<dbus::types::ObjectPath>::is_basic_type());
    ASSERT_TRUE(dbus::helper::TypeMapper<dbus::types::ObjectPath>::requires_signature());
    ASSERT_STREQ(DBUS_TYPE_VARIANT_AS_STRING DBUS_TYPE_DOUBLE_AS_STRING, dbus::helper::TypeMapper<dbus::types::Variant<double>>::signature().c_str());
}

TEST(Variant, EncodingAndDecodingWorksCorrectly)
{
    namespace dbus = org::freedesktop::dbus;
    const dbus::types::Variant<double> expected_value {42.};
    auto msg = a_method_call();
    DBusMessageIter iter;
    dbus_message_iter_init_append(msg.get(), std::addressof(iter));

    {
        ASSERT_NO_THROW(dbus::encode_argument(std::addressof(iter), expected_value););
    }

    dbus_message_iter_init(msg.get(), std::addressof(iter));
    ASSERT_EQ(expected_value, dbus::decode_argument<dbus::types::Variant<double>>(std::addressof(iter)));
}

TEST(Signature, TypeMapperSpecializationReturnsCorrectValues)
{
    namespace dbus = org::freedesktop::dbus;
    ASSERT_EQ(dbus::ArgumentType::signature, dbus::helper::TypeMapper<dbus::types::Signature>::type_value());
    ASSERT_FALSE(dbus::helper::TypeMapper<dbus::types::Signature>::is_basic_type());
    ASSERT_TRUE(dbus::helper::TypeMapper<dbus::types::Signature>::requires_signature());
    ASSERT_STREQ(DBUS_TYPE_SIGNATURE_AS_STRING, dbus::helper::TypeMapper<dbus::types::Signature>::signature().c_str());
}

TEST(Signature, EncodingAndDecodingWorksCorrectly)
{
    namespace dbus = org::freedesktop::dbus;
    const dbus::types::Signature expected_value
    {"(ii)"
    };
    auto msg = a_method_call();
    DBusMessageIter iter;
    dbus_message_iter_init_append(msg.get(), std::addressof(iter));

    {
        ASSERT_NO_THROW(dbus::encode_argument(std::addressof(iter), expected_value););
    }

    dbus_message_iter_init(msg.get(), std::addressof(iter));
    ASSERT_EQ(expected_value, dbus::decode_argument<dbus::types::Signature>(std::addressof(iter)));
}
