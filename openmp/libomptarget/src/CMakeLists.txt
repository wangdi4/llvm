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

if(INTEL_COLLAB)
 if(INTEL_CUSTOMIZATION)
   include_directories("${LIBOMPTARGET_OPENMP_TARGET_HEADER_FOLDER}")
   add_dependencies(omptarget libomptarget-libiomp-file)
   target_link_libraries(omptarget ${LIBOMP_LIB_FILE})
 else(INTEL_CUSTOMIZATION)
   # OpenMP runtime library must be linked in.
   target_link_libraries(omptarget omp)
 endif(INTEL_CUSTOMIZATION)
 target_link_libraries(omptarget ${CMAKE_DL_LIBS})
 if (NOT WIN32)
   target_link_libraries(omptarget
    "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/exports")
  endif()
else(INTEL_COLLAB)
target_link_libraries(omptarget
  ${CMAKE_DL_LIBS}
  "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/exports")
endif(INTEL_COLLAB)

# Install libomptarget under the lib destination folder.
if(INTEL_COLLAB)
install(TARGETS omptarget LIBRARY COMPONENT omptarget
  DESTINATION "${OPENMP_INSTALL_LIBDIR}"
  RUNTIME COMPONENT omptarget DESTINATION bin
  ARCHIVE COMPONENT omptarget DESTINATION "${OPENMP_INSTALL_LIBDIR}")
else(INTEL_COLLAB)
install(TARGETS omptarget LIBRARY COMPONENT omptarget
  DESTINATION "${OPENMP_INSTALL_LIBDIR}")
endif(INTEL_COLLAB)

# INTEL_COLLAB
# llvm_update_compile_flags properly disables exceptions, so the system
# headers (which may use EH, e.g MSVC's xlocale) can be properly compiled.
llvm_update_compile_flags(omptarget)
# end INTEL_COLLAB
