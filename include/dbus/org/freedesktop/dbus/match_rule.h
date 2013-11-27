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
#ifndef CORE_DBUS_MATCH_RULE_H_
#define CORE_DBUS_MATCH_RULE_H_

#include <org/freedesktop/dbus/message.h>
#include <org/freedesktop/dbus/visibility.h>

#include <org/freedesktop/dbus/types/object_path.h>

#include <memory>
#include <string>

namespace core
{
namespace dbus
{
/**
 * @brief Wraps a DBus match rule.
 */
class ORG_FREEDESKTOP_DBUS_DLL_PUBLIC MatchRule
{
public:
    /**
     * @brief Constructs a valid match rule.
     */
    MatchRule();
    ~MatchRule();
    MatchRule(const MatchRule& rhs);
    MatchRule& operator=(const MatchRule& rhs);

    /**
     * @brief Adjusts the message type that this rule applies to.
     * @param t The new type
     * @return The match rule instance.
     */
    MatchRule& type(Message::Type t);

    /**
     * @brief Adjusts the message type that this rule applies to.
     * @param t The new type
     * @return A new match rule instance.
     */
    MatchRule type(Message::Type t) const;

    /**
     * @brief Adjusts the sender that this rule applies to.
     * @param s The new sender
     * @return The match rule instance.
     */
    MatchRule& sender(const std::string& s);

    /**
     * @brief Adjusts the sender that this rule applies to.
     * @param s The new sender
     * @return A new match rule instance.
     */
    MatchRule sender(const std::string& s) const;

    /**
     * @brief Adjusts the interface that this rule applies to.
     * @param i The new interface.
     * @return The match rule instance.
     */
    MatchRule& interface(const std::string& i);

    /**
     * @brief Adjusts the interface that this rule applies to.
     * @param i The new interface.
     * @return A new match rule instance.
     */
    MatchRule interface(const std::string& i) const;

    /**
     * @brief Adjusts the member that this rule applies to.
     * @param m The new member.
     * @return The match rule instance.
     */
    MatchRule& member(const std::string& m);

    /**
     * @brief Adjusts the member that this rule applies to.
     * @param m The new member.
     * @return A new match rule instance.
     */
    MatchRule member(const std::string& m) const;

    /**
     * @brief Adjusts the path that this rule applies to.
     * @param p The new path.
     * @return The match rule instance.
     */
    MatchRule& path(const types::ObjectPath& p);

    /**
     * @brief Adjusts the path that this rule applies to.
     * @param p The new path.
     * @return A new match rule instance.
     */
    MatchRule path(const types::ObjectPath& p) const;

    /**
     * @brief Constructs a valid match rule string from this instance.
     * @return A string formatted according to DBus match rule rules.
     */
    std::string as_string() const;

private:
    struct Private;
    std::unique_ptr<Private> d;
};
}
}

#endif // CORE_DBUS_MATCH_RULE_H_
