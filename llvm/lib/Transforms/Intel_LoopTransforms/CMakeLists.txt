add_llvm_library(LLVMIntel_LoopTransforms 
        Intel_LoopTransforms.cpp
        SSADeconstruction.cpp
        HIRPrinter.cpp
        HIRCodeGen.cpp
        HIROptPredicate.cpp
        HIRCompleteUnroll.cpp
        HIRGeneralUnroll.cpp
        HIRDummyTransformation.cpp
        HIRLoopInterchange.cpp
        HIRRuntimeDD.cpp
        HIRLoopDistribution.cpp
        ParVecDirectiveInsertion.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Intel_LoopTransforms
        )

add_dependencies(LLVMIntel_LoopTransforms intrinsics_gen)
add_subdirectory(Utils)
