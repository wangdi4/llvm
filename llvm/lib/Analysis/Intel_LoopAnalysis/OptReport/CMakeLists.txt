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
    OptReportSupport.cpp
    )

  # Create protobuf .h and .cc files, and put them in a library for use by OptReportSupport component.
  set(Protobuf_USE_STATIC_LIBS ON)
  set(protobuf_MODULE_COMPATIBLE ON CACHE BOOL "")
  set(PROTOBUF_USE_LIBCXX FALSE)
  if (LLVM_ENABLE_LIBCXX)
    if(LLVM_COMPILER_IS_GCC_COMPATIBLE)
      check_cxx_compiler_flag("-stdlib=libc++" CXX_COMPILER_SUPPORTS_STDLIB)
      llvm_check_linker_flag(CXX "-stdlib=libc++" CXX_LINKER_SUPPORTS_STDLIB)
      if(CXX_COMPILER_SUPPORTS_STDLIB AND CXX_LINKER_SUPPORTS_STDLIB)
        set(PROTOBUF_USE_LIBCXX TRUE)
      endif()
    endif()
  endif()
  if (PROTOBUF_USE_LIBCXX)
    set(PROTOBUF_ROOT /rdrive/pkg/protobuf/protobuf-3.17.3-libcxx)
  else()
    set(PROTOBUF_ROOT /rdrive/pkg/protobuf/protobuf-master-20210712)
  endif()
  find_package(Protobuf REQUIRED CONFIG HINTS ${PROTOBUF_ROOT})
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
    protobuf::libprotobuf
    )

endif()

add_llvm_component_library(LLVMIntel_OptReportSupport
  OptReportSupport.cpp

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  Core
  Support
  Intel_OptReport
  )

if(INTEL_ENABLE_PROTO_BIN_OPTRPT)
  target_link_libraries(LLVMIntel_OptReportSupport
    PRIVATE
    protobuf::libprotobuf
    LLVMIntel_OptReportProtoWriter
    )
endif()
