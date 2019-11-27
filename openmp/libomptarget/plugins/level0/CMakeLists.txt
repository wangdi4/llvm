# INTEL_CUSTOMIZATION
##===----------------------------------------------------------------------===##
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
##===----------------------------------------------------------------------===##
#
# Plugin for Level0 devices
#
##===----------------------------------------------------------------------===##

if(LIBOMPTARGET_DEP_LEVEL0_FOUND)
  if((CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64)$" OR
      CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64") AND
     (CMAKE_SYSTEM_NAME MATCHES "Linux" OR
      CMAKE_SYSTEM_NAME MATCHES "Windows"))

    libomptarget_say("Building Level0 offloading plugin.")

    # Define the suffix for the runtime messaging dumps.
    add_definitions(-DTARGET_NAME=LEVEL0)
    #add_definitions(-DTARGET_LEVEL0_VERSION=?)

    if(LIBOMPTARGET_CMAKE_BUILD_TYPE MATCHES debug)
      add_definitions(-DOMPTARGET_LEVEL0_DEBUG)
    endif()

    if(CMAKE_SYSTEM_NAME MATCHES "Linux")
      include_directories(${LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIRS})
    endif()

    include_directories(${LIBOMPTARGET_DEP_OPENCL_INCLUDE_DIRS})
    include_directories(${LIBOMPTARGET_DEP_LEVEL0_INCLUDE_DIRS})
    add_library(omptarget.rtl.level0 SHARED src/rtl.cpp)

    if(NOT OPENMP_STANDALONE_BUILD)
      # We have to disable EH for Windows compilation.
      # For standalone OpenMP build, we need to come up
      # with our own EH flags management.
      llvm_update_compile_flags(omptarget.rtl.level0)
    endif()

    # Install plugin under the lib destination folder.
    # DLL part (for DLL platforms) is installed into bin.
    # Do not install interface library for DLL platforms,
    # since the plugin is only supposed to be used by libomptarget.
    install(TARGETS omptarget.rtl.level0
            LIBRARY COMPONENT llvm-level0-ompplugin
            DESTINATION lib${LIBOMPTARGET_LIBDIR_SUFFIX}
            RUNTIME COMPONENT llvm-level0-ompplugin
            DESTINATION bin)

    add_dependencies(omptarget.rtl.level0 libomptarget-libiomp-file)

    if(CMAKE_SYSTEM_NAME MATCHES "Linux")
      target_link_libraries(omptarget.rtl.level0
          ${LIBOMPTARGET_DEP_LEVEL0_LIBRARIES} ${LIBOMP_LIB_FILE}
          "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/../exports")
    elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
      target_link_libraries(omptarget.rtl.level0
          ${LIBOMPTARGET_DEP_LEVEL0_LIBRARIES} ${LIBOMP_LIB_FILE})
    else()
      message(FATAL_ERROR "Missing platfrom support.")
    endif()

    # Report to the parent scope that we are building a plugin for Level0.
    set(LIBOMPTARGET_SYSTEM_TARGETS
      "${LIBOMPTARGET_SYSTEM_TARGETS} spir64-unknown-unknown" PARENT_SCOPE)
  else()
    libomptarget_say("Not building Level0 offloading plugin: only support Level0 in x86_64 Linux/Windows hosts.")
  endif()
else()
  libomptarget_say("Not building Level0 offloading plugin: Level0 not found in system.")
endif()

# end INTEL_CUSTOMIZATION
