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
#ifndef CORE_DBUS_IMPL_PROPERTY_H_
#define CORE_DBUS_IMPL_PROPERTY_H_

namespace core
{
namespace dbus
{
template<typename PropertyType>
const typename Property<PropertyType>::ValueType&
Property<PropertyType>::get() const
{
    if (parent->is_stub())
    {
        Super::mutable_get() = parent->invoke_method_synchronously<
                interfaces::Properties::Get,
                types::Variant<typename PropertyType::ValueType>
                >(interface, name).value().get();
    }
    return Super::get();
}

template<typename PropertyType>
void
Property<PropertyType>::set(const typename Property<PropertyType>::ValueType& new_value)
{
    if (parent->is_stub())
    {
        if (!writable)
        {
            throw std::runtime_error("Property is not writable");
        }

        parent->invoke_method_synchronously<
                interfaces::Properties::Set,
                void
                >(interface, name, types::Variant<ValueType>(new_value));
    }

    Super::set(new_value);
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
    }
}

template<typename PropertyType>
void
Property<PropertyType>::handle_get(const Message::Ptr& msg)
{
    auto reply = Message::make_method_return(msg);
    reply->writer() << types::Variant<ValueType>(Super::get());

    parent->parent->get_connection()->send(reply);
}

template<typename PropertyType>
void
Property<PropertyType>::handle_set(const Message::Ptr& msg)
{
    if (!writable)
    {
        auto error = Message::make_error(
                    msg,
                    traits::Service<interfaces::Properties>::interface_name() + ".NotWritableError",
                    name + "is not writable");

        parent->parent->get_connection()->send(error);
        return;
    }

    std::string s; types::Variant<ValueType> value;
    try
    {
        msg->reader() >> s >> s >> value;
        Super::set(value.get());
    }
    catch (...)
    {
        auto error = Message::make_error(
                    msg,
                    traits::Service<interfaces::Properties>::interface_name() + ".NotWritableError",
                    name + "is not writable");

        parent->parent->get_connection()->send(error);
        return;
    }

    auto reply = Message::make_method_return(msg);
    parent->parent->get_connection()->send(reply);
}

template<typename PropertyType>
void
Property<PropertyType>::handle_changed(const types::Variant<types::Any>& arg)
{
    try
    {
        typename PropertyType::ValueType value;
        arg.get().reader() >> value;
        set(value);
    }
    catch (...)
    {
    }
}
}
}

#endif // CORE_DBUS_IMPL_PROPERTY_H_
