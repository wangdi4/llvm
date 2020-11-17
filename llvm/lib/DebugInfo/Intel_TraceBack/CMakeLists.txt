add_llvm_component_library(LLVMDebugInfoTraceBack
  TraceDINode.cpp
  TraceContext.cpp
  TraceByteParser.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/DebugInfo/Intel_TraceBack

  LINK_COMPONENTS
  BinaryFormat
  MC
  Object
  Support
  )
