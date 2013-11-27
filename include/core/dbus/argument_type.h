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
#ifndef CORE_DBUS_ARGUMENT_TYPE_H_
#define CORE_DBUS_ARGUMENT_TYPE_H_

#include <map>
#include <ostream>
#include <string>

#include <dbus/dbus.h>

namespace core
{
namespace dbus
{
/**
 * @brief Wraps the underlying DBus types and exposes them as a strongly typed enum.
 */
enum class ArgumentType : int
{
    byte = DBUS_TYPE_BYTE,
    boolean = DBUS_TYPE_BOOLEAN,
    int16 = DBUS_TYPE_INT16,
    uint16 = DBUS_TYPE_UINT16,
    int32 = DBUS_TYPE_INT32,
    uint32 = DBUS_TYPE_UINT32,
    int64 = DBUS_TYPE_INT64,
    uint64 = DBUS_TYPE_UINT64,
    floating_point = DBUS_TYPE_DOUBLE,
    string = DBUS_TYPE_STRING,
    object_path = DBUS_TYPE_OBJECT_PATH,
    signature = DBUS_TYPE_SIGNATURE,
    unix_fd = DBUS_TYPE_UNIX_FD,
    array = DBUS_TYPE_ARRAY,
    variant = DBUS_TYPE_VARIANT,
    structure = DBUS_TYPE_STRUCT,
    dictionary_entry = DBUS_TYPE_DICT_ENTRY,
    invalid = DBUS_TYPE_INVALID
};

/**
 * @brief Pretty prints ArgumentType to an output stream.
 * @param out The stream to write to.
 * @param type The type to be printed.
 * @return The stream that has been written to.
 */
inline std::ostream& operator<<(std::ostream& out, const ArgumentType& type) noexcept(true)
{
    static const std::map<ArgumentType, std::string> lut =
    {
        {ArgumentType::byte, "byte"},
        {ArgumentType::boolean, "boolean"},
        {ArgumentType::int16, "int16"},
        {ArgumentType::uint16, "uint16"},
        {ArgumentType::int32, "int32"},
        {ArgumentType::uint32, "uint32"},
        {ArgumentType::int64, "int64"},
        {ArgumentType::uint64, "uint64"},
        {ArgumentType::floating_point, "floating_point"},
        {ArgumentType::string, "string"},
        {ArgumentType::object_path, "object_path"},
        {ArgumentType::signature, "signature"},
        {ArgumentType::unix_fd, "unix_fd"},
        {ArgumentType::array, "array"},
        {ArgumentType::variant, "variant"},
        {ArgumentType::structure, "structure"},
        {ArgumentType::dictionary_entry, "dictionary_entry"},
        {ArgumentType::invalid, "invalid"}
    };

    try
    {
        out << lut.at(type);
    } catch(...)
    {
        out << lut.at(ArgumentType::invalid);
    }

    return out;
}
}
}

#endif // CORE_DBUS_ARGUMENT_TYPE_H_
