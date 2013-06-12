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
            inline static std::string name()
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
            inline static std::string name()
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
            inline static std::string name()
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
            inline static std::string name()
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
            inline static std::string name()
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
            inline static std::string name()
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
            inline static std::string name()
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
            inline static std::string name()
            {
                return "DeviceAdded";
            };
            typedef UPower Interface;
            typedef org::freedesktop::dbus::types::ObjectPath ArgumentType;
        };
        struct DeviceRemoved
        {
            inline static std::string name()
            {
                return "DeviceRemoved";
            };
            typedef UPower Interface;
            typedef org::freedesktop::dbus::types::ObjectPath ArgumentType;
        };
        struct DeviceChanged
        {
            inline static std::string name()
            {
                return "DeviceChanged";
            };
            typedef UPower Interface;
            typedef std::string ArgumentType;
        };
        struct Changed
        {
            inline static std::string name()
            {
                return "Changed";
            };
            typedef UPower Interface;
            typedef void ArgumentType;
        };
        struct Sleeping
        {
            inline static std::string name()
            {
                return "Sleeping";
            };
            typedef UPower Interface;
            typedef void ArgumentType;
        };
        struct Resuming
        {
            inline static std::string name()
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

        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
    };

    struct Device
    {
        struct GetHistory
        {
            inline static std::string name()
            {
                return "GetHistory";
            } typedef Device Interface;
            inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
        };
        struct GetStatistics
        {
            inline static std::string name()
            {
                return "GetStatistics";
            } typedef Device Interface;
            inline static const std::chrono::milliseconds default_timeout() { return std::chrono::seconds{1}; }
        };
        struct Properties
        {
            struct NativePath
            {
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
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
                inline static std::string name()
                {
                    return "Changed";
                };
                typedef Device Interface;
                typedef void ArgumentType;
            };
        };
    };

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
    inline static const std::string& interface_name()
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
    inline static const std::string& interface_name()
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
