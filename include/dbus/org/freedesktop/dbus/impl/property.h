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
Property<PropertyType>::get() const
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
Property<PropertyType>::set(const typename PropertyType::ValueType& new_value)
{
    property_value.set(new_value);
    if (parent->is_stub())
    {
        if (!writable)
            throw std::runtime_error("Property is not writable");
        parent->invoke_method_synchronously<
            interfaces::Properties::Set,
            void
            >(interface, name, property_value);
    }

    std::lock_guard<std::mutex> lg(observers_guard);
    for (auto observer : observers)
        observer(new_value);
}

/**
  * @brief Subscribes to changes to this property.
  * @param observer The observer to be called in the case of changes.
  */
template<typename PropertyType>
inline typename Property<PropertyType>::Token
Property<PropertyType>::subscribe_to_changes(const typename Property<PropertyType>::ChangeObserver& observer)
{
    Token token;
    std::lock_guard<std::mutex> lg(observers_guard);
    token = observers.insert(observers.end(), observer);
    return token;
}

/**
 * @brief Cancel a previous subscription to change notifications.
 * @param token Represents the previous subscription.
 */
template<typename PropertyType>
inline void
Property<PropertyType>::unsubscribe_from_changes(const typename Property<PropertyType>::Token& token)
{
    std::lock_guard<std::mutex> lg(observers_guard);
    observers.erase(token);
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
Property<PropertyType>::handle_get(const Message::Ptr& msg)
{
    auto reply = Message::make_method_return(msg);
    reply->writer() << property_value;

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

    std::string s;
    try
    {
        msg->reader() >> s >> s >> property_value;
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
Property<PropertyType>::handle_changed(const Message::Ptr& msg)
{
    try
    {
        typename PropertyType::ValueType value;
        msg->reader() >> value;
        set(value);
    }
    catch (...)
    {
    }
}
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_IMPL_PROPERTY_H_
