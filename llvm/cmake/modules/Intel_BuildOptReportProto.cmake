# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2023 Intel Corporation
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
include(FetchContent)

set(OPT_REPORT_DIR ${PROJECT_SOURCE_DIR}/include/llvm/Analysis/Intel_OptReport)
set(OPT_REPORT_PROTO ${OPT_REPORT_DIR}/opt_report_proto.proto)
set(OPT_REPORT_PROTO_DIR ${PROJECT_BINARY_DIR}/opt-report-proto)
set(OPT_REPORT_PROTO_SRCS ${OPT_REPORT_PROTO_DIR}/opt_report_proto.pb.cc)
set(OPT_REPORT_PROTO_HDRS ${OPT_REPORT_PROTO_DIR}/opt_report_proto.pb.h)

if(TARGET generate_opt_report_proto)
  return()
endif()

if(DEFINED ENV{ICS_GIT_MIRROR} AND NOT "$ENV{ICS_GIT_MIRROR}" STREQUAL "")
  STRING(REGEX REPLACE "\\\\" "/" GITSERVER "$ENV{ICS_GIT_MIRROR}")
      set(PROTOBUF_REPO "${GITSERVER}/protobuf.git")
else()
  set(PROTOBUF_REPO "https://github.com/protocolbuffers/protobuf.git")
endif()

# Get project
FetchContent_Declare(protobuf
  GIT_REPOSITORY    ${PROTOBUF_REPO}
  GIT_TAG           v3.17.3
  GIT_SUBMODULES ""
  GIT_SHALLOW TRUE
  UPDATE_DISCONNECTED 1
  SOURCE_SUBDIR cmake
)

set(protobuf_BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(protobuf_USE_STATIC_LIBS ON CACHE INTERNAL "")
set(protobuf_MODULE_COMPATIBLE ON CACHE INTERNAL "")

# Suppress tests build
set(protobuf_BUILD_TESTS OFF CACHE INTERNAL "disable protobuf tests")

FetchContent_MakeAvailable(protobuf)

if(NOT EXISTS ${protobuf_SOURCE_DIR})
  message(FATAL_ERROR "Protobuf has not been fetched from ${PROTOBUF_REPO}")
endif()

set_property(
  TARGET libprotobuf-lite libprotobuf libprotoc protoc
  APPEND_STRING
  PROPERTY COMPILE_FLAGS "-w ")

add_custom_command(
    OUTPUT ${OPT_REPORT_PROTO_SRCS} ${OPT_REPORT_PROTO_HDRS}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OPT_REPORT_PROTO_DIR}
    COMMAND protoc -I=${OPT_REPORT_DIR} --cpp_out=${OPT_REPORT_PROTO_DIR} ${OPT_REPORT_PROTO}
    WORKING_DIRECTORY ${OPT_REPORT_DIR}
    DEPENDS protoc ${OPT_REPORT_PROTO})

add_custom_target(generate_opt_report_proto
    DEPENDS ${OPT_REPORT_PROTO_SRCS} ${OPT_REPORT_PROTO_HDRS})
