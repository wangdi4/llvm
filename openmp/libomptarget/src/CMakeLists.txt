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
  MemoryManager.cpp
  rtl.cpp
  omptarget.cpp
)

# Build libomptarget library with libdl dependency. Add LLVMSupport
# dependency if building in-tree with profiling enabled.
if(OPENMP_STANDALONE_BUILD OR (NOT OPENMP_ENABLE_LIBOMPTARGET_PROFILING))
  add_library(omptarget SHARED ${src_files})
  if(INTEL_COLLAB)
    if(INTEL_CUSTOMIZATION)
      include_directories("${LIBOMPTARGET_OPENMP_TARGET_HEADER_FOLDER}")
      add_dependencies(omptarget libomptarget-libiomp-file)
      target_link_libraries(omptarget PRIVATE ${LIBOMP_LIB_FILE})
    else(INTEL_CUSTOMIZATION)
      # OpenMP runtime library must be linked in.
      target_link_libraries(omptarget PRIVATE omp)
    endif(INTEL_CUSTOMIZATION)
    target_link_libraries(omptarget PRIVATE ${CMAKE_DL_LIBS})
    if (NOT WIN32)
      target_link_libraries(omptarget PRIVATE
        "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/exports")
    endif()
  else(INTEL_COLLAB)
  target_link_libraries(omptarget
    ${CMAKE_DL_LIBS}
    "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/exports")
  endif(INTEL_COLLAB)
else()
  if(INTEL_COLLAB)
    set(LLVM_LINK_COMPONENTS
      Support
      )
    add_llvm_library(omptarget SHARED ${src_files}
        LINK_LIBS ${CMAKE_DL_LIBS}
        )
    if(INTEL_CUSTOMIZATION)
      include_directories("${LIBOMPTARGET_OPENMP_TARGET_HEADER_FOLDER}")
      add_dependencies(omptarget libomptarget-libiomp-file)
      target_link_libraries(omptarget PRIVATE ${LIBOMP_LIB_FILE})
    else(INTEL_CUSTOMIZATION)
      # OpenMP runtime library must be linked in.
      target_link_libraries(omptarget PRIVATE omp)
    endif(INTEL_CUSTOMIZATION)
    if (NOT WIN32)
      target_link_libraries(omptarget PRIVATE
        "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/exports")
    endif()
    target_compile_definitions(omptarget PUBLIC OMPTARGET_PROFILE_ENABLED)
  else(INTEL_COLLAB)
  set(LLVM_LINK_COMPONENTS
    Support
    )
  add_llvm_library(omptarget SHARED ${src_files}
      LINK_LIBS ${CMAKE_DL_LIBS}
      "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/exports"
      )
  target_compile_definitions(omptarget PUBLIC OMPTARGET_PROFILE_ENABLED)
  endif(INTEL_COLLAB)
endif()

if(INTEL_CUSTOMIZATION)
  if(WIN32)
    libomptarget_add_resource_file(omptarget)
  endif()
endif(INTEL_CUSTOMIZATION)

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
