set(LLVM_LINK_COMPONENTS
  Analysis
  Core
  Vectorize
  AsmParser
  CodeGen
  MC
  X86CodeGen
  X86Info
  X86Desc
  Target
  )

add_llvm_unittest(VectorizeTests
# INTEL_CUSTOMIZATION
  IntelVPlanAlignmentAnalysisTest.cpp
  IntelVPlanCGTest.cpp
  IntelVPlanCheckNodeParent.cpp
  IntelVPlanClonePlanTest.cpp
  IntelVPlanDATest.cpp
  IntelVPlanTTIMemrefCostTest.cpp
  IntelVPlanValueTrackingTest.cpp
  IntelVPlanVPSubscriptTest.cpp
  IntelVPlanCheckPsadbwHeuristic.cpp
# end INTEL_CUSTOMIZATION
  VPlanDominatorTreeTest.cpp
  VPlanLoopInfoTest.cpp
  VPlanPredicatorTest.cpp
  VPlanTest.cpp
  VPlanHCFGTest.cpp
  VPlanSlpTest.cpp
  )
