#include "org/freedesktop/dbus/match_rule.h"

#include <gtest/gtest.h>

TEST(MatchRule, ConstructingAMatchRuleYieldsCorrectResult)
{
    org::freedesktop::dbus::MatchRule rule;
    rule
    .type(org::freedesktop::dbus::Message::Type::signal)
    .sender("org.freedesktop.DBus")
    .interface("org.freedesktop.DBus")
    .member("ListNames")
    .path(org::freedesktop::dbus::types::ObjectPath("/org/freedesktop/DBus"));

    const std::string expected_rule
    {"type='signal',sender='org.freedesktop.DBus',interface='org.freedesktop.DBus',member='ListNames',path='/org/freedesktop/DBus'"
    };

    EXPECT_EQ(expected_rule, rule.as_string());
}
