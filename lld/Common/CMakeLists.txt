<<<<<<< HEAD
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
set(LLD_SYSTEM_LIBS ${LLVM_PTHREAD_LIB})

if(NOT HAVE_CXX_ATOMICS64_WITHOUT_LIB)
  list(APPEND LLD_SYSTEM_LIBS atomic)
endif()

=======
>>>>>>> f0b451c77f14947e3e7d314f048679fa2f5c6298
find_first_existing_vc_file("${LLVM_MAIN_SRC_DIR}" llvm_vc)
find_first_existing_vc_file("${LLD_SOURCE_DIR}" lld_vc)

set(version_inc "${CMAKE_CURRENT_BINARY_DIR}/VCSVersion.inc")
set(generate_vcs_version_script "${LLVM_CMAKE_DIR}/GenerateVersionFromVCS.cmake")

if(lld_vc AND LLVM_APPEND_VC_REV)
  set(lld_source_dir ${LLD_SOURCE_DIR})
endif()

add_custom_command(OUTPUT "${version_inc}"
  DEPENDS "${lld_vc}" "${generate_vcs_version_script}"
  COMMAND ${CMAKE_COMMAND} "-DNAMES=LLD"
  "-DLLD_SOURCE_DIR=${lld_source_dir}"
  "-DHEADER_FILE=${version_inc}"
  -P "${generate_vcs_version_script}")

# Mark the generated header as being generated.
set_source_files_properties("${version_inc}"
  PROPERTIES GENERATED TRUE
  HEADER_FILE_ONLY TRUE)

add_lld_library(lldCommon
  Args.cpp
  CommonLinkerContext.cpp
  DWARF.cpp
  ErrorHandler.cpp
  Filesystem.cpp
  Memory.cpp
  Reproduce.cpp
  Strings.cpp
  TargetOptionsCommandFlags.cpp
  Timer.cpp
  VCSVersion.inc
  Version.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLD_INCLUDE_DIR}/lld/Common

  LINK_COMPONENTS
  Codegen
  Core
  DebugInfoDWARF
  Demangle
  MC
  Option
  Support
  Target

  LINK_LIBS
  ${LLVM_PTHREAD_LIB}
  ${LLVM_ATOMIC_LIB}

  DEPENDS
  intrinsics_gen
  )

# INTEL_CUSTOMIZATION
# Export LLD header files for OpenCL CPU RT in-tree Windows build.
target_include_directories(lldCommon INTERFACE
  $<BUILD_INTERFACE:${LLD_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${LLD_BINARY_DIR}/include>
  )
# end INTEL_CUSTOMIZATION
