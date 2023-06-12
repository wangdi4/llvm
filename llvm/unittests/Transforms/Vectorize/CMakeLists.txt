set(LLVM_LINK_COMPONENTS
  Analysis
  Core
  Vectorize
  AsmParser
  CodeGen
  Intel_LoopAnalysis   # INTEL
  Intel_LoopTransforms # INTEL
  MC
  X86CodeGen
  X86Info
  X86Desc
  Target
  TargetParser
  )

add_llvm_unittest(VectorizeTests
# INTEL_CUSTOMIZATION
  IntelVPAssumptionCacheTest.cpp
  IntelVPOperandBundlesTest.cpp
  IntelVPlanAlignmentAnalysisTest.cpp
  IntelVPlanAllZeroCheckCost.cpp
  IntelVPlanCGTest.cpp
  IntelVPlanCheckNodeParent.cpp
  IntelVPlanClonePlanTest.cpp
  IntelVPlanDATest.cpp
  IntelVPlanLoopInfoTest.cpp
  IntelVPlanTest.cpp
  IntelVPlanTTIMemrefCostTest.cpp
  IntelVPlanValueTrackingTest.cpp
  IntelVPlanVerifierTest.cpp
# INTEL_FEATURE_SW_ADVANCED
  IntelVPlanCheckHeuristics.cpp
# end INTEL_FEATURE_SW_ADVANCED
# end INTEL_CUSTOMIZATION
  VPlanTest.cpp
  VPDomTreeTest.cpp
  VPlanHCFGTest.cpp
  VPlanSlpTest.cpp
  VPlanVerifierTest.cpp
  )

set_property(TARGET VectorizeTests PROPERTY FOLDER "Tests/UnitTests/TransformsTests")
