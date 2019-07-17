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
if (WIN32)
  set(LIBOMP_LIB_FILE "")

  if (INTEL_CUSTOMIZATION)
    set(LIBOMP_LIB_NAME iomp5md libiomp5md)
    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
      set(LIBOMP_LIB_LOC
        "${IL0_PREBUILT_COMPONENTS_PREFIX}/compiler/lib/ia32_win")
    else()
      set(LIBOMP_LIB_LOC
        "${IL0_PREBUILT_COMPONENTS_PREFIX}/compiler/lib/intel64_win")
    endif()
    message(STATUS "Looking for OMP library (${LIBOMP_LIB_NAME}) in ${LIBOMP_LIB_LOC}")
    find_library(LIBOMP_LIB NAMES ${LIBOMP_LIB_NAME} PATHS "${LIBOMP_LIB_LOC}")

    if (NOT LIBOMP_LIB)
      message(FATAL_ERROR "OMP library required for libomptarget is not found.")
    endif()

    message(STATUS "OMP library found: ${LIBOMP_LIB}")

    # We cannot use UNC paths with forward slashes with MS linker,
    # so we have to copy the OMP library to the current binary directory.
    get_filename_component(LIBOMP_LIB_BASENAME "${LIBOMP_LIB}" NAME)
    add_custom_command(TARGET omptarget PRE_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LIBOMP_LIB}" ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Copying ${LIBOMP_LIB} to ${CMAKE_CURRENT_BINARY_DIR}")
    set(LIBOMP_LIB_FILE ${LIBOMP_LIB_BASENAME})
  endif(INTEL_CUSTOMIZATION)

  target_link_libraries(omptarget
    ${CMAKE_DL_LIBS} ${LIBOMP_LIB_FILE})
else()
  target_link_libraries(omptarget
    ${CMAKE_DL_LIBS}
    "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/exports")
endif()
# end INTEL_COLLAB

# Install libomptarget under the lib destination folder.
install(TARGETS omptarget LIBRARY COMPONENT omptarget
  DESTINATION "${OPENMP_INSTALL_LIBDIR}" # INTEL
# INTEL_COLLAB
  RUNTIME COMPONENT omptarget DESTINATION "${OPENMP_INSTALL_LIBDIR}"
# end INTEL_COLLAB
  ) # INTEL

# INTEL_COLLAB
# llvm_update_compile_flags properly disables exceptions, so the system
# headers (which may use EH, e.g MSVC's xlocale) can be properly compiled.
llvm_update_compile_flags(omptarget)
# end INTEL_COLLAB
