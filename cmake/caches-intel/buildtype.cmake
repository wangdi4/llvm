# INTEL CONFIDENTIAL
#
# Copyright (C) 2022 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may not
# use, modify, copy, publish, distribute, disclose or transmit this software or
# the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.

# Translate ICS_BUILDTYPE to cmake files to include.
if(NOT DEFINED ENV{ICS_BUILDTYPE})
  message(FATAL_ERROR "This cmake file requires ICS_BUILDTYPE to be defined")
endif()

# Set CMAKE_INSTALL_PREFIX to the expected deploy directory.
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  set(target_os "linux")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  set(target_os "win")
else()
  message(FATAL_ERROR "Unknown system, cannot determine correct deploy directory")
endif()
set(CMAKE_INSTALL_PREFIX $ENV{ICS_WSDIR}/deploy/${target_os}_$ENV{ICS_BUILDTYPE} CACHE PATH "")

string(REPLACE "srelusingrel" "sreleaseusingrelease" buildtype $ENV{ICS_BUILDTYPE})

if(buildtype STREQUAL "debug")
  include(${CMAKE_CURRENT_LIST_DIR}/Debug.cmake)
elseif(buildtype STREQUAL "prod")
  include(${CMAKE_CURRENT_LIST_DIR}/Asserts.cmake)
elseif(buildtype STREQUAL "release")
  include(${CMAKE_CURRENT_LIST_DIR}/Release.cmake)
elseif(buildtype STREQUAL "sdebugusingdebug")
  include(${CMAKE_CURRENT_LIST_DIR}/Debug.cmake)
  include(${CMAKE_CURRENT_LIST_DIR}/Stage2.cmake)
elseif(buildtype STREQUAL "sdebugusingprod")
  include(${CMAKE_CURRENT_LIST_DIR}/Debug.cmake)
  include(${CMAKE_CURRENT_LIST_DIR}/Stage2.cmake)
elseif(buildtype STREQUAL "sprodusingdebug")
  include(${CMAKE_CURRENT_LIST_DIR}/Asserts.cmake)
  include(${CMAKE_CURRENT_LIST_DIR}/Stage2.cmake)
elseif(buildtype STREQUAL "sprodusingprod")
  include(${CMAKE_CURRENT_LIST_DIR}/Asserts.cmake)
  include(${CMAKE_CURRENT_LIST_DIR}/Stage2.cmake)
elseif(buildtype STREQUAL "sreleaseusingrelease")
  include(${CMAKE_CURRENT_LIST_DIR}/Release.cmake)
  include(${CMAKE_CURRENT_LIST_DIR}/Stage2.cmake)
elseif(buildtype STREQUAL "pgogen_sprodusingprod")
  include(${CMAKE_CURRENT_LIST_DIR}/Asserts.cmake)
  include(${CMAKE_CURRENT_LIST_DIR}/PGOGen.cmake)
elseif(buildtype STREQUAL "pgouse_sprodusingprod")
  include(${CMAKE_CURRENT_LIST_DIR}/Asserts.cmake)
  include(${CMAKE_CURRENT_LIST_DIR}/PGOUse.cmake)
elseif(buildtype STREQUAL "pgogen_sreleaseusingrelease")
  include(${CMAKE_CURRENT_LIST_DIR}/Release.cmake)
  include(${CMAKE_CURRENT_LIST_DIR}/PGOGen.cmake)
elseif(buildtype STREQUAL "pgouse_sreleaseusingrelease")
  include(${CMAKE_CURRENT_LIST_DIR}/Release.cmake)
  include(${CMAKE_CURRENT_LIST_DIR}/PGOUse.cmake)
endif()
