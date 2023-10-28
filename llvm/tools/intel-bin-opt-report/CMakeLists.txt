# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2022 Intel Corporation
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
set(LLVM_LINK_COMPONENTS
  Core
  Object
  Support
  )

# Tool will be built only if Protobuf-based binary opt-report feature is available.
if(INTEL_ENABLE_PROTO_BIN_OPTRPT)
  # Needed by LLVM's CMake checks since target is linked with an optionally built library.
  set(LLVM_OPTIONAL_SOURCES
    BinOptReport.cpp
    )

  include(Intel_BuildOptReportProto)

  add_definitions(-DGOOGLE_PROTOBUF_NO_RTTI)

  set(LLVM_OPTIONAL_SOURCES ${LLVM_OPTIONAL_SOURCES} ${OPT_REPORT_PROTO_SRCS})

  add_llvm_library(LLVMIntel_OptReportProtoReader
    ${OPT_REPORT_PROTO_SRCS}
    ${OPT_REPORT_PROTO_HDRS}

    LINK_LIBS
    protobuf::libprotobuf
    )

  target_include_directories(LLVMIntel_OptReportProtoReader
      SYSTEM PRIVATE $<TARGET_PROPERTY:protobuf::libprotobuf,INCLUDE_DIRECTORIES>)
  target_include_directories(LLVMIntel_OptReportProtoReader PRIVATE ${OPT_REPORT_PROTO_DIR})

  add_llvm_tool(intel-bin-opt-report
    BinOptReport.cpp
    )

  target_include_directories(intel-bin-opt-report
      SYSTEM PRIVATE $<TARGET_PROPERTY:protobuf::libprotobuf,INCLUDE_DIRECTORIES>)
  target_include_directories(intel-bin-opt-report PRIVATE ${OPT_REPORT_PROTO_DIR})

  target_link_libraries(intel-bin-opt-report
    PRIVATE
    protobuf::libprotobuf
    LLVMIntel_OptReportProtoReader
    )
endif()
