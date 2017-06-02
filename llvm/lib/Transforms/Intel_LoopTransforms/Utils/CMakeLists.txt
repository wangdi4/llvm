add_llvm_library(LLVMIntel_LoopUtils
  BlobUtils.cpp
  CanonExprUtils.cpp
  DDRefUtils.cpp
  DDUtils.cpp
  HLNodeUtils.cpp
  HIRTransformUtils.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Intel_LoopTransforms
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Intel_LoopTransforms/Utils
  )

add_dependencies(LLVMIntel_LoopUtils intrinsics_gen)
