# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2021 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may not
# use, modify, copy, publish, distribute, disclose or transmit this software or
# the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.
#
# end INTEL_CUSTOMIZATION
add_llvm_component_library(LLVMVectorize
  LoadStoreVectorizer.cpp
  LoopVectorizationLegality.cpp
  LoopVectorize.cpp
  SLPVectorizer.cpp
  Vectorize.cpp
  VectorCombine.cpp
  VPlan.cpp
  VPlanHCFGBuilder.cpp
  VPlanRecipes.cpp
  VPlanSLP.cpp
  VPlanTransforms.cpp
  VPlanVerifier.cpp
# INTEL_CUSTOMIZATION
  Intel_LoadCoalescing.cpp
  IntelMFReplacement.cpp
  Intel_VPlan/IntelLoopVectorizationLegality.cpp
  Intel_VPlan/IntelLoopVectorizationPlanner.cpp
  Intel_VPlan/IntelVPBasicBlock.cpp
  Intel_VPlan/IntelVPLoopAnalysis.cpp
  Intel_VPlan/IntelVPOCodeGen.cpp
  Intel_VPlan/IntelVPlan.cpp
  Intel_VPlan/IntelVPlanAlignmentAnalysis.cpp
  Intel_VPlan/IntelVPlanAllZeroBypass.cpp
  Intel_VPlan/IntelVPlanCFGBuilder.cpp
  Intel_VPlan/IntelVPlanCFGMerger.cpp
  Intel_VPlan/IntelVPlanCallVecDecisions.cpp
  Intel_VPlan/IntelVPlanClone.cpp
  Intel_VPlan/IntelVPlanCostModel.cpp
  Intel_VPlan/IntelVPlanCostModelHeuristics.cpp
  Intel_VPlan/IntelVPlanDivergenceAnalysis.cpp
  Intel_VPlan/IntelVPlanDriver.cpp
  Intel_VPlan/IntelVPlanEvaluator.cpp
  Intel_VPlan/IntelVPlanExternals.cpp
  Intel_VPlan/IntelVPlanFunctionVectorizer.cpp
  Intel_VPlan/IntelVPlanHCFGBuilder.cpp
  Intel_VPlan/IntelVPlanIdioms.cpp
  Intel_VPlan/IntelVPlanLCSSA.cpp
  Intel_VPlan/IntelVPlanLoopCFU.cpp
  Intel_VPlan/IntelVPlanLoopExitCanonicalization.cpp
  Intel_VPlan/IntelVPlanLoopInfo.cpp
  Intel_VPlan/IntelVPlanLoopUnroller.cpp
  Intel_VPlan/IntelVPlanMaskedModeLoop.cpp
  Intel_VPlan/IntelVPMemRefTransform.cpp
  Intel_VPlan/IntelVPlanOptrpt.cpp
  Intel_VPlan/IntelVPlanPragmaOmpOrderedSimdExtract.cpp
  Intel_VPlan/IntelVPlanPragmaOmpSimdIf.cpp
  Intel_VPlan/IntelVPlanPredicator.cpp
  Intel_VPlan/IntelVPlanScalarEvolution.cpp
  Intel_VPlan/IntelVPlanScalVecAnalysis.cpp
  Intel_VPlan/IntelVPlanSSADeconstruction.cpp
  Intel_VPlan/IntelVPlanSyncDependenceAnalysis.cpp
  Intel_VPlan/IntelVPTransformLibraryCalls.cpp
  Intel_VPlan/IntelVPlanValueTracking.cpp
  Intel_VPlan/IntelVPlanVConflictTransformation.cpp
  Intel_VPlan/IntelVPlanVLSAnalysis.cpp
  Intel_VPlan/IntelVPlanVLSClient.cpp
  Intel_VPlan/IntelVPlanVLSTransform.cpp
  Intel_VPlan/IntelVPlanVectorShape.cpp
  Intel_VPlan/IntelVPlanVectorizeIndirectCalls.cpp
  Intel_VPlan/IntelVPlanVerifier.cpp
  Intel_VPlan/IntelVPSOAAnalysis.cpp
  Intel_VPlan/VPlanHIR/IntelLoopVectorizationPlannerHIR.cpp
  Intel_VPlan/VPlanHIR/IntelVPOCodeGenHIR.cpp
  Intel_VPlan/VPlanHIR/IntelVPlanDecomposerHIR.cpp
  Intel_VPlan/VPlanHIR/IntelVPlanHCFGBuilderHIR.cpp
  Intel_VPlan/VPlanHIR/IntelVPlanInstructionDataHIR.cpp
  Intel_VPlan/VPlanHIR/IntelVPlanScalarEvolutionHIR.cpp
  Intel_VPlan/VPlanHIR/IntelVPlanValueTrackingHIR.cpp
  Intel_VPlan/VPlanHIR/IntelVPlanVLSAnalysisHIR.cpp
  Intel_VPlan/VPlanHIR/IntelVPlanVerifierHIR.cpp
# end INTEL_CUSTOMIZATION

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Vectorize

  DEPENDS
  intrinsics_gen
# INTEL_CUSTOMIZATION
  directives_gen
# end INTEL_CUSTOMIZATION

  LINK_COMPONENTS
  Analysis
  Core
  Support
  TransformUtils
# INTEL_CUSTOMIZATION
  Intel_LoopAnalysis
  Intel_LoopTransforms
  VPOAnalysis
  VPOTransforms
# end INTEL_CUSTOMIZATION
  )

# INTEL_CUSTOMIZATION
# INTEL_FEATURE_SW_DTRANS
is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_SW_DTRANS)
if (p)
  target_link_libraries(LLVMVectorize PRIVATE LLVMIntel_DTransOpt)
endif()
# end INTEL_FEATURE_SW_DTRANS

# This is to fix the error (unused private field) with xmain self build about
# the PeelTC data member of class VPlanRemainderEvaluator.
if ((HOST_LINUX OR HOST_WINNT) AND
       "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|IntelLLVM")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-private-field")
endif()

# end INTEL_CUSTOMIZATION
