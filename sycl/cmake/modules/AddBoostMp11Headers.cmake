# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2022 Intel Corporation
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
#
# end INTEL_CUSTOMIZATION
# boost/mp11 headers import and preprocessing
# See more comments in cmake/modules/PreprocessBoostMp11Headers.cmake

include(FetchContent)

set(BOOST_MP11_GIT_REPO https://github.com/boostorg/mp11.git)
# Author: pdimov
# Date: Feb 16, 2022
# Comment:
#   Merge pull request #71 from grisumbras/feature/mp_valid_and_true
set(BOOST_MP11_GIT_TAG 7bc4e1ae9b36ec8ee635c3629b59ec525bbe82b9)

if (INTEL_CUSTOMIZATION)
  set(BOOST_MP11_GIT_REPO https://github.com/intel-innersource/applications.compilers.source.mp11)
  if (DEFINED ENV{ICS_GIT_MIRROR} AND NOT "$ENV{ICS_GIT_MIRROR}" STREQUAL "")
    # ICS_GIT_MIRROR, if set, overrides default internal URL.
    set(BOOST_MP11_GIT_REPO $ENV{ICS_GIT_MIRROR}/applications.compilers.source.mp11)
    STRING(REGEX REPLACE "\\\\" "/" BOOST_MP11_GIT_REPO "${BOOST_MP11_GIT_REPO}")
  endif ()
endif (INTEL_CUSTOMIZATION)

# Either download from github or use existing if BOOST_MP11_SOURCE_DIR is set
if (NOT DEFINED BOOST_MP11_SOURCE_DIR)
  message(STATUS "BOOST_MP11_SOURCE_DIR not set, downloading boost/mp11 headers from ${BOOST_MP11_GIT_REPO}")

  FetchContent_Declare(boost_mp11
    GIT_REPOSITORY ${BOOST_MP11_GIT_REPO}
    GIT_TAG ${BOOST_MP11_GIT_TAG}
  )
  FetchContent_GetProperties(boost_mp11)
  FetchContent_MakeAvailable(boost_mp11)

  set(BOOST_MP11_SOURCE_DIR ${boost_mp11_SOURCE_DIR})
  set(BOOST_MP11_SRC_PATH ${BOOST_MP11_GIT_REPO})
  set(BOOST_MP11_SRC_ID "git commit hash: ${BOOST_MP11_GIT_TAG}")
else (NOT DEFINED BOOST_MP11_SOURCE_DIR)
  message(STATUS "Using boost/mp11 headers from ${BOOST_MP11_SOURCE_DIR}")
  set(BOOST_MP11_SRC_PATH ${BOOST_MP11_SOURCE_DIR})
  set(BOOST_MP11_SRC_ID "ID not set")
endif(NOT DEFINED BOOST_MP11_SOURCE_DIR)

# Read all header file names into HEADERS_BOOST_MP11
file(GLOB_RECURSE HEADERS_BOOST_MP11 CONFIGURE_DEPENDS "${BOOST_MP11_SOURCE_DIR}/include/boost/*")

set(BOOST_MP11_DESTINATION_DIR ${SYCL_INCLUDE_BUILD_DIR}/sycl/detail/boost)
string(REPLACE "${BOOST_MP11_SOURCE_DIR}/include/boost" "${BOOST_MP11_DESTINATION_DIR}"
  OUT_HEADERS_BOOST_MP11 "${HEADERS_BOOST_MP11}")

# The target which produces preprocessed boost/mp11 headers
add_custom_target(boost_mp11-headers
  DEPENDS ${OUT_HEADERS_BOOST_MP11})

# Run preprocessing on each header, output result into
# ${BOOST_MP11_DESTINATION_DIR}
add_custom_command(
  OUTPUT ${OUT_HEADERS_BOOST_MP11}
  DEPENDS ${HEADERS_BOOST_MP11}
  COMMAND ${CMAKE_COMMAND}
    -DIN=${BOOST_MP11_SOURCE_DIR}/include/boost
    -DOUT=${BOOST_MP11_DESTINATION_DIR}
    -DSRC_PATH="${BOOST_MP11_SRC_PATH}"
    -DSRC_ID="${BOOST_MP11_SRC_ID}"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/PreprocessBoostMp11Headers.cmake
  COMMENT "Preprocessing boost/mp11 headers ${BOOST_MP11_SOURCE_DIR}/include/boost -> ${BOOST_MP11_DESTINATION_DIR}...")
