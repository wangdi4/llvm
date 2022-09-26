# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2021 Intel Corporation
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
cmake_minimum_required(VERSION 3.2)

# Supported configurations
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_llvm_external_project(opencl)

set(LLVM_LINK_COMPONENTS
        ${LLVM_TARGETS_TO_BUILD}
        Support
        )
file(GLOB TARGET_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

set(AOCL_IOC64_PROJECT_NAME ioc64)

add_llvm_tool_in_bin(${AOCL_IOC64_PROJECT_NAME} ${TARGET_SOURCES} CUSTOM_WIN_VER)

if(NOT MSVC)
  # FIXME: when built with icx it produces a warning.
  target_compile_options(${AOCL_IOC64_PROJECT_NAME} PRIVATE "-Wno-unused-parameter")
endif()

target_link_libraries(${AOCL_IOC64_PROJECT_NAME}
  PRIVATE
    OpenCL-Headers
    OpenCL-ICD)

if(INTEL_CUSTOMIZATION)
  dpcpptarget_add_resource_file(${AOCL_IOC64_PROJECT_NAME}
    "${AOCL_IOC64_PROJECT_NAME}.exe"
    "Intel(R) oneAPI DPC++ Compiler"
    "OpenCL ahead-of-time (AOT) compilation tool for Intel(R) FPGA Emulation Platform.")
endif()

# Use it as fake dependency in order to force another command(s) to execute.
add_custom_command(OUTPUT __force_it
        COMMAND "${CMAKE_COMMAND}" -E echo
        )
#Serialize installation to avoid missing components due to build race conditions
set(__chain_dep __force_it)

set(manifest_list)

message( STATUS "Adding component ${AOCL_IOC64_PROJECT_NAME} to deploy")

set (manifest_file ${CMAKE_CURRENT_BINARY_DIR}/install_manifest_${AOCL_IOC64_PROJECT_NAME}.txt)
add_custom_command(OUTPUT ${manifest_file}
        COMMAND "${CMAKE_COMMAND}"
        "-DCMAKE_INSTALL_COMPONENT=${AOCL_IOC64_PROJECT_NAME}"
        -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
        DEPENDS  ${__chain_dep}
        COMMENT "Deploying component ${AOCL_IOC64_PROJECT_NAME}"
        USES_TERMINAL
        )
list(APPEND manifest_list ${manifest_file})
set(__chain_dep ${manifest_file})

add_custom_target(deploy-${AOCL_IOC64_PROJECT_NAME}
        DEPENDS ${AOCL_IOC64_PROJECT_NAME} ${manifest_list}
        )

# The following copy operation is only to avoid regression during this loading of PSG drop
# and QA update. It will be removed after PSG drop and QA change are landed.
if(WIN32)
    set(exeExt ".exe")
endif(WIN32)

install (CODE
    "
    configure_file(${CMAKE_INSTALL_PREFIX}/bin/${AOCL_IOC64_PROJECT_NAME}${exeExt}
                   ${CMAKE_INSTALL_PREFIX}/bin/aocl-ioc64${exeExt} COPYONLY)
    " COMPONENT ioc64)
