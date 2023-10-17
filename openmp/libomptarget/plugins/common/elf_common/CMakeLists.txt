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
##===----------------------------------------------------------------------===##
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
##===----------------------------------------------------------------------===##
#
# Common ELF functionality for target plugins
#
##===----------------------------------------------------------------------===##
add_library(elf_common OBJECT elf_common.cpp elf_light.cpp)

# This is required when using LLVM libraries.
llvm_update_compile_flags(elf_common)

if (LLVM_LINK_LLVM_DYLIB)
  set(llvm_libs LLVM)
else()
  llvm_map_components_to_libnames(llvm_libs BinaryFormat Object Support)
endif()

target_link_libraries(elf_common PUBLIC ${llvm_libs} ${OPENMP_PTHREAD_LIB})

# Build elf_common with PIC to be able to link it with plugin shared libraries.
set_property(TARGET elf_common PROPERTY POSITION_INDEPENDENT_CODE ON)

# Expose elf_common.h directory to the users of this library.
target_include_directories(elf_common
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${LIBOMPTARGET_INCLUDE_DIR}
)
# end INTEL_CUSTOMIZATION
