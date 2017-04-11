add_llvm_library(LLVMIntel_LoopTransforms 
        Intel_LoopTransforms.cpp
        HIRSSADeconstruction.cpp
        HIRTempCleanup.cpp
        HIRPrinter.cpp
        HIRCodeGen.cpp
        HIROptPredicate.cpp
        HIRPreVecCompleteUnroll.cpp
        HIRPostVecCompleteUnroll.cpp
        HIRCompleteUnroll.cpp
        HIRGeneralUnroll.cpp
        HIRUnrollAndJam.cpp
        HIRDummyTransformation.cpp
        HIRLoopInterchange.cpp
        HIRRuntimeDD.cpp
        HIRLoopDistribution.cpp
        HIRLoopDistributionMemRec.cpp
        HIRLoopDistributionLoopNest.cpp
        HIRLoopReversal.cpp 
        HIRLMM.cpp
        HIRScalarReplArray.cpp
        ParVecDirectiveInsertion.cpp
        HIRParDirInsert.cpp
        HIRVecDirInsert.cpp
        HIROptVarPredicate.cpp
        HIRIdiomRecognition.cpp
        HIRMVForConstUB.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Intel_LoopTransforms
        )

add_dependencies(LLVMIntel_LoopTransforms intrinsics_gen)
add_subdirectory(Utils)
