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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_MATCH_RULE_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_MATCH_RULE_H_

#include "org/freedesktop/dbus/message.h"
#include "org/freedesktop/dbus/types/object_path.h"

#include <map>
#include <sstream>
#include <string>

namespace org
{
namespace freedesktop
{
namespace dbus
{
/**
 * @brief Wraps a DBus match rule.
 */
class MatchRule
{
private:
    struct Comma
    {
        Comma() : is_required(false)
        {
        }

        friend std::ostream& operator<<(std::ostream& out, const Comma& c)
        {
            if (c.is_required)
                out << ",";
            c.is_required = !c.is_required;
            return out;
        }

        mutable bool is_required;
    };
public:
    /**
     * @brief Constructs an empty match rule.
     */
    inline MatchRule()
    {
    }

    /**
     * @brief Adjusts the message type that this rule applies to.
     * @param t The new type
     * @return The match rule instance.
     */
    inline MatchRule& type(Message::Type t)
    {
        d.type = t;
        return *this;
    }

    /**
     * @brief Adjusts the message type that this rule applies to.
     * @param t The new type
     * @return A new match rule instance.
     */
    inline MatchRule type(Message::Type t) const
    {
        MatchRule result {*this};
        result.d.type = t;
        return result;
    }

    /**
     * @brief Adjusts the sender that this rule applies to.
     * @param s The new sender
     * @return The match rule instance.
     */
    inline MatchRule& sender(const std::string& s)
    {
        d.sender = s;
        return *this;
    }

    /**
     * @brief Adjusts the sender that this rule applies to.
     * @param s The new sender
     * @return A new match rule instance.
     */
    inline MatchRule sender(const std::string& s) const
    {
        MatchRule result {*this};
        result.d.sender = s;
        return result;
    }

    /**
     * @brief Adjusts the interface that this rule applies to.
     * @param i The new interface.
     * @return The match rule instance.
     */
    inline MatchRule& interface(const std::string& i)
    {
        d.interface = i;
        return *this;
    }

    /**
     * @brief Adjusts the interface that this rule applies to.
     * @param i The new interface.
     * @return A new match rule instance.
     */
    inline MatchRule interface(const std::string& i) const
    {
        MatchRule result {*this};
        result.d.interface = i;
        return result;
    }

    /**
     * @brief Adjusts the member that this rule applies to.
     * @param m The new member.
     * @return The match rule instance.
     */
    inline MatchRule& member(const std::string& m)
    {
        d.member = m;
        return *this;
    }

    /**
     * @brief Adjusts the member that this rule applies to.
     * @param m The new member.
     * @return A new match rule instance.
     */
    inline MatchRule member(const std::string& m) const
    {
        MatchRule result {*this};
        result.d.member = m;
        return result;
    }

    /**
     * @brief Adjusts the path that this rule applies to.
     * @param p The new path.
     * @return The match rule instance.
     */
    inline MatchRule& path(const types::ObjectPath& p)
    {
        d.path = p;
        return *this;
    }

    /**
     * @brief Adjusts the path that this rule applies to.
     * @param p The new path.
     * @return A new match rule instance.
     */
    inline MatchRule path(const types::ObjectPath& p) const
    {
        MatchRule result {*this};
        return result.path(p);
    }

    /**
     * @brief Constructs a valid match rule string from this instance.
     * @return A string formatted according to DBus match rule rules.
     */
    inline std::string as_string() const
    {
        static const std::map<Message::Type, std::string> lut =
        {
            {Message::Type::signal, "signal"},
            {Message::Type::method_call, "method_call"},
            {Message::Type::method_return, "method_return"},
            {Message::Type::error, "error"}
        };
        Comma comma;
        std::stringstream ss;
        if (d.type != Message::Type::invalid)
            ss << "type='" << lut.at(d.type) << "'" << comma;
        if (!d.sender.empty())
            ss << comma << "sender='" << d.sender << "'" << comma;
        if (!d.interface.empty())
            ss << comma << "interface='" << d.interface << "'" << comma;
        if (!d.member.empty())
            ss << comma << "member='" << d.member << "'" << comma;
        if (!d.path.empty())
            ss << comma << "path='" << d.path.as_string() << "'" << comma;

        return ss.str();
    }
private:
    struct Private
    {
        Private() : type(Message::Type::invalid)
        {
        }

        Message::Type type;
        std::string sender;
        std::string interface;
        std::string member;
        types::ObjectPath path;
    } d;
};
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_MATCH_RULE_H_
