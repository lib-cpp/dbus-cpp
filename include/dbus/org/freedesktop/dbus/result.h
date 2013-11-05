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

#include "org/freedesktop/dbus/codec.h"

#include <dbus/dbus.h>

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
struct Result
{
    /**
     * @brief from_message parses the result from a raw dbus message.
     * @throw std::runtime_error in case of errors.
     * @param msg The message to parse the result from.
     */
    inline void from_message(DBusMessage* msg)
    {
        if (dbus_message_get_type(msg) == DBUS_MESSAGE_TYPE_METHOD_CALL)
            throw std::runtime_error("Cannot construct result from method call");
        if (dbus_message_get_type(msg) == DBUS_MESSAGE_TYPE_SIGNAL)
            throw std::runtime_error("Cannot construct result from signal");

        if (dbus_message_get_type(msg) == DBUS_MESSAGE_TYPE_ERROR)
        {
            struct Scope
            {
                Scope()
                {
                    dbus_error_init(std::addressof(error));
                }

                ~Scope()
                {
                    dbus_error_free(std::addressof(error));
                }

                DBusError error;
            } scope;

            dbus_set_error_from_message(std::addressof(scope.error), msg);
            set_error(std::string(scope.error.name) + ": " + std::string(scope.error.message));
        }
        else if (dbus_message_get_type(msg) == DBUS_MESSAGE_TYPE_METHOD_RETURN)
        {
            reset_error();
            DBusMessageIter it;
            if (!dbus_message_iter_init(msg, std::addressof(it)))
                throw std::runtime_error("Could not initialize message iterator");
            try
            {
                decode_message(std::addressof(it), d.value);
            }
            catch (const std::runtime_error& e)
            {
                set_error(e);
            }
        }
    }

    /**
     * @brief Check if the result is an error.
     * @return true if the invocation returned an error, false otherwise.
     */
    inline bool is_error() const
    {
        return d.is_error;
    }

    /**
     * @brief Accesses the contained error message if any.
     * @return A string describing the error state. Can be empty if no error occured.
     */
    inline const std::string& error() const
    {
        return d.what;
    }

    /**
     * @brief Adjusts the error contained within this object.
     * @param [in] The new error description.
     */
    inline void set_error(const std::runtime_error& error)
    {
        set_error(error.what());
    }

    /**
     * @brief Adjusts the error contained within this object.
     * @param [in] The new error description.
     */
    inline void set_error(const std::string& error)
    {
        d.is_error = true;
        d.what = error;
    }

    /**
     * @brief Resets the error state.
     * @post is_error() returns false and error() returns an empty string.
     */
    inline void reset_error()
    {
        d.is_error = false;
        d.what.clear();
    }

    /**
     * @brief Non-mutable access to the contained value.
     * @return A non-mutable reference to the contained value.
     */
    inline const T& value() const
    {
        return d.value;
    }

    struct Private
    {
        Private() : is_error(false), value()
        {
        }

        bool is_error;
        std::string what;
        T value;
    } d;
};

/**
 * @brief Wraps a remote method invocation and its error state. Template specialization for void results.
 */
template<>
struct Result<void>
{
    /**
     * @brief from_message parses the result from a raw dbus message.
     * @throw std::runtime_error in case of errors.
     * @param msg The message to parse the result from.
     */
    inline void from_message(DBusMessage* msg)
    {
        if (dbus_message_get_type(msg) == DBUS_MESSAGE_TYPE_METHOD_CALL)
            throw std::runtime_error("Cannot construct result from method call");
        if (dbus_message_get_type(msg) == DBUS_MESSAGE_TYPE_SIGNAL)
            throw std::runtime_error("Cannot construct result from signal");

        if (dbus_message_get_type(msg) == DBUS_MESSAGE_TYPE_ERROR)
        {
            struct Scope
            {
                Scope()
                {
                    dbus_error_init(std::addressof(error));
                }

                ~Scope()
                {
                    dbus_error_free(std::addressof(error));
                }

                DBusError error;
            } scope;

            dbus_set_error_from_message(std::addressof(scope.error), msg);
            set_error(std::string(scope.error.name) + ": " + std::string(scope.error.message));
        }
        else if (dbus_message_get_type(msg) == DBUS_MESSAGE_TYPE_METHOD_RETURN)
        {
            reset_error();
            DBusMessageIter iter;
            if (dbus_message_iter_init(msg, std::addressof(iter)))
            {
                throw std::runtime_error("Expected an empty reply, received one containing values.");
            }
        }
    }

    /**
     * @brief Check if the result is an error.
     * @return true if the invocation returned an error, false otherwise.
     */
    inline bool is_error() const
    {
        return d.is_error;
    }

    /**
     * @brief Accesses the contained error message if any.
     * @return A string describing the error state. Can be empty if no error occured.
     */
    inline const std::string& error() const
    {
        return d.what;
    }

    /**
     * @brief Adjusts the error contained within this object.
     * @param [in] The new error description.
     */
    inline void set_error(const std::runtime_error& error)
    {
        set_error(error.what());
    }

    /**
     * @brief Adjusts the error contained within this object.
     * @param [in] The new error description.
     */
    inline void set_error(const std::string& s)
    {
        d.is_error = true;
        d.what = s;
    }

    /**
     * @brief Resets the error state.
     * @post is_error() returns false and error() returns an empty string.
     */
    inline void reset_error()
    {
        d.is_error = false;
        d.what.clear();
    }

    struct Private
    {
        Private() : is_error(false)
        {
        }

        bool is_error;
        std::string what;
    } d;
};
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_RESULT_H_
