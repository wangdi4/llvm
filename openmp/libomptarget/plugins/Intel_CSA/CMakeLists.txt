##===----------------------------------------------------------------------===##
#
#    Copyright (C) 2018 Intel Corporation. All rights reserved.
#
#    The information and source code contained herein is the exclusive
#    property of Intel Corporation. and may not be disclosed, examined
#    or reproduced in whole or in part without explicit written authorization
#    from the company.
#
##===----------------------------------------------------------------------===##
#
# Build a plugin for a csa machine if available.
#
##===----------------------------------------------------------------------===##

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(tmachine "x86_64")
  set(tmachine_name "csa")
  set(tmachine_libname "csa")
  set(tmachine_triple "csa")
  set(CMAKE_SKIP_RPATH TRUE)

  if(CMAKE_SYSTEM_PROCESSOR MATCHES "${tmachine}$")
    find_library(LibCSAUMR csaumr PATHS $ENV{CSABASE_ROOT}/lib NO_DEFAULT_PATH)
    if(LibCSAUMR)
      libomptarget_say("Building ${tmachine_name} offloading plugin.")

      include_directories(${LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIR})
      include_directories($ENV{CSABASE_ROOT}/include)

      add_library("omptarget.rtl.${tmachine_libname}" SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rtl.cpp)

      # Install plugin under the lib destination folder.
      install(TARGETS "omptarget.rtl.${tmachine_libname}"
        COMPONENT llvm-csa-ompplugin
        LIBRARY DESTINATION "lib")

      target_link_libraries(
        "omptarget.rtl.${tmachine_libname}"
        ${LibCSAUMR}
        dl
        rt
        "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/../exports")

      # Report to the parent scope that we are building a plugin.
      set(LIBOMPTARGET_SYSTEM_TARGETS
        "${LIBOMPTARGET_SYSTEM_TARGETS} ${tmachine_triple}" PARENT_SCOPE)
    else(LibCSAUMR)
      libomptarget_say("Not building ${tmachine_name} offloading plugin: libcsaumr.so dependency not found.")
    endif(LibCSAUMR)
  else()
    libomptarget_say("Not building ${tmachine_name} offloading plugin: machine not found in the system.")
  endif()
else()
 libomptarget_say("Not building csa offloading plugin: machine not found in the system.")
endif()
