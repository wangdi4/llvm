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
<<<<<<< HEAD
if(INTEL_COLLAB)
add_library(elf_common OBJECT elf_common.cpp elf_light.cpp)
else(INTEL_COLLAB)
add_library(elf_common OBJECT elf_common.cpp)
endif(INTEL_COLLAB)
=======

add_library(elf_common OBJECT elf_common.cpp ELFSymbols.cpp)
>>>>>>> 894531f59beb03c17a6e11e5b9c9995182b8d727

# Build elf_common with PIC to be able to link it with plugin shared libraries.
set_property(TARGET elf_common PROPERTY POSITION_INDEPENDENT_CODE ON)
llvm_update_compile_flags(elf_common)
set(LINK_LLVM_LIBS LLVMBinaryFormat LLVMObject LLVMSupport)
if (LLVM_LINK_LLVM_DYLIB)
  set(LINK_LLVM_LIBS LLVM)
endif()
target_link_libraries(elf_common INTERFACE ${LINK_LLVM_LIBS})
# INTEL_CUSTOMIZATION
  # Statically link C run-time library on Windows.
  # Otherwise, we cannot use LLVMObject and LLVMSupport.
  # Alternatively we can link LLVMSupport_dyn (which we already build)
  # and LLVMObject_dyn (yet another component library to be built
  # in two flavors).
  set_msvc_crt_flags(elf_common)
# end INTEL_CUSTOMIZATION
add_dependencies(elf_common ${LINK_LLVM_LIBS})

# The code uses Debug.h, which requires threads support.
target_link_libraries(elf_common INTERFACE ${OPENMP_PTHREAD_LIB})

# Expose elf_common.h directory to the users of this library.
target_include_directories(elf_common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(elf_common PRIVATE ${LIBOMPTARGET_INCLUDE_DIR})
