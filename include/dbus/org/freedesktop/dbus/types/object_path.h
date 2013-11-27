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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_TYPES_OBJECT_PATH_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_TYPES_OBJECT_PATH_H_

#include <org/freedesktop/dbus/visibility.h>

#include <exception>
#include <stdexcept>
#include <string>

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace types
{
/**
 * @brief The ObjectPath class encapsulates a DBus object path.
 */
class ORG_FREEDESKTOP_DBUS_DLL_PUBLIC ObjectPath
{
public:
    /**
     * @brief root() returns the default object path.
     * @return The string representation of the default object path.
     */
    static const std::string& root();

    /**
     * @brief The Errors struct summarizes all exceptions thrown by
     * methods of class ObjectPath.
     */
    struct Errors
    {
        Errors() = delete;

        /**
         * @brief The InvalidObjectPath struct is thrown if a string representation of an object path is invalid.
         */
        struct InvalidObjectPathStringRepresentation : public std::logic_error
        {
            inline InvalidObjectPathStringRepresentation(const std::string& s)
                : std::logic_error(
                      "Could not construct valid object path from provided string: " + s)
            {
            }
        };
    };

    /**
     * @brief Constructs an object path from a string.
     * @throw Errors::InvalidObjectPathStringRepresentation if path is invalid.
     * @param [in] path The string to construct the object path from.
     *
     * Specifying a default argument here causes valgrind to report a "possible lost".
     * However, this is spurious and
     *   http://stackoverflow.com/questions/10750299/if-i-specify-a-default-value-for-an-argument-of-type-stdstring-in-c-cou
     * gives some insight.
     */
    ObjectPath(const std::string& path = ObjectPath::root());

    /**
     * @brief Checks if an object path is empty.
     * @return true iff the object path is empty.
     */
    bool empty() const;

    /**
     * @brief as_string provides a string representation of the object path.
     * @return A non-mutable reference to a string representation of the object path.
     */
    const std::string& as_string() const;

    /**
     * @brief operator < compares two object path instances.
     * @param rhs The right-hand-side of the comparison.
     * @return true iff this instance is smaller than the right-hand-side.
     */
    bool operator<(const ObjectPath& rhs) const;

    /**
     * @brief operator < compares two object path instances for equality.
     * @param rhs The right-hand-side of the comparison.
     * @return true iff this instance equals the right-hand-side.
     */
    bool operator==(const ObjectPath& rhs) const;

private:
    std::string path;
};

/**
 * @brief operator << pretty prints an object path instance.
 * @param out The stream to pretty print to.
 * @param path The instance to be printed.
 * @return The stream that has been written to.
 */
ORG_FREEDESKTOP_DBUS_DLL_PUBLIC std::ostream& operator<<(std::ostream& out, const ObjectPath& path);
}
}
}
}

namespace std
{
/**
 * @brief Enables usage of ObjectPath instances in hashed containers.
 */
template<>
struct ORG_FREEDESKTOP_DBUS_DLL_PUBLIC hash<org::freedesktop::dbus::types::ObjectPath>
{
    /**
     * @brief operator () calculates the hash of an object path instance.
     * @param p The instance to calculate the hash value for.
     */
    size_t operator()(const org::freedesktop::dbus::types::ObjectPath& p) const;
};
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_TYPES_OBJECT_PATH_H_
