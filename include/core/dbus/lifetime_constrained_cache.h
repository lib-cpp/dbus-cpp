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
#ifndef CORE_DBUS_LIFETIME_CONSTRAINED_CACHE_H_
#define CORE_DBUS_LIFETIME_CONSTRAINED_CACHE_H_

#include <core/signal.h>

#include <memory>
#include <tuple>
#include <unordered_map>

namespace core
{
namespace dbus
{

template<typename T>
struct LifetimeConstraintTraits
{
    static inline const core::Signal<void>& access_signal_object_about_to_be_destroyed(const T& t)
    {
        return t.about_to_be_destroyed();
    }
};

template<typename Key, typename Value>
class ThreadSafeLifetimeConstrainedCache
{
public:
    ThreadSafeLifetimeConstrainedCache() = default;
    ~ThreadSafeLifetimeConstrainedCache()
    {
        std::lock_guard<std::mutex> lg(guard);
        for (auto& element : cache)
        {
            std::get<1>(element.second).disconnect();
        }
    }

    ThreadSafeLifetimeConstrainedCache(const ThreadSafeLifetimeConstrainedCache&) = delete;
    ThreadSafeLifetimeConstrainedCache& operator=(const ThreadSafeLifetimeConstrainedCache&) = delete;

    inline std::shared_ptr<Value> retrieve_value_for_key(const Key& key)
    {
        std::lock_guard<std::mutex> lg(guard);

        auto it = cache.find(key);
        if (it == cache.end())
            return std::shared_ptr<Value>{};

        return std::get<0>(it->second).lock();
    }

    inline bool insert_value_for_key(const Key& key, const std::shared_ptr<Value>& value)
    {
        std::lock_guard<std::mutex> lg(guard);

        auto it = cache.find(key);

        if (it != cache.end())
            return false;

        auto connection = LifetimeConstraintTraits<Value>::access_signal_object_about_to_be_destroyed(*value).connect([this, key]()
        {
            remove_value_for_key(key);
        });

        bool inserted{false};

        std::tie(std::ignore, inserted) = cache.insert(
                    std::make_pair(
                        key,
                        std::make_tuple(std::weak_ptr<Value>(value), connection)));

        return inserted;
    }

    inline void remove_value_for_key(const Key& key)
    {
        std::lock_guard<std::mutex> lg(guard);
        auto it = cache.find(key);

        if (it == cache.end())
            return;

        cache.erase(it);
    }

    inline bool has_value_for_key(const Key& key) const
    {
        std::lock_guard<std::mutex> lg(guard);
        return cache.find(key) != cache.end();
    }

private:
    mutable std::mutex guard;
    std::unordered_map<Key, std::tuple<std::weak_ptr<Value>, core::Connection>> cache;
};
}
}

#endif // CORE_DBUS_LIFETIME_CONSTRAINED_CACHE_H_
