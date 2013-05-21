#ifndef ORG_FREEDESKTOP_UPOWER_H_
#define ORG_FREEDESKTOP_UPOWER_H_

#include "org/freedesktop/dbus/service.h"
#include "org/freedesktop/dbus/types/object_path.h"

#include <chrono>
#include <string>

namespace org
{
namespace freedesktop
{
struct UPower
{
    struct Properties
    {
        struct DaemonVersion
        {
            static std::string name()
            {
                return "DaemonVersion";
            };
            typedef UPower Interface;
            typedef std::string ValueType;
            static const bool readable = true;
            static const bool writable = false;
        };
        struct CanSuspend
        {
            static std::string name()
            {
                return "CanSuspend";
            };
            typedef UPower Interface;
            typedef bool ValueType;
            static const bool readable = true;
            static const bool writable = false;
        };
        struct CanHibernate
        {
            static std::string name()
            {
                return "CanHibernate";
            };
            typedef UPower Interface;
            typedef bool ValueType;
            static const bool readable = true;
            static const bool writable = false;
        };
        struct OnBattery
        {
            static std::string name()
            {
                return "OnBattery";
            };
            typedef UPower Interface;
            typedef bool ValueType;
            static const bool readable = true;
            static const bool writable = false;
        };
        struct OnLowBattery
        {
            static std::string name()
            {
                return "OnLowBattery";
            };
            typedef UPower Interface;
            typedef bool ValueType;
            static const bool readable = true;
            static const bool writable = false;
        };
        struct LidIsClosed
        {
            static std::string name()
            {
                return "LidIsClosed";
            };
            typedef UPower Interface;
            typedef bool ValueType;
            static const bool readable = true;
            static const bool writable = false;
        };
        struct LidIsPresent
        {
            static std::string name()
            {
                return "LidIsPresent";
            };
            typedef UPower Interface;
            typedef bool ValueType;
            static const bool readable = true;
            static const bool writable = false;
        };
    };

    struct Signals
    {
        struct DeviceAdded
        {
            static std::string name()
            {
                return "DeviceAdded";
            };
            typedef UPower Interface;
            typedef org::freedesktop::dbus::types::ObjectPath ArgumentType;
        };
        struct DeviceRemoved
        {
            static std::string name()
            {
                return "DeviceRemoved";
            };
            typedef UPower Interface;
            typedef org::freedesktop::dbus::types::ObjectPath ArgumentType;
        };
        struct DeviceChanged
        {
            static std::string name()
            {
                return "DeviceChanged";
            };
            typedef UPower Interface;
            typedef std::string ArgumentType;
        };
        struct Changed
        {
            static std::string name()
            {
                return "Changed";
            };
            typedef UPower Interface;
            typedef void ArgumentType;
        };
        struct Sleeping
        {
            static std::string name()
            {
                return "Sleeping";
            };
            typedef UPower Interface;
            typedef void ArgumentType;
        };
        struct Resuming
        {
            static std::string name()
            {
                return "Resuming";
            };
            typedef UPower Interface;
            typedef void ArgumentType;
        };
    };

    struct EnumerateDevices
    {
        typedef UPower Interface;

        static const std::string& name()
        {
            static const std::string s
            {
                "EnumerateDevices"
            };
            return s;
        }

        static const std::chrono::milliseconds default_timeout;
    };

    struct Device
    {
        struct GetHistory
        {
            static std::string name()
            {
                return "GetHistory";
            } typedef Device Interface;
            static const std::chrono::milliseconds default_timeout;
        };
        struct GetStatistics
        {
            static std::string name()
            {
                return "GetStatistics";
            } typedef Device Interface;
            static const std::chrono::milliseconds default_timeout;
        };
        struct Properties
        {
            struct NativePath
            {
                static std::string name()
                {
                    return "NativePath";
                };
                typedef Device Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct Vendor
            {
                static std::string name()
                {
                    return "Vendor";
                };
                typedef Device Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct Model
            {
                static std::string name()
                {
                    return "Model";
                };
                typedef Device Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct Serial
            {
                static std::string name()
                {
                    return "Serial";
                };
                typedef Device Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct UpdateTime
            {
                static std::string name()
                {
                    return "UpdateTime";
                };
                typedef Device Interface;
                typedef uint64_t ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct Type
            {
                static std::string name()
                {
                    return "Type";
                };
                typedef Device Interface;
                typedef uint32_t ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct PowerSupply
            {
                static std::string name()
                {
                    return "PowerSupply";
                };
                typedef Device Interface;
                typedef bool ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct HasHistory
            {
                static std::string name()
                {
                    return "HasHistory";
                };
                typedef Device Interface;
                typedef bool ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct HasStatistics
            {
                static std::string name()
                {
                    return "HasStatistics";
                };
                typedef Device Interface;
                typedef bool ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct Online
            {
                static std::string name()
                {
                    return "Online";
                };
                typedef Device Interface;
                typedef bool ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct Energy
            {
                static std::string name()
                {
                    return "Energy";
                };
                typedef Device Interface;
                typedef double ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct EnergyEmpty
            {
                static std::string name()
                {
                    return "EnergyEmpty";
                };
                typedef Device Interface;
                typedef double ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct EnergyFull
            {
                static std::string name()
                {
                    return "EnergyFull";
                };
                typedef Device Interface;
                typedef double ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct EnergyFullDesign
            {
                static std::string name()
                {
                    return "EnergyFullDesign";
                };
                typedef Device Interface;
                typedef double ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct EnergyRate
            {
                static std::string name()
                {
                    return "EnergyRate";
                };
                typedef Device Interface;
                typedef double ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct Voltage
            {
                static std::string name()
                {
                    return "Voltage";
                };
                typedef Device Interface;
                typedef double ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct TimeToEmpty
            {
                static std::string name()
                {
                    return "TimeToEmpty";
                };
                typedef Device Interface;
                typedef int64_t ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct TimeToFull
            {
                static std::string name()
                {
                    return "TimeToFull";
                };
                typedef Device Interface;
                typedef int64_t ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct Percentage
            {
                static std::string name()
                {
                    return "Percentage";
                };
                typedef Device Interface;
                typedef double ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct IsPresent
            {
                static std::string name()
                {
                    return "IsPresent";
                };
                typedef Device Interface;
                typedef bool ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct State
            {
                static std::string name()
                {
                    return "State";
                };
                typedef Device Interface;
                typedef uint32_t ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct IsRechargeable
            {
                static std::string name()
                {
                    return "IsRechargeable";
                };
                typedef Device Interface;
                typedef bool ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct Capacity
            {
                static std::string name()
                {
                    return "Capacity";
                };
                typedef Device Interface;
                typedef double ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct Technology
            {
                static std::string name()
                {
                    return "Technology";
                };
                typedef Device Interface;
                typedef uint32_t ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct RecallNotice
            {
                static std::string name()
                {
                    return "RecallNotice";
                };
                typedef Device Interface;
                typedef bool ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct RecallVendor
            {
                static std::string name()
                {
                    return "RecallVendor";
                };
                typedef Device Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
            struct RecallUrl
            {
                static std::string name()
                {
                    return "RecallUrl";
                };
                typedef Device Interface;
                typedef std::string ValueType;
                static const bool readable = true;
                static const bool writable = false;
            };
        };
        struct Signals
        {
            struct Changed
            {
                static std::string name()
                {
                    return "Changed";
                };
                typedef Device Interface;
                typedef void ArgumentType;
            };
        };
    };

};
const std::chrono::milliseconds UPower::EnumerateDevices::default_timeout
{
    10*1000
};
const std::chrono::milliseconds UPower::Device::GetHistory::default_timeout
{
    10*1000
};
const std::chrono::milliseconds UPower::Device::GetStatistics::default_timeout
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
struct Service<org::freedesktop::UPower>
{
    static const std::string& interface_name()
    {
        static const std::string s
        {
            "org.freedesktop.UPower"
        };
        return s;
    }
};

template<>
struct Service<org::freedesktop::UPower::Device>
{
    static const std::string& interface_name()
    {
        static const std::string s
        {
            "org.freedesktop.UPower.Device"
        };
        return s;
    }
};
}
}
}
}

#endif // ORG_FREEDESKTOP_UPOWER_H_
