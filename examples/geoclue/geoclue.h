#ifndef ORG_FREEDESKTOP_GEOCLUE_H_
#define ORG_FREEDESKTOP_GEOCLUE_H_

#include "org/freedesktop/dbus/service.h"
#include "org/freedesktop/dbus/types/object_path.h"
#include "org/freedesktop/dbus/types/struct.h"

#include <chrono>
#include <string>

namespace org
{
namespace freedesktop
{
struct Geoclue
{
    struct Master
    {
        struct Create
        {
            static std::string name()
            {
                return "Create";
            } typedef Master Interface;
            static const std::chrono::milliseconds default_timeout;
        };
    };

    struct MasterClient
    {
        struct SetRequirements
        {
            static std::string name()
            {
                return "SetRequirements";
            } typedef MasterClient Interface;
            static const std::chrono::milliseconds default_timeout;
        };
        struct GetAddressProvider
        {
            static std::string name()
            {
                return "GetAddressProvider";
            } typedef MasterClient Interface;
            static const std::chrono::milliseconds default_timeout;
        };
        struct GetPositionProvider
        {
            static std::string name()
            {
                return "GetPositionProvider";
            } typedef MasterClient Interface;
            static const std::chrono::milliseconds default_timeout;
        };
    };

    struct Address
    {
        struct GetAddress
        {
            static std::string name()
            {
                return "GetAddress";
            } typedef Address Interface;
            static const std::chrono::milliseconds default_timeout;
        };
    };

    struct Position
    {
        struct GetPosition
        {
            static std::string name()
            {
                return "GetPosition";
            } typedef Position Interface;
            static const std::chrono::milliseconds default_timeout;
        };
        struct Signals
        {
            struct PositionChanged
            {
                static std::string name()
                {
                    return "PositionChanged";
                };
                typedef Position Interface;
                typedef std::tuple<int32_t, int32_t, double, double, double, dbus::types::Struct<std::tuple<int32_t, double, double>>> ArgumentType;
            };
        };
    };
};
const std::chrono::milliseconds Geoclue::Master::Create::default_timeout
{
    10*1000
};
const std::chrono::milliseconds Geoclue::MasterClient::SetRequirements::default_timeout
{
    10*1000
};
const std::chrono::milliseconds Geoclue::MasterClient::GetAddressProvider::default_timeout
{
    10*1000
};
const std::chrono::milliseconds Geoclue::MasterClient::GetPositionProvider::default_timeout
{
    10*1000
};
const std::chrono::milliseconds Geoclue::Address::GetAddress::default_timeout
{
    10*1000
};
const std::chrono::milliseconds Geoclue::Position::GetPosition::default_timeout
{
    10*1000
};
}
}

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace traits
{
template<>
struct Service<org::freedesktop::Geoclue>
{
    static const std::string& interface_name()
    {
        static const std::string s
        {"org.freedesktop.Geoclue"
        };
        return s;
    }
};

template<>
struct Service<org::freedesktop::Geoclue::Master>
{
    static const std::string& interface_name()
    {
        static const std::string s
        {"org.freedesktop.Geoclue.Master"
        };
        return s;
    }
};

template<>
struct Service<org::freedesktop::Geoclue::MasterClient>
{
    static const std::string& interface_name()
    {
        static const std::string s
        {"org.freedesktop.Geoclue.MasterClient"
        };
        return s;
    }
};

template<>
struct Service<org::freedesktop::Geoclue::Address>
{
    static const std::string& interface_name()
    {
        static const std::string s
        {"org.freedesktop.Geoclue.Address"
        };
        return s;
    }
};

template<>
struct Service<org::freedesktop::Geoclue::Position>
{
    static const std::string& interface_name()
    {
        static const std::string s
        {"org.freedesktop.Geoclue.Position"
        };
        return s;
    }
};
}
}
}
}

#endif // ORG_FREEDESKTOP_GEOCLUE_H_
