/*
 * Copyright © 2013 Canonical Ltd.
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

#include <core/dbus/types/object_path.h>

#include <gtest/gtest.h>

TEST(ObjectPath, comparison_for_equality_yields_correct_result)
{
    core::dbus::types::ObjectPath op1{"/does/not/exist"};
    auto op2 = op1;

    EXPECT_TRUE(op2 == op1);
}

TEST(ObjectPath, comparison_for_inequality_yields_correct_result)
{
    core::dbus::types::ObjectPath op1{"/does/not/exist"};
    core::dbus::types::ObjectPath op2{"/does/not/exist/different"};

    EXPECT_TRUE(op2 != op1);
}
