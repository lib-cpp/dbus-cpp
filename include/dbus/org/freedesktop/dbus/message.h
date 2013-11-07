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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_MESSAGE_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_MESSAGE_H_

#include <org/freedesktop/dbus/codec.h>
#include <org/freedesktop/dbus/visibility.h>

#include <org/freedesktop/dbus/types/object_path.h>

#include <dbus/dbus.h>

#include <exception>
#include <map>
#include <memory>
#include <ostream>
#include <stdexcept>

namespace org
{
namespace freedesktop
{
namespace dbus
{
class Error;

/**
 * @brief The Message class wraps a raw DBus message
 */
class ORG_FREEDESKTOP_DBUS_DLL_PUBLIC Message : public std::enable_shared_from_this<Message>
{
public:
    typedef std::shared_ptr<Message> Ptr;

    /**
     * @brief The Type enum models the type of the message.
     */
    enum class Type : int
    {
        invalid = DBUS_MESSAGE_TYPE_INVALID, ///< Invalid message type
        signal = DBUS_MESSAGE_TYPE_SIGNAL, ///< A signal message
        method_call = DBUS_MESSAGE_TYPE_METHOD_CALL, ///< A method-call message
        method_return = DBUS_MESSAGE_TYPE_METHOD_RETURN, ///< A method-return message
        error = DBUS_MESSAGE_TYPE_ERROR ///< An error message.
    };

    /**
     * @brief The Reader class allows type-safe reading of arguments from a message.
     */
    class Reader
    {
    public:
        Reader(const Reader&) = default;
        Reader& operator=(const Reader&) = default;

        /**
         * @brief Reads an instance of type T from the underlying message and advances the iterator into the message.
         * @tparam T The type to read from the underlying message.
         * @param [out] t The instance of T to read to.
         * @return The instance of the reader.
         */
        template<typename T>
        inline Reader& operator>>(T& t);

        /**
         * @brief Reads an instance of type T from the underlying message. Does not advance the iterator into the message.
         * @tparam T The type to read from the underlying message.
         * @param [out] t The instance of T to read to.
         */
        template<typename T>
        inline void peek(T& t);

        /**
         * @brief Reads an instance of type T from the underlying message and advances the iterator into the message.
         * @tparam T The type to read from the underlying message.
         * @param [out] t The instance of T to read to.
         * @return The instance of the reader.
         */
        template<typename T>
        inline Reader& pop(T& t);

    protected:
        friend class Message;
        explicit Reader(const std::shared_ptr<Message>& msg);

    private:
        std::shared_ptr<Message> message;
        DBusMessageIter iter;
    };

    /**
     * @brief The Writer class allows type-safe serialization of input arguments to a message.
     */
    class Writer
    {
    public:
        Writer(const Writer&) = default;
        Writer& operator=(const Writer&) = default;

        /**
         * @brief Writes an instance of type T to the underlying message and advances the iterator into the message.
         * @tparam T The type to write to the underlying message.
         * @param [out] t The instance of T to write.
         * @return The instance of the Writer.
         */
        template<typename T>
        inline Writer& operator<<(const T& t);

        /**
         * @brief Writes an instance of type T to the underlying message and advances the iterator into the message.
         * @tparam T The type to write to the underlying message.
         * @param [out] t The instance of T to write.
         * @return The instance of the Writer.
         */
        template<typename... Args>
        inline Writer& append(const Args& ... args);

    protected:
        friend class Message;
        explicit Writer(const std::shared_ptr<Message>& msg);

    private:
        std::shared_ptr<Message> message;
        DBusMessageIter iter;
    };

    /**
     * @brief make_method_call creates an instance of Message with type Type::method_call.
     * @param destination The name of the remote service to send the message to.
     * @param path The name of the remote object to send the message to.
     * @param interface The interface to route the message to.
     * @param method The actual method that should be invoked
     * @return An instance of message of type Type::method_call.
     * @throw std::runtime_error if any of the parameters violates the DBus specification.
     */
    static std::shared_ptr<Message> make_method_call(
        const std::string& destination,
        const std::string& path,
        const std::string& interface,
        const std::string& method);

    /**
     * @brief make_method_return creates a message instance in response to a raw DBus message of type method-call.
     * @param msg The message to reply to, must not be null. Must be of type Type::method_call.
     * @return An instance of message of type Type::method_return.
     */
    static std::shared_ptr<Message> make_method_return(DBusMessage* msg);

    /**
     * @brief make_signal creates a message instance wrapping a signal emission.
     * @param path The path of the object emitting the signal.
     * @param interface The interface containing the signal.
     * @param signal The actual signal name.
     * @return An instance of message of type Type::signal.
     */
    static std::shared_ptr<Message> make_signal(
        const std::string& path, 
        const std::string& interface, 
        const std::string& signal);

    /**
     * @brief make_error creates an error message instance in response to a raw DBus message of type method-call.
     * @param in_reply_to The message to reply to, must not be null. Must be of type Type::method_call.
     * @param error_name The name of the error.
     * @param error_desc Human-readable description of the error.
     * @return An instance of message of type Type::error.
     */
    static std::shared_ptr<Message> make_error(
        DBusMessage* in_reply_to, 
        const std::string& error_name, 
        const std::string& error_desc);

    /**
     * @brief from_raw_message creates an instance of message from a raw message.
     * @param msg The message to wrap.
     * @return An instance of Message with a type corresponding to the type of the raw message.
     */
    static std::shared_ptr<Message> from_raw_message(DBusMessage* msg);

    /**
     * @brief Queries the type of the message.
     */
    Type type() const;

    /**
     * @brief Checks if the message expects a reply, i.e., is of type Type::method_call.
     */
    bool expects_reply() const;

    /**
     * @brief Queries the path of the object that this message belongs to.
     */
    types::ObjectPath path() const;

    /**
     * @brief Queries the member name that this message corresponds to.
     */
    std::string member() const;

    /**
     * @brief Queries the type signature of this message.
     */
    std::string signature() const;

    /**
     * @brief Queries the interface name that this message corresponds to.
     */
    std::string interface() const;

    /**
     * @brief Queries the name of the destination that this message should go to.
     */
    std::string destination() const;

    /**
     * @brief Queries the name of the sender that this message originates from.
     */
    std::string sender() const;

    /**
      * @brief Extracts error information from the message.
      * @throw std::runtime_error if not an error message.
      */
    Error error() const;

    /**
     * @brief Creates a Reader instance to read from this message.
     */
    Reader reader();

    /**
     * @brief Creates a Writer instance to write to this message.
     */
    Writer writer();

    /**
     * @brief Extracts the raw DBus message contained within this instance. Use with care.
     */
    DBusMessage* get() const;

private:
    Message(
        DBusMessage* msg, 
        bool ref_on_construction = false);
    
    std::shared_ptr<DBusMessage> dbus_message;
};

std::ostream& operator<<(std::ostream& out, const Message::Type& type);
}
}
}

#include "impl/message.h"

#endif // DBUS_ORG_FREEDESKTOP_DBUS_MESSAGE_H_
