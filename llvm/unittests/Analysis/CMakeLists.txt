set(LLVM_LINK_COMPONENTS
  Analysis
  AsmParser
  Core
  Support
  TransformUtils
  )

add_llvm_unittest(AnalysisTests
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
<<<<<<< HEAD
# INTEL_CUSTOMIZATION
  Intel_VectorVariantTest.cpp
# end INTEL_CUSTOMIZATION
=======
  InlineFeaturesAnalysisTest.cpp
>>>>>>> e2cc854015fe962654f439561c84098fb9aadbb2
  IVDescriptorsTest.cpp
  LazyCallGraphTest.cpp
  LoadsTest.cpp
  LoopInfoTest.cpp
  LoopNestTest.cpp
  MemoryBuiltinsTest.cpp
  MemorySSATest.cpp
  OrderedInstructionsTest.cpp
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
<<<<<<< HEAD
  )

add_subdirectory(ML)
=======
  )
>>>>>>> e2cc854015fe962654f439561c84098fb9aadbb2
