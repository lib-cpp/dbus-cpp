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

find_package(PkgConfig)
find_package(Boost 1.49 COMPONENTS system)
find_package(Threads)

pkg_check_modules(DBUS dbus-1)

include_directories(
  ${CMAKE_SOURCE_DIR}/include/dbus
  ${DBUS_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  )

add_library(
  dbus_cpp SHARED

  bus.cpp
  error.cpp
  service.cpp

  asio/executor.cpp
)

target_link_libraries(
  dbus_cpp

  ${Boost_LIBRARIES}
  ${DBUS_LIBRARIES}
)