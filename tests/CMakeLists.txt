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

set (OLD_CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
# Don't treat warnings as errors in 3rd_party/{gmock,cucumber-cpp}
string (REPLACE " -Werror " " " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
find_package(Gtest REQUIRED)
include_directories(${GMOCK_INCLUDE_DIR} ${GTEST_INCLUDE_DIR})
set (CMAKE_CXX_FLAGS ${OLD_CMAKE_CXX_FLAGS})

find_package(Threads)

add_definitions(-DCORE_DBUS_ENABLE_GOOGLE_TEST_FIXTURE)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/test_data.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/test_data.h @ONLY)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

include_directories(
  ${DBUS_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${GTEST_INCLUDE_DIRS}
  ${PROCESS_CPP_INCLUDE_DIRS}
)

add_executable(
  bus_test
  bus_test.cpp
  )

add_executable(
  dbus_test
  dbus_test.cpp
  )

add_executable(
  codec_test
  codec_test.cpp
  )

add_executable(
  compiler_test
  compiler_test.cpp
  )

add_executable(
  executor_test
  executor_test.cpp
  )

add_executable(
  stl_codec_test
  stl_codec_test.cpp
  )

add_executable(
  message_test
  message_test.cpp   
  )

add_executable(
  match_rule_test
  match_rule_test.cpp
  )

add_executable(
  message_router_test
  message_router_test.cpp
  )

add_executable(
  service_test
  service_test.cpp
  )

add_executable(
  types_test
  types_test.cpp
  )

target_link_libraries(
  bus_test

  dbus-cpp
  dbus-cppc-helper

  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${GTEST_BOTH_LIBRARIES}
  ${PROCESS_CPP_LIBRARIES}
)

target_link_libraries(
  dbus_test

  dbus-cpp
  dbus-cppc-helper

  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${GTEST_BOTH_LIBRARIES}
  )

target_link_libraries(
  executor_test

  dbus-cpp
  dbus-cppc-helper

  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${GTEST_BOTH_LIBRARIES}
  )

target_link_libraries(
  codec_test

  dbus-cpp
  dbus-cppc-helper

  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${GTEST_BOTH_LIBRARIES}
  )

target_link_libraries(
  compiler_test

  dbus-cpp
  dbus-cppc-helper

  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${LIBXML2_LIBRARIES}
  ${GMOCK_LIBRARY}
  ${GTEST_BOTH_LIBRARIES}
  )

target_link_libraries(
  stl_codec_test

  dbus-cpp
  dbus-cppc-helper

  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${GTEST_BOTH_LIBRARIES}
  )

target_link_libraries(
  message_test

  dbus-cpp
  dbus-cppc-helper

  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${GTEST_BOTH_LIBRARIES}
  )

target_link_libraries(
  match_rule_test

  dbus-cpp
  dbus-cppc-helper

  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${GTEST_BOTH_LIBRARIES}
  )

target_link_libraries(
  message_router_test

  dbus-cpp
  dbus-cppc-helper

  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${GTEST_BOTH_LIBRARIES}
  )

target_link_libraries(
  service_test

  dbus-cpp
  dbus-cppc-helper

  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${GTEST_BOTH_LIBRARIES}
  )

target_link_libraries(
  types_test

  dbus-cpp

  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${GTEST_BOTH_LIBRARIES}
  )

add_test(bus_test ${CMAKE_CURRENT_BINARY_DIR}/bus_test)
add_test(dbus_test ${CMAKE_CURRENT_BINARY_DIR}/dbus_test)
add_test(executor_test ${CMAKE_CURRENT_BINARY_DIR}/executor_test)
add_test(codec_test ${CMAKE_CURRENT_BINARY_DIR}/codec_test)
add_test(compiler_test ${CMAKE_CURRENT_BINARY_DIR}/compiler_test)
add_test(stl_codec_test ${CMAKE_CURRENT_BINARY_DIR}/stl_codec_test)
add_test(types_test ${CMAKE_CURRENT_BINARY_DIR}/types_test)
add_test(message_test ${CMAKE_CURRENT_BINARY_DIR}/message_test)
add_test(match_rule_test ${CMAKE_CURRENT_BINARY_DIR}/match_rule_test)
add_test(message_router_test ${CMAKE_CURRENT_BINARY_DIR}/message_router_test)
add_test(service_test ${CMAKE_CURRENT_BINARY_DIR}/service_test)
