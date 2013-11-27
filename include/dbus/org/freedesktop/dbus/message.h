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

#include <org/freedesktop/dbus/argument_type.h>
#include <org/freedesktop/dbus/visibility.h>

#include <org/freedesktop/dbus/types/any.h>
#include <org/freedesktop/dbus/types/object_path.h>
#include <org/freedesktop/dbus/types/signature.h>
#include <org/freedesktop/dbus/types/unix_fd.h>
#include <org/freedesktop/dbus/types/variant.h>

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
template<typename T> struct Codec;
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
        ~Reader();

        Reader(const Reader&) = delete;
        Reader& operator=(const Reader&) = delete;

        Reader(Reader&&);
        Reader& operator=(Reader&&);

        /**
          * @brief Returns the current type.
          *
          */
        ArgumentType type() const;

        /**
          * @brief Advances this view into the message.
          */
        void pop();

        /**
         * @brief Reads a byte from the underlying message.
         */
        std::int8_t pop_byte();

        /**
         * @brief Reads a boolean from the underlying message.
         */
        bool pop_boolean();

        /**
         * @brief Reads an int16 from the underlying message.
         */
        std::int16_t pop_int16();

        /**
         * @brief Reads a uint16 from the underlying message.
         */
        std::uint16_t pop_uint16();

        /**
         * @brief Reads an int32 from the underlying message.
         */
        std::int32_t pop_int32();

        /**
         * @brief Reads a uint32 from the underlying message.
         */
        std::uint32_t pop_uint32();

        /**
         * @brief Reads an int64 from the underlying message.
         */
        std::int64_t pop_int64();

        /**
         * @brief Reads a uint64 from the underlying message.
         */
        std::uint64_t pop_uint64();

        /**
         * @brief Reads a floating point value from the underlying message.
         */
        double pop_floating_point();

        /**
         * @brief Reads a string from the underlying message.
         */
        const char* pop_string();

        /**
         * @brief Reads an object_path from the underlying message.
         */
        types::ObjectPath pop_object_path();

        /**
         * @brief Reads a signature from the underlying message.
         */
        types::Signature pop_signature();

        /**
         * @brief Reads a unix fd from the underlying message.
         */
        types::UnixFd pop_unix_fd();

        /**
         * @brief Prepares reading of an array from the underlying message.
         * @return A reader pointing to the array.
         */
        Reader pop_array();

        /**
         * @brief Prepares reading of a structure from the underlying message.
         * @return A reader pointing into the structure.
         */
        Reader pop_structure();

        /**
         * @brief Prepares reading of a variant from the underlying message.
         * @return A reader pointing into the variant.
         */
        Reader pop_variant();

        /**
         * @brief Prepares reading of a dict entry from the underlying message.
         * @return A reader pointing to the array.
         */
        Reader pop_dict_entry();

    private:
        friend struct Codec<types::Any>;
        friend class Message;
        explicit Reader(const std::shared_ptr<Message>& msg);

        const std::shared_ptr<Message>& access_message();

        struct Private;
        std::unique_ptr<Private> d;
    };

    /**
     * @brief The Writer class allows type-safe serialization of input arguments to a message.
     */
    class Writer
    {
    public:
        ~Writer();
        Writer(const Writer&) = delete;
        Writer& operator=(const Writer&) = delete;

        Writer(Writer&&);
        Writer& operator=(Writer&&);

        /**
         * @brief Writes a byte to the underlying message.
         */
        void push_byte(std::int8_t value);

        /**
         * @brief Writes a boolean to the underlying message.
         */
        void push_boolean(bool value);

        /**
         * @brief Writes an int16 to the underlying message.
         */
        void push_int16(std::int16_t value);

        /**
         * @brief Writes a uint16 to the underlying message.
         */
        void push_uint16(std::uint16_t value);

        /**
         * @brief Writes an int32 to the underlying message.
         */
        void push_int32(std::int32_t value);

        /**
         * @brief Writes a uint32 to the underlying message.
         */
        void push_uint32(std::uint32_t value);

        /**
         * @brief Writes an int64 to the underlying message.
         */
        void push_int64(std::int64_t value);

        /**
         * @brief Writes a uint64 to the underlying message.
         */
        void push_uint64(std::uint64_t value);

        /**
         * @brief Writes a floating point value to the underlying message.
         */
        void push_floating_point(double value);

        /**
         * @brief Writes a string to the underlying message.
         */
        void push_stringn(const char* value, std::size_t size);

        /**
         * @brief Writes an object_path to the underlying message.
         */
        void push_object_path(const types::ObjectPath& value);

        /**
         * @brief Writes a signature to the underlying message.
         */
        void push_signature(const types::Signature& value);

        /**
         * @brief Writes a unix fd to the underlying message.
         */
        void push_unix_fd(const types::UnixFd& value);

        /**
         * @brief Prepares writing of an array to the underlying message.
         * @param [in] signature The signature of the contained data type.
         */
        Writer open_array(const types::Signature& signature);

        /**
         * @brief Finalizes writing of an array to the underlying message.
         */
        void close_array(Writer writer);

        /**
         * @brief Prepares writing of a structure to the underlying message.
         */
        Writer open_structure();

        /**
         * @brief Finalizes writing of a structure to the underlying message.
         */
        void close_structure(Writer writer);

        /**
         * @brief Prepares writing of a variant to the underlying message.
         * @param [in] signature The signature of the contained data type.
         */
        Writer open_variant(const types::Signature& signature);

        /**
         * @brief Finalizes writing of a variant to the underlying message.
         */
        void close_variant(Writer writer);

        /**
         * @brief Prepares writing of a dict entry to the underlying message.
         */
        Writer open_dict_entry();

        /**
         * @brief Finalizes writing of a dict entry to the underlying message.
         */
        void close_dict_entry(Writer writer);

    private:
        friend class Message;
        explicit Writer(const std::shared_ptr<Message>& msg);

        struct Private;
        std::unique_ptr<Private> d;
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
        const types::ObjectPath& path,
        const std::string& interface,
        const std::string& method);

    /**
     * @brief make_method_return creates a message instance in response to a raw DBus message of type method-call.
     * @param msg The message to reply to, must not be null. Must be of type Type::method_call.
     * @return An instance of message of type Type::method_return.
     */
    static std::shared_ptr<Message> make_method_return(const Message::Ptr& msg);

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
        const Message::Ptr& in_reply_to,
        const std::string& error_name,
        const std::string& error_desc);

    /**
     * @brief from_raw_message creates an instance of message from a raw message.
     * @param msg The message to wrap.
     * @return An instance of Message with a type corresponding to the type of the raw message.
     */
    static std::shared_ptr<Message> from_raw_message(DBusMessage* msg);

    ~Message();

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
     * @brief Meant for testing purposes only.
     */
    void ensure_serial_larger_than_zero_for_testing();

private:
    friend class Bus;
    friend struct Codec<types::Any>;

    std::shared_ptr<Message> clone();

    struct Private;
    std::unique_ptr<Private> d;

    Message(std::unique_ptr<Private> d);
};
typedef std::shared_ptr<Message> MessagePtr;
typedef std::unique_ptr<Message> MessageUPtr;
}
}
}

namespace std
{
template<>
struct hash<org::freedesktop::dbus::Message::Type>
{
    size_t operator()(const org::freedesktop::dbus::Message::Type& type) const
    {
        static const hash<int> h {};
        return h(static_cast<int>(type));
    }
};
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_MESSAGE_H_
