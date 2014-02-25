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
find_package(Threads)

pkg_check_modules(DBUS dbus-1)

include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${DBUS_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${PROCESS_CPP_INCLUDE_DIR}
)

configure_file(
  fixture.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/fixture.cpp @ONLY
)

add_library(
  dbus-cpp SHARED

  ${CMAKE_CURRENT_BINARY_DIR}/fixture.cpp

  bus.cpp
  dbus.cpp
  error.cpp
  match_rule.cpp
  message.cpp
  service.cpp
  service_watcher.cpp

  asio/executor.cpp

  types/object_path.cpp
)

target_link_libraries(
  dbus-cpp

  ${Boost_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${LIBXML2_LIBRARIES}
  ${PROCESS_CPP_LIBRARIES}
)

set_target_properties(
  dbus-cpp

  PROPERTIES
  VERSION ${DBUS_CPP_VERSION_MAJOR}.${DBUS_CPP_VERSION_MINOR}.${DBUS_CPP_VERSION_PATCH}
  SOVERSION ${DBUS_CPP_VERSION_MAJOR}
)

install(
  TARGETS dbus-cpp
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# We want to test compiler functionality in our testing framework and
# thus collect symbols in this temporary helper library that is not
# installed but just linked to the actual compiler executable.
add_library(
  dbus-cppc-helper

  compiler.cpp
  generator.cpp
  introspection_parser.cpp
)

target_link_libraries(
  dbus-cppc-helper

  ${Boost_LIBRARIES}
  ${LIBXML2_LIBRARIES}
)

add_executable(
  dbus-cppc

  compiler_main.cpp
)

target_link_libraries(
  dbus-cppc

  dbus-cppc-helper
)

install(
  TARGETS dbus-cppc
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
