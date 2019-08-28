##===----------------------------------------------------------------------===##
# 
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# 
##===----------------------------------------------------------------------===##
#
# Build offloading library libomptarget.so.
#
##===----------------------------------------------------------------------===##

libomptarget_say("Building offloading runtime library libomptarget.")

set(src_files
  api.cpp
  device.cpp
  interface.cpp
  rtl.cpp
  omptarget.cpp
)

# Build libomptarget library with libdl dependency.
add_library(omptarget SHARED ${src_files})

# INTEL_COLLAB
include_directories("${LIBOMPTARGET_OPENMP_TARGET_HEADER_FOLDER}")

add_dependencies(omptarget libomptarget-libiomp-file)

if (WIN32)
  target_link_libraries(omptarget
    ${CMAKE_DL_LIBS} ${LIBOMP_LIB_FILE})
else()
  target_link_libraries(omptarget
    ${CMAKE_DL_LIBS} ${LIBOMP_LIB_FILE}
    "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/exports")
endif()
# end INTEL_COLLAB

# Install libomptarget under the lib destination folder.
install(TARGETS omptarget LIBRARY COMPONENT omptarget
  DESTINATION "${OPENMP_INSTALL_LIBDIR}" # INTEL
# INTEL_COLLAB
  RUNTIME COMPONENT omptarget DESTINATION bin
  ARCHIVE COMPONENT omptarget DESTINATION "${OPENMP_INSTALL_LIBDIR}"
# end INTEL_COLLAB
  ) # INTEL

# INTEL_COLLAB
# llvm_update_compile_flags properly disables exceptions, so the system
# headers (which may use EH, e.g MSVC's xlocale) can be properly compiled.
llvm_update_compile_flags(omptarget)
# end INTEL_COLLAB
