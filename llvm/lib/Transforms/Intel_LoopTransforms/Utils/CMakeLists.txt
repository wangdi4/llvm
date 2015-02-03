add_llvm_library(LLVMIntel_LoopUtils
  CanonExprUtils.cpp
  DDRefUtils.cpp
  HLNodeUtils.cpp
  HLNodeVisitor.cpp
  )
add_dependencies(LLVMIntel_LoopUtils LLVMIntel_LoopIR)
