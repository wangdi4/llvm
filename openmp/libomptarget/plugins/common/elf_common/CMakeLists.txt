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

if(INTEL_COLLAB)
# Use libelf, is possible.
# Otherwise, use LLVM ELF (unless OPENMP_STANDALONE_BUILD).
option(LIBOMPTARGET_FORCE_LLVM_ELF
  "Use LLVM ELFObjectFile implementation for elf_common.\
  This requires in-tree build." OFF)
add_library(elf_common OBJECT elf_common.cpp elf_light.cpp)

if(LIBOMPTARGET_DEP_LIBELF_FOUND AND NOT LIBOMPTARGET_FORCE_LLVM_ELF)
  target_compile_definitions(elf_common PRIVATE -DMAY_USE_LIBELF)
  target_include_directories(elf_common PRIVATE
    ${LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIR})
  target_link_libraries(elf_common INTERFACE
    ${LIBOMPTARGET_DEP_LIBELF_LIBRARIES})
elseif(NOT OPENMP_STANDALONE_BUILD)
  llvm_update_compile_flags(elf_common)
  set(LINK_LLVM_LIBS LLVMBinaryFormat LLVMObject LLVMSupport)
  target_link_libraries(elf_common INTERFACE ${LINK_LLVM_LIBS})
  add_dependencies(elf_common ${LINK_LLVM_LIBS})
else(LIBOMPTARGET_DEP_LIBELF_FOUND AND NOT LIBOMPTARGET_FORCE_LLVM_ELF)
  libomptarget_say("Not building elf_common offload plugin library: libelf dependency not found.")
endif(LIBOMPTARGET_DEP_LIBELF_FOUND AND NOT LIBOMPTARGET_FORCE_LLVM_ELF)

# The code uses Debug.h, which requires threads support.
target_link_libraries(elf_common INTERFACE ${OPENMP_PTHREAD_LIB})

# Expose elf_common.h directory to the users of this library.
else(INTEL_COLLAB)
add_library(elf_common INTERFACE)
endif(INTEL_COLLAB)
target_include_directories(elf_common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
