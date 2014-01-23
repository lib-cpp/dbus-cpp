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

#include <core/dbus/lifetime_constrained_cache.h>

#include <gtest/gtest.h>

namespace
{
struct Dummy
{
    const core::Signal<void>& about_to_be_destroyed() const
    {
        return signal_about_to_be_destroyed;
    }

    core::Signal<void> signal_about_to_be_destroyed;
};
}

TEST(Cache, an_inserted_object_is_retrievable)
{
    core::dbus::ThreadSafeLifetimeConstrainedCache<std::string, Dummy> cache;

    auto object = std::make_shared<Dummy>();

    EXPECT_TRUE(cache.insert_value_for_key("key", object));
    EXPECT_TRUE(cache.has_value_for_key("key"));
    EXPECT_EQ(object, cache.retrieve_value_for_key("key"));
}
