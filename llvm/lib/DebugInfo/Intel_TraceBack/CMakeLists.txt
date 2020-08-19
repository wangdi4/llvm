add_llvm_component_library(LLVMDebugInfoTraceBack
  TraceDINode.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/DebugInfo/TraceBack
  )
