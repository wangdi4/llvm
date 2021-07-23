#
#    Copyright (C) 2019-2019 Intel Corporation. All rights reserved.
#
#    The information and source code contained herein is the exclusive
#    property of Intel Corporation. and may not be disclosed, examined
#    or reproduced in whole or in part without explicit written authorization
#    from the company.
#
#    Source file: CMakeLists.txt
#    ------------
#    Configures build for project.
#

if(INTEL_ENABLE_PROTO_BIN_OPTRPT)
  # Needed by LLVM's CMake checks since target is linked with an optionally built library.
  set(LLVM_OPTIONAL_SOURCES
    LoopOptReportSupport.cpp
    )

  # Create protobuf .h and .cc files, and put them in a library for use by LoopOptReportSupport component.
  find_package(Protobuf REQUIRED)
  add_definitions(-DGOOGLE_PROTOBUF_NO_RTTI)
  add_definitions(-DINTEL_ENABLE_PROTO_BIN_OPTRPT)
  include_directories(${PROTOBUF_INCLUDE_DIRS})
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  set(OPT_REPORT_PROTO ${PROJECT_SOURCE_DIR}/include/llvm/Analysis/Intel_LoopAnalysis/OptReport/opt_report_proto.proto)
  protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${OPT_REPORT_PROTO})
  set(LLVM_OPTIONAL_SOURCES ${LLVM_OPTIONAL_SOURCES} ${PROTO_SRCS})
  message( STATUS "INTEL: Protobuf lib found : ${PROTOBUF_LIBRARIES}, binary opt-report C++ library built for compiler." )
  add_llvm_library(LLVMIntel_OptReportProtoWriter
    ${PROTO_SRCS}
    ${PROTO_HDRS}

    LINK_LIBS
    ${PROTOBUF_LIBRARIES}
    )

endif()

add_llvm_component_library(LLVMIntel_LoopOptReportSupport
  LoopOptReportSupport.cpp

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  Core
  Support
  Intel_OptReport
  )

if(INTEL_ENABLE_PROTO_BIN_OPTRPT)
  target_link_libraries(LLVMIntel_LoopOptReportSupport
    PRIVATE
    ${PROTOBUF_LIBRARIES}
    LLVMIntel_OptReportProtoWriter
    )
endif()
