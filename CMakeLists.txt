# Copyright © 2013 Canonical Ltd.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authored by: Thomas Voss <thomas.voss@canonical.com>

cmake_minimum_required(VERSION 2.8)

project(dbus-cpp)

find_package(Boost COMPONENTS filesystem program_options system REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(DBUS dbus-1)
pkg_check_modules(PROCESS_CPP process-cpp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(cmake/EnableCoverageReport.cmake)
include(cmake/PrePush.cmake)

include(GNUInstallDirs)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wall -pedantic -Wextra -fvisibility=hidden -fPIC -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror -Wall -fno-strict-aliasing -fvisibility=hidden -fvisibility-inlines-hidden -pedantic -Wextra -fPIC -pthread")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")

# We leverage GCC's sanitize functionality
set(
  DBUS_CPP_SANITIZE None
  CACHE STRING "Enables sanitizer functionality if set to a value in {address, leak}")

if((${DBUS_CPP_SANITIZE} STREQUAL "address") OR (${DBUS_CPP_SANITIZE} STREQUAL "leak"))
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=${DBUS_CPP_SANITIZE}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=${DBUS_CPP_SANITIZE}")
endif()

#####################################################################
# Enable code coverage calculation with gcov/gcovr/lcov
# Usage:
#  * Switch build type to coverage (use ccmake or cmake-gui)
#  * Invoke make, make test, make coverage
#  * Find html report in subdir coveragereport
#  * Find xml report feasible for jenkins in coverage.xml
#####################################################################
IF(CMAKE_BUILD_TYPE MATCHES [cC][oO][vV][eE][rR][aA][gG][eE])
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftest-coverage -fprofile-arcs" )
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ftest-coverage -fprofile-arcs" )
  SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -ftest-coverage -fprofile-arcs" )
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ftest-coverage -fprofile-arcs" )
ENDIF(CMAKE_BUILD_TYPE MATCHES [cC][oO][vV][eE][rR][aA][gG][eE])

set(DBUS_CPP_VERSION_MAJOR 5)
set(DBUS_CPP_VERSION_MINOR 0) 
set(DBUS_CPP_VERSION_PATCH 0)

include(CTest)

include_directories(
  include
  ${LIBXML2_INCLUDE_DIR}
)

add_subdirectory(data)
add_subdirectory(doc)
add_subdirectory(examples)
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(tests)

enable_coverage_report(benchmark)
