# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2023 Intel Corporation
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

function(add_sycl_library LIB_NAME TYPE)
  cmake_parse_arguments("ARG"
    ""
    "LINKER_SCRIPT"
    "SOURCES;INCLUDE_DIRS;LIBRARIES"
    ${ARGN}
  )
  add_library(${LIB_NAME} ${TYPE} ${ARG_SOURCES})
  target_include_directories(${LIB_NAME} PRIVATE ${ARG_INCLUDE_DIRS})
  target_link_libraries(${LIB_NAME} PRIVATE ${ARG_LIBRARIES})
if (INTEL_CUSTOMIZATION)
  # The top-level xmain CMakeLists.txt chooses runtimes appropriate for
  # statically-linked binaries, so we need to arrange for SYCL libraries to use
  # the *DLL versions here.
  # In intel/llvm CMAKE_MSVC_RUNTIME_LIBRARY is not specified, and the default
  # is already appropriate.
  set_property(TARGET ${LIB_NAME} PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif (INTEL_CUSTOMIZATION)

  add_dependencies(sycl-runtime-libraries ${LIB_NAME})

  if (ARG_LINKER_SCRIPT AND UNIX AND NOT APPLE)
    target_link_libraries(${LIB_NAME} PRIVATE
      "-Wl,--version-script=${ARG_LINKER_SCRIPT}")
  endif()

  target_compile_definitions(${LIB_NAME} PRIVATE __SYCL_BUILD_SYCL_DLL)

  if (UNIX)
    target_compile_options(${LIB_NAME} PRIVATE -fvisibility=hidden)
  else()
    add_stripped_pdb(${LIB_NAME})
  endif()

  # TODO: Enabled for MSVC
  if (NOT MSVC AND SYCL_LIB_WITH_DEBUG_SYMBOLS)
    separate_arguments(CMAKE_CXX_FLAGS_DEBUG_SEPARATED UNIX_COMMAND "${CMAKE_CXX_FLAGS_DEBUG}")
    target_compile_options(${LIB_NAME} PRIVATE ${CMAKE_CXX_FLAGS_DEBUG_SEPARATED})
  endif()

  # TODO remove add_common_options
  add_common_options(${LIB_NAME})
endfunction()

function(add_sycl_plugin PLUGIN_NAME)
  cmake_parse_arguments("ARG"
    ""
    ""
    "SOURCES;INCLUDE_DIRS;LIBRARIES;HEADER"
    ${ARGN}
  )

  add_sycl_library("pi_${PLUGIN_NAME}" SHARED
    LINKER_SCRIPT "${PROJECT_SOURCE_DIR}/plugins/ld-version-script.txt"
    SOURCES ${ARG_SOURCES}
    INCLUDE_DIRS
      ${ARG_INCLUDE_DIRS}
      ${sycl_inc_dir}
    LIBRARIES
      ${ARG_LIBRARIES}
      OpenCL-Headers
  )

  # Install feature test header
  if (NOT "${ARG_HEADER}" STREQUAL "")
    get_filename_component(HEADER_NAME ${ARG_HEADER} NAME)
    configure_file(
      ${ARG_HEADER}
      ${SYCL_INCLUDE_BUILD_DIR}/sycl/detail/plugins/${PLUGIN_NAME}/${HEADER_NAME}
      COPYONLY)

    install(FILES ${ARG_HEADER}
            DESTINATION ${SYCL_INCLUDE_DIR}/sycl/detail/plugins/${PLUGIN_NAME}
            COMPONENT pi_${PLUGIN_NAME})
  endif()

  install(TARGETS pi_${PLUGIN_NAME}
    LIBRARY DESTINATION "lib${LLVM_LIBDIR_SUFFIX}" COMPONENT pi_${PLUGIN_NAME}
    RUNTIME DESTINATION "bin" COMPONENT pi_${PLUGIN_NAME})

  set (manifest_file
    ${CMAKE_CURRENT_BINARY_DIR}/install_manifest_pi_${PLUGIN_NAME}.txt)
  add_custom_command(OUTPUT ${manifest_file}
    COMMAND "${CMAKE_COMMAND}"
    "-DCMAKE_INSTALL_COMPONENT=pi_${PLUGIN_NAME}"
    -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    COMMENT "Deploying component pi_${PLUGIN_NAME}"
    USES_TERMINAL
  )
  add_custom_target(install-sycl-plugin-${PLUGIN_NAME}
    DEPENDS
      ${manifest_file} pi_${PLUGIN_NAME}
    )

  set_property(GLOBAL APPEND PROPERTY SYCL_TOOLCHAIN_INSTALL_COMPONENTS
    pi_${PLUGIN_NAME})
endfunction()
