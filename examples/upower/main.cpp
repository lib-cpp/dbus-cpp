#include "upower.h"

#include "org/freedesktop/dbus/bus.h"
#include "org/freedesktop/dbus/interfaces/properties.h"
#include "org/freedesktop/dbus/types/struct.h"
#include "org/freedesktop/dbus/types/stl/tuple.h"
#include "org/freedesktop/dbus/types/stl/vector.h"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/moment.hpp>

#include <sys/types.h>
#include <signal.h>

namespace acc = boost::accumulators;
namespace dbus = org::freedesktop::dbus;

namespace
{
dbus::Bus::Ptr the_system_bus()
{
    static dbus::Bus::Ptr system_bus = std::make_shared<dbus::Bus>(dbus::WellKnownBus::system);
    return system_bus;
}
}

int main(int, char**)
{
    auto bus = the_system_bus();
    std::thread t {std::bind(&dbus::Bus::run, bus)};
    auto upower = dbus::Service::use_service(bus, dbus::traits::Service<org::freedesktop::UPower>::interface_name());
    auto upower_object = upower->object_for_path(dbus::types::ObjectPath("/org/freedesktop/UPower"));

    auto all_properties = upower_object->get_all_properties<org::freedesktop::UPower>();
    std::for_each(all_properties.begin(), all_properties.end(), [](const std::pair<const std::string, dbus::types::Variant<dbus::types::Any>>& pair)
    {
        std::cout << pair.first << " -> " << pair.second.get() << std::endl;
    });

    std::cout << upower_object->get_property<org::freedesktop::UPower::Properties::DaemonVersion>()->value() << std::endl;
    std::cout << upower_object->get_property<org::freedesktop::UPower::Properties::CanSuspend>()->value() << std::endl;
    std::cout << upower_object->get_property<org::freedesktop::UPower::Properties::CanHibernate>()->value() << std::endl;
    std::cout << upower_object->get_property<org::freedesktop::UPower::Properties::OnBattery>()->value() << std::endl;
    std::cout << upower_object->get_property<org::freedesktop::UPower::Properties::LidIsClosed>()->value() << std::endl;
    std::cout << upower_object->get_property<org::freedesktop::UPower::Properties::LidIsPresent>()->value() << std::endl;

    auto device_added_signal = upower_object->get_signal<org::freedesktop::UPower::Signals::DeviceAdded>();
    dbus::signals::ScopedConnection sc1
    {
        device_added_signal->connect([](const org::freedesktop::UPower::Signals::DeviceAdded::ArgumentType&)
        {
            std::cout << "org::freedesktop::UPower::Signals::DeviceAdded" << std::endl;
        })
    };
    auto device_removed_signal = upower_object->get_signal<org::freedesktop::UPower::Signals::DeviceRemoved>();
    dbus::signals::ScopedConnection sc2
    {
        device_removed_signal->connect([](const org::freedesktop::UPower::Signals::DeviceRemoved::ArgumentType&)
        {
            std::cout << "org::freedesktop::UPower::Signals::DeviceRemoved" << std::endl;
        })
    };
    auto device_changed_signal = upower_object->get_signal<org::freedesktop::UPower::Signals::DeviceChanged>();
    dbus::signals::ScopedConnection sc3
    {
        device_changed_signal->connect([](const org::freedesktop::UPower::Signals::DeviceChanged::ArgumentType&)
        {
            std::cout << "org::freedesktop::UPower::Signals::DeviceChanged" << std::endl;
        })
    };
    auto changed_signal = upower_object->get_signal<org::freedesktop::UPower::Signals::Changed>();
    dbus::signals::ScopedConnection sc4
    {
        changed_signal->connect([]()
        {
            std::cout << "org::freedesktop::UPower::Signals::Changed" << std::endl;
        })
    };
    auto sleeping_signal = upower_object->get_signal<org::freedesktop::UPower::Signals::Sleeping>();
    dbus::signals::ScopedConnection sc5
    {
        sleeping_signal->connect([]()
        {
            std::cout << "org::freedesktop::UPower::Signals::Sleeping" << std::endl;
        })
    };
    auto resuming_signal = upower_object->get_signal<org::freedesktop::UPower::Signals::Resuming>();
    dbus::signals::ScopedConnection sc6
    {
        resuming_signal->connect([]()
        {
            std::cout << "org::freedesktop::UPower::Signals::Resuming" << std::endl;
        })
    };
    auto devices = upower_object->invoke_method_synchronously<org::freedesktop::UPower::EnumerateDevices, std::vector<dbus::types::ObjectPath>>();
    std::cout << "Devices count: " << devices.value().size() << std::endl;
    std::for_each(devices.value().begin(), devices.value().end(), [upower_object](const dbus::types::ObjectPath& path)
    {
        std::cout << "===================================================================================" << std::endl;
        auto device = upower_object->add_object_for_path(path);
        auto device_changed_signal = device->get_signal<org::freedesktop::UPower::Device::Signals::Changed>();
        device_changed_signal->connect([]()
        {
            std::cout << "org::freedesktop::UPower::Device::Signals::Changed" << std::endl;
        });
        try
        {
            auto stats = device->invoke_method_synchronously<
                         org::freedesktop::UPower::Device::GetStatistics,
                         std::vector<dbus::types::Struct<std::tuple<double, double>>>,
                         std::string>("charging");
            std::for_each(stats.value().begin(), stats.value().end(), [](const dbus::types::Struct<std::tuple<double, double>>& t)
            {
                std::cout << std::get<0>(t.value) << ", " << std::get<1>(t.value) << std::endl;
            });
        }
        catch (const std::runtime_error& e)
        {
            std::cout << e.what() << std::endl;
        }

        try
        {
            acc::accumulator_set<double, acc::stats<acc::tag::mean, acc::tag::moment<2> > > as;
            auto history = device->invoke_method_synchronously<
                           org::freedesktop::UPower::Device::GetHistory,
                           std::vector<dbus::types::Struct<std::tuple<uint32_t, double, uint32_t>>>,
                           std::string, uint32_t, uint32_t
                           >("charge", 0, 50);
            std::for_each(history.value().begin(), history.value().end(), [&as](const dbus::types::Struct<std::tuple<uint32_t, double, uint32_t>>& t)
            {
                as(std::get<1>(t.value));
            });
            std::cout << "Load statistics(Mean: " << acc::mean(as) << ", 2nd Moment: " << std::sqrt(acc::moment<2>(as)) << ")" << std::endl;
        }
        catch (const std::runtime_error& e)
        {
            std::cout << e.what() << std::endl;
        }

        std::cout << "Manual: " << path << std::endl;
        std::cout << "\tVendor: " << device->get_property<org::freedesktop::UPower::Device::Properties::Vendor>()->value() << std::endl;
        std::cout << "\tModel: " << device->get_property<org::freedesktop::UPower::Device::Properties::Model>()->value() << std::endl;
        std::cout << "\tSerial: " << device->get_property<org::freedesktop::UPower::Device::Properties::Serial>()->value() << std::endl;
        std::cout << "\tUpdateTime: " << device->get_property<org::freedesktop::UPower::Device::Properties::UpdateTime>()->value() << std::endl;
        std::cout << "\tType: " << device->get_property<org::freedesktop::UPower::Device::Properties::Type>()->value() << std::endl;
        std::cout << "\tPowerSupply: " << device->get_property<org::freedesktop::UPower::Device::Properties::PowerSupply>()->value() << std::endl;
        std::cout << "\tHasHistory: " << device->get_property<org::freedesktop::UPower::Device::Properties::HasHistory>()->value() << std::endl;
        std::cout << "\tHasStatistics: " << device->get_property<org::freedesktop::UPower::Device::Properties::HasStatistics>()->value() << std::endl;
        std::cout << "\tOnline: " << device->get_property<org::freedesktop::UPower::Device::Properties::Online>()->value() << std::endl;
        std::cout << "\tEnergy: " << device->get_property<org::freedesktop::UPower::Device::Properties::Energy>()->value() << std::endl;
        std::cout << "\tEnergyEmpty: " << device->get_property<org::freedesktop::UPower::Device::Properties::EnergyEmpty>()->value() << std::endl;
        std::cout << "\tEnergyFull: " << device->get_property<org::freedesktop::UPower::Device::Properties::EnergyFull>()->value() << std::endl;
        std::cout << "\tEnergyFullDesign: " << device->get_property<org::freedesktop::UPower::Device::Properties::EnergyFullDesign>()->value() << std::endl;
        std::cout << "\tEnergyRate: " << device->get_property<org::freedesktop::UPower::Device::Properties::EnergyRate>()->value() << std::endl;
        std::cout << "\tVoltage: " << device->get_property<org::freedesktop::UPower::Device::Properties::Voltage>()->value() << std::endl;
        //std::cout << "\tTimeToEmpty: " << device->get_property<UPower::Device::Properties::TimeToEmpty>()->value() << std::endl;
        //std::cout << "\tTimeToFull: " << device->get_property<UPower::Device::Properties::TimeToFull>()->value() << std::endl;
        std::cout << "\tPercentage: " << device->get_property<org::freedesktop::UPower::Device::Properties::Percentage>()->value() << std::endl;
        std::cout << "\tIsPresent: " << device->get_property<org::freedesktop::UPower::Device::Properties::IsPresent>()->value() << std::endl;
        std::cout << "\tState: " << device->get_property<org::freedesktop::UPower::Device::Properties::State>()->value() << std::endl;
        std::cout << "\tIsRechargeable: " << device->get_property<org::freedesktop::UPower::Device::Properties::IsRechargeable>()->value() << std::endl;
        std::cout << "\tCapacity: " << device->get_property<org::freedesktop::UPower::Device::Properties::Capacity>()->value() << std::endl;
        std::cout << "\tTechnology: " << device->get_property<org::freedesktop::UPower::Device::Properties::Technology>()->value() << std::endl;
        std::cout << "\tRecallNotice: " << device->get_property<org::freedesktop::UPower::Device::Properties::RecallNotice>()->value() << std::endl;
        std::cout << "\tRecallVendor: " << device->get_property<org::freedesktop::UPower::Device::Properties::RecallVendor>()->value() << std::endl;
        std::cout << "\tRecallUrl: " << device->get_property<org::freedesktop::UPower::Device::Properties::RecallUrl>()->value() << std::endl;
        std::cout << "-----------------------------------------------------------------------------------" << std::endl;
        auto properties = device->get_all_properties<org::freedesktop::UPower::Device>();
        std::for_each(properties.begin(), properties.end(), [](const std::pair<const std::string, dbus::types::Variant<dbus::types::Any>>& pair)
        {
            std::cout << "\t" << pair.first << " -> " << pair.second.get() << std::endl;
        });
        std::cout << "===================================================================================" << std::endl;
    });

    sigset_t signal_set;
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGINT);
    int signal;
    sigwait(&signal_set, &signal);

    bus->stop();

    if (t.joinable())
        t.join();
}
