set(LLVM_LINK_COMPONENTS
  Analysis
  Core
  Vectorize
  AsmParser
  )

add_llvm_unittest(VectorizeTests
# INTEL_CUSTOMIZATION
  IntelVPlanAlignmentAnalysisTest.cpp
  IntelVPlanCheckNodeParent.cpp
  IntelVPlanClonePlanTest.cpp
  IntelVPlanValueTrackingTest.cpp
  IntelVPlanVPSubscriptTest.cpp
# end INTEL_CUSTOMIZATION
  VPlanDominatorTreeTest.cpp
  VPlanLoopInfoTest.cpp
  VPlanPredicatorTest.cpp
  VPlanTest.cpp
  VPlanHCFGTest.cpp
  VPlanSlpTest.cpp
  )
