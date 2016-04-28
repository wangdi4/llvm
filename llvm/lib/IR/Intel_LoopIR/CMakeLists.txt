add_llvm_library(LLVMIntel_LoopIR
  BlobDDRef.cpp
  CanonExpr.cpp
  DDRef.cpp
  HLDDNode.cpp  
  HLGoto.cpp  
  HLIf.cpp  
  HLInst.cpp  
  HLLabel.cpp  
  HLLoop.cpp
  HLNode.cpp
  HLRegion.cpp
  HLSwitch.cpp
  IRRegion.cpp
  RegDDRef.cpp
  HIRVerifier.cpp
  Diag.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/IR
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/IR/Intel_LoopIR
  )

add_dependencies(LLVMIntel_LoopIR intrinsics_gen intel_directives_gen)
