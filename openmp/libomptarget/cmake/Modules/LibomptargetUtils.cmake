# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2021 Intel Corporation
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
#
#//===----------------------------------------------------------------------===//
#//
#// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
#// See https://llvm.org/LICENSE.txt for license information.
#// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#//
#//===----------------------------------------------------------------------===//
#

# void libomptarget_say(string message_to_user);
# - prints out message_to_user
macro(libomptarget_say message_to_user)
  message(STATUS "LIBOMPTARGET: ${message_to_user}")
endmacro()

# void libomptarget_warning_say(string message_to_user);
# - prints out message_to_user with a warning
macro(libomptarget_warning_say message_to_user)
  message(WARNING "LIBOMPTARGET: ${message_to_user}")
endmacro()

# void libomptarget_error_say(string message_to_user);
# - prints out message_to_user with an error and exits cmake
macro(libomptarget_error_say message_to_user)
  message(FATAL_ERROR "LIBOMPTARGET: ${message_to_user}")
endmacro()

if(INTEL_CUSTOMIZATION)
macro(libomptarget_add_resource_file target)
  if(WIN32)
    set(library_name "${target}.dll")
    set(resource_file "${CMAKE_CURRENT_BINARY_DIR}/${target}.res")
    get_target_property(target_include ${target} INCLUDE_DIRECTORIES)
    find_file(omptarget_rc omptarget.rc PATHS ${target_include})
    set(rc_flags
        -I${CMAKE_SOURCE_DIR}/../clang/include
        -I${LLVM_INCLUDE_DIR}
        -DINTEL_CUSTOMIZATION=1
        -DLIBRARY_NAME="${library_name}"
        -Fo${resource_file}
    )
    add_custom_command(
      OUTPUT ${resource_file}
      COMMAND rc ${rc_flags} ${omptarget_rc}
      DEPENDS ${omptarget_rc}
      VERBATIM
    )
    add_custom_target(${target}_res ALL DEPENDS ${resource_file})
    add_dependencies(${target} ${target}_res)
    target_link_libraries(${target} PRIVATE ${resource_file})
  endif()
endmacro()
endif(INTEL_CUSTOMIZATION)
