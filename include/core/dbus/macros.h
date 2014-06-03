/*
 * Copyright © 2014 Canonical Ltd.
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

#ifndef CORE_DBUS_MACROS_H_
#define CORE_DBUS_MACROS_H_

#include <core/dbus/types/object_path.h>

#include <chrono>
#include <string>

#define SECONDS(seconds) std::chrono::seconds{seconds};

#define DBUS_CPP_METHOD_WITH_TIMEOUT_DEF(Name, Itf, Timeout) \
    struct Name \
    { \
        typedef Itf Interface; \
        inline static const std::string& name() \
        { \
            static const std::string s{#Name}; \
            return s; \
        } \
        inline static const std::chrono::milliseconds default_timeout() { return std::chrono::milliseconds{Timeout}; } \
    };\

#define DBUS_CPP_METHOD_DEF(Name, Itf) DBUS_CPP_METHOD_WITH_TIMEOUT_DEF(Name, Itf, 2000)

#define DBUS_CPP_SIGNAL_DEF(Name, Itf, ArgType) \
    struct Name \
    { \
        inline static std::string name() \
        { \
            return #Name; \
        }; \
        typedef Itf Interface; \
        typedef ArgType ArgumentType; \
    };\

#define DBUS_CPP_READABLE_PROPERTY_DEF(Name, Itf, Type) \
    struct Name \
    { \
        inline static std::string name() \
        { \
            return #Name; \
        }; \
        typedef Itf Interface; \
        typedef Type ValueType; \
        static const bool readable = true; \
        static const bool writable = false; \
    }; \

#define DBUS_CPP_WRITABLE_PROPERTY_DEF(Name, Itf, Type) \
    struct Name \
    { \
        inline static std::string name() \
        { \
            return #Name; \
        }; \
        typedef Itf Interface; \
        typedef Type ValueType; \
        static const bool readable = true; \
        static const bool writable = true; \
    }; \

#endif // MPRIS_MACROS_H_

