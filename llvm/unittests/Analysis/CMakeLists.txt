set(LLVM_LINK_COMPONENTS
  Analysis
  AsmParser
  Core
  Support
  TransformUtils
  )

if (DEFINED LLVM_HAVE_TF_API)
  LIST(APPEND EXTRA_TESTS TFUtilsTest.cpp)
else()
  LIST(APPEND LLVM_OPTIONAL_SOURCES TFUtilsTest.cpp)
endif()

add_llvm_unittest_with_input_files(AnalysisTests
  AliasAnalysisTest.cpp
  AliasSetTrackerTest.cpp
  AssumeBundleQueriesTest.cpp
  BasicAliasAnalysisTest.cpp
  BlockFrequencyInfoTest.cpp
  BranchProbabilityInfoTest.cpp
  CallGraphTest.cpp
  CaptureTrackingTest.cpp
  CFGTest.cpp
  CGSCCPassManagerTest.cpp
  DDGTest.cpp
  DivergenceAnalysisTest.cpp
  DomTreeUpdaterTest.cpp
  GlobalsModRefTest.cpp
  InlineFeaturesAnalysisTest.cpp
<<<<<<< HEAD
# INTEL_CUSTOMIZATION
  Intel_VectorVariantTest.cpp
# end INTEL_CUSTOMIZATION
  InlineSizeEstimatorAnalysisTest.cpp
=======
>>>>>>> 9870f77441c58d4a2dfefe69c6c103b0a03d940a
  IVDescriptorsTest.cpp
  LazyCallGraphTest.cpp
  LoadsTest.cpp
  LoopInfoTest.cpp
  LoopNestTest.cpp
  MemoryBuiltinsTest.cpp
  MemorySSATest.cpp
  PhiValuesTest.cpp
  ProfileSummaryInfoTest.cpp
  ScalarEvolutionTest.cpp
  VectorFunctionABITest.cpp
  SparsePropagation.cpp
  TargetLibraryInfoTest.cpp
  TBAATest.cpp
  UnrollAnalyzerTest.cpp
  ValueLatticeTest.cpp
  ValueTrackingTest.cpp
  VectorUtilsTest.cpp
  ${EXTRA_TESTS}
  )

 target_link_libraries(AnalysisTests PRIVATE LLVMTestingSupport)
