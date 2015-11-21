#!/bin/sh

# Copyright (C) 2015 Canonical Ltd
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authored by: Michi Henning <michi.henning@canonical.com>

#
# Script to read the version numbers from VERSION and QT-VERSION
# and write the version components and the soversion numbers
# into separate files, so we can pick them up from both
# gen-debian-files.sh and CMakeLists.txt.
#

set -e  # Fail if any command fails.

progname=$(basename $0)

[ $# -lt 1 -o $# -gt 2 ] && {
    echo "usage: $progname path-to-debian-dir [output-dir]" >&2
    exit 1
}
dir=$1
output_dir=`pwd`
[ $# -eq 2 ] && output_dir=$2

# Write the various version numbers into a bunch of files. This allows
# us to easily pick them up from both gen-debian-files.sh and CMakeLists.txt.

distro=$(lsb_release -c -s)

full_version=$(cat "${dir}"/VERSION)

major=$(echo $full_version | cut -d'.' -f1)
minor=$(echo $full_version | cut -d'.' -f2)
micro=$(echo $full_version | cut -d'.' -f3)
major_minor="${major}.${minor}"

vivid_full_version=$(cat "${dir}"/VERSION.vivid)
vivid_major=$(echo $vivid_full_version | cut -d'.' -f1)
vivid_soversion=$vivid_major

if [ "$distro" = "vivid" ]
then
    soversion=${vivid_soversion}
else
    soversion="${major}"
fi
[ -n $soversion ]

echo ${full_version} >${output_dir}/libdbus-cpp.full-version
echo ${major} >${output_dir}/libdbus-cpp.major-version
echo ${minor} >${output_dir}/libdbus-cpp.minor-version
echo ${micro} >${output_dir}/libdbus-cpp.micro-version
echo ${major_minor} >${output_dir}/libdbus-cpp.major-minor-version
echo ${soversion} >${output_dir}/libdbus-cpp.soversion
echo ${vivid_soversion} >${output_dir}/libdbus-cpp.vivid-soversion
