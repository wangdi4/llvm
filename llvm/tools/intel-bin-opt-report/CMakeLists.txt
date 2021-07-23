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

  # TODO: There is potentially a way to share Protobuf generated sources across multiple components instead of rebuilding here.

  # Create protobuf .h and .cc files, and put them in a library for use by LoopOptReportSupport component.
  find_package(Protobuf REQUIRED)
  add_definitions(-DGOOGLE_PROTOBUF_NO_RTTI)
  include_directories(${PROTOBUF_INCLUDE_DIRS})
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  set(OPT_REPORT_PROTO ${PROJECT_SOURCE_DIR}/include/llvm/Analysis/Intel_LoopAnalysis/OptReport/opt_report_proto.proto)
  protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${OPT_REPORT_PROTO})
  set(LLVM_OPTIONAL_SOURCES ${LLVM_OPTIONAL_SOURCES} ${PROTO_SRCS})
  message( STATUS "INTEL: Protobuf lib found : ${PROTOBUF_LIBRARIES}, binary opt-report C++ library built for reader tool." )
  add_llvm_library(LLVMIntel_OptReportProtoReader
    ${PROTO_SRCS}
    ${PROTO_HDRS}

    LINK_LIBS
    ${PROTOBUF_LIBRARIES}
    )

  add_llvm_tool(intel-bin-opt-report
    BinOptReport.cpp
    )

  target_link_libraries(intel-bin-opt-report
    PRIVATE
    ${PROTOBUF_LIBRARIES}
    LLVMIntel_OptReportProtoReader
    )
endif()
