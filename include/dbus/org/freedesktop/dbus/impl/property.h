/*
 * Copyright © 2012 Canonical Ltd.
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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_IMPL_PROPERTY_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_IMPL_PROPERTY_H_

namespace org
{
namespace freedesktop
{
namespace dbus
{
template<typename PropertyType>
const typename PropertyType::ValueType&
Property<PropertyType>::value()
{
    if (parent->is_stub())
        property_value = parent->invoke_method_synchronously<
            interfaces::Properties::Get,
            types::Variant<typename PropertyType::ValueType>
            >(interface, name).value();
    return property_value.get();
}

template<typename PropertyType>
void
Property<PropertyType>::value(const typename PropertyType::ValueType& new_value)
{
    property_value.set(new_value);
    if (parent->is_stub())
    {
        if (!writable)
            std::runtime_error("Property is not writable");
        parent->invoke_method_synchronously<
            interfaces::Properties::Set,
            void
            >(interface, name, property_value);
    }
}

template<typename PropertyType>
bool
Property<PropertyType>::is_writable() const
{
    return writable;
}

template<typename PropertyType>
std::shared_ptr<Property<PropertyType>>
Property<PropertyType>::make_property(
    const std::shared_ptr<Object>& parent)
{
    return std::shared_ptr<Property<PropertyType>>(
        new Property<PropertyType>(
            parent,
            traits::Service<typename PropertyType::Interface>::interface_name(),
            PropertyType::name(),
            PropertyType::writable));
}

template<typename PropertyType>
Property<PropertyType>::Property(
    const std::shared_ptr<Object>& parent,
    const std::string& interface,
    const std::string& name,
    bool writable)
        : parent(parent),
          interface(interface),
          name(name),
          writable(writable)
{
    if (!parent->is_stub())
    {
        parent->get_property_router.install_route(
            Object::PropertyKey
            {
                traits::Service<typename PropertyType::Interface>::interface_name(),
                PropertyType::name()
            },
            std::bind(&Property::handle_get, this, std::placeholders::_1));
        parent->set_property_router.install_route(
            Object::PropertyKey
            {
                traits::Service<typename PropertyType::Interface>::interface_name(),
                PropertyType::name()
            },
            std::bind(
                &Property::handle_set,
                this,
                std::placeholders::_1));
        parent->install_method_handler<interfaces::Properties::Get>(
            std::bind(
                &Property::handle_get,
                this,
                std::placeholders::_1));
        parent->install_method_handler<interfaces::Properties::Set>(
            std::bind(
                &Property::handle_set,
                this,
                std::placeholders::_1));
    }
}

template<typename PropertyType>
void
Property<PropertyType>::handle_get(DBusMessage* msg)
{
    auto reply = Message::make_method_return(msg);
    reply->writer() << property_value;

    parent->parent->get_connection()->send(reply->get());
}

template<typename PropertyType>
void
Property<PropertyType>::handle_set(DBusMessage* msg)
{
    if (!writable)
    {
        auto error = Message::make_error(
            msg,
            traits::Service<interfaces::Properties>::interface_name() + ".NotWritableError",
            name + "is not writable");

        parent->parent->get_connection()->send(error->get());
        return;
    }

    std::string s;
    auto m = Message::from_raw_message(msg);
    try
    {
        m->reader() >> s >> s >> property_value;
    }
    catch (...)
    {
        auto error = Message::make_error(
            msg,
            traits::Service<interfaces::Properties>::interface_name() + ".NotWritableError",
            name + "is not writable");

        parent->parent->get_connection()->send(error->get());
        return;
    }

    auto reply = Message::make_method_return(msg);
    parent->parent->get_connection()->send(reply->get());
}
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_IMPL_PROPERTY_H_
