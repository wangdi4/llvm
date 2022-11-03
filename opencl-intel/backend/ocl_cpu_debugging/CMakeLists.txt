# Copyright (C) 2011-2022 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this software
# or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express or
# implied warranties, other than those that are expressly stated in the License.

if(WIN32)

  use_rtti(TRUE)

  # This change was added in order to support the 15.33 GFX driver CPU runtime
  # (version 3.0) installation with the new SDK
  # TODO - undo this when OclCpuDebugging.dll is added to the driver
  # OclCpuDebugging.dll won't be a part of the driver, "This increases footprint
  # of the driver and don't bring value to iGFX driver product. This library
  # should be distributed as part of SDK which is targeted for developers not
  # final users."

  set(TARGET_NAME "${DEBUGGING_DLL_NAME}${DEBUGGING_DLL_VERSION}")

  project(${TARGET_NAME})

  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bit
    set(PLATFORM 32)
  else(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bit
    set(PLATFORM 64)
  endif(CMAKE_SIZEOF_VOID_P EQUAL 4)

  set(OCL_CPU_DEBUGGING_DIR ${CMAKE_CURRENT_SOURCE_DIR})

  set(OCL_CPU_DEBUGGING_IMPORT_DIRS
      ${OCL_SOURCE_DIR}/externals/protobuf/include
      ${OCL_SOURCE_DIR}/utils/cl_hw_utils/export
      ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export)

  set(OCL_CPU_DEBUGGING_EXPORT_DIR ${OCL_CPU_DEBUGGING_DIR}/export)

  # Create target project

  # Add additional lib direcories
  link_directories(${TBB_LIB_DIR})

  set(SOCKETLIB ws2_32)

  # Create DLL
  add_opencl_library(
    ${TARGET_NAME}
    SHARED
    dllmain.cpp
    debug_communicator.cpp
    debug_server.cpp
    debuginfo_utils.cpp
    debugservermessages.pb.cc
    protobufpackedmessage.cpp
    INCLUDE_DIRS
    ${LLVM_INCLUDE_DIRS}
    ${CL_API_HEADERS}
    ${OCL_CPU_DEBUGGING_IMPORT_DIRS}
    ${OCL_SOURCE_DIR}/externals/tbb/include
    ${OCL_SOURCE_DIR}/externals/itt/include
    LINK_LIBS
    ${SOCKETLIB}
    cl_sys_utils
    protobuf
    LLVMCore
    RC_TEMPLATE
    ${CMAKE_CURRENT_SOURCE_DIR}/OclCpuDebugging.rc.in)

  set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME})

endif(WIN32)

# Install the (GPL'd) GDB OCL plugin. The name and location is important, as GDB
# will auto-load it if it has the same name as a loaded shared object.
copy_to(libintelocl.so-gdb.py DESTINATION lib)
install_to(libintelocl.so-gdb.py DESTINATION lib/${OUTPUT_ARCH_SUFF} COMPONENT
           ocl-gdb-plugin)
