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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_RESULT_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_RESULT_H_

#include <org/freedesktop/dbus/codec.h>
#include <org/freedesktop/dbus/error.h>
#include <org/freedesktop/dbus/message.h>

#include <stdexcept>
#include <string>

namespace org
{
namespace freedesktop
{
namespace dbus
{
/**
 * @brief Wraps a remote method invocation, its error state and result.
 * @tparam T Result type.
 */
template<typename T>
class Result
{
public:
    /**
     * @brief from_message parses the result from a raw dbus message.
     * @throw std::runtime_error in case of errors.
     * @param message The message to parse the result from.
     */
    inline static Result from_message(const Message::Ptr& message)
    {
        Result result;

        switch(message->type())
        {
        case Message::Type::method_call:
            throw std::runtime_error("Cannot construct result from method call");
            break;
        case Message::Type::signal:
            throw std::runtime_error("Cannot construct result from signal");
            break;
        case Message::Type::error:
            result.d.error = message->error();
            break;
        case Message::Type::method_return:
            message->reader() >> result.d.value;
            break;
        default:
            break;
        }

        return std::move(result);
    }

    /**
     * @brief Check if the result is an error.
     * @return true if the invocation returned an error, false otherwise.
     */
    inline bool is_error() const
    {
        return static_cast<bool>(d.error);
    }

    /**
     * @brief Accesses the contained error message if any.
     * @return A string describing the error state. Can be empty if no error occured.
     */
    inline const Error& error() const
    {
        return d.error;
    }

    /**
     * @brief Non-mutable access to the contained value.
     * @return A non-mutable reference to the contained value.
     */
    inline const T& value() const
    {
        return d.value;
    }

private:
    struct Private
    {
        Error error;
        T value;
    } d;
};

/**
 * @brief Wraps a remote method invocation and its error state. Template specialization for void results.
 */
template<>
class Result<void>
{
public:
    /**
     * @brief from_message parses the result from a raw dbus message.
     * @throw std::runtime_error in case of errors.
     * @param message The message to parse the result from.
     */
    inline static Result from_message(const Message::Ptr& message)
    {
        Result result;

        switch(message->type())
        {
        case Message::Type::method_call:
            throw std::runtime_error("Cannot construct result from method call");
            break;
        case Message::Type::signal:
            throw std::runtime_error("Cannot construct result from signal");
            break;
        case Message::Type::error:
            result.d.error = message->error();
            break;
        case Message::Type::method_return:
        default:
            break;
        }

        return std::move(result);
    }

    /**
     * @brief Check if the result is an error.
     * @return true if the invocation returned an error, false otherwise.
     */
    inline bool is_error() const
    {
        return static_cast<bool>(d.error);
    }

    /**
     * @brief Accesses the contained error message if any.
     * @return A string describing the error state. Can be empty if no error occured.
     */
    inline const Error& error() const
    {
        return d.error;
    }

private:
    struct Private
    {
        Error error;
    } d;
};
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_RESULT_H_
