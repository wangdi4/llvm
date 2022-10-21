# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2021 Intel Corporation
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
add_llvm_component_library(LLVMIntel_LoopTransforms
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
        HIRLoopBlocking.cpp
        HIRGenerateMKLCall.cpp
        HIRLoopDistribution.cpp
        HIRLoopDistributionMemRec.cpp
        HIRLoopDistributionLoopNest.cpp
        HIRLoopDistributionGraph.cpp
        HIRLoopDistributionPreProcGraph.cpp
        HIRLoopRematerialize.cpp
        HIRLoopReroll.cpp
        HIROptReportEmitter.cpp
        HIRLoopReversal.cpp
        HIRLMM.cpp
        HIRLoopCollapse.cpp
        HIRPMSymbolicTripCountCompleteUnroll.cpp
        HIRScalarReplArray.cpp
        ParVecDirectiveInsertion.cpp
        HIRParDirInsert.cpp
        HIRVecDirInsert.cpp
        HIROptVarPredicate.cpp
        HIRIdiomRecognition.cpp
        HIRMVForConstUB.cpp
        HIRMVForVariableStride.cpp
        HIRLoopConcatenation.cpp
        HIRArrayTranspose.cpp
        HIRAosToSoa.cpp
        Utils/HIRTransformUtils.cpp
        Utils/HIRArrayContractionUtils.cpp
        HIRLoopFusion.cpp
        HIRLoopFusionGraph.cpp
        HIRDeadStoreElimination.cpp
        HIRLastValueComputation.cpp
        HIRPropagateCastedIV.cpp
        HIRMultiExitLoopReroll.cpp
        HIRRecognizeParLoop.cpp
        HIRMinMaxRecognition.cpp
        HIRIdentityMatrixIdiomRecognition.cpp
        HIRPrefetching.cpp
        HIRSinkingForPerfectLoopnest.cpp
        HIRUndoSinkingForPerfectLoopnest.cpp
        HIRConditionalTempSinking.cpp
        HIRMemoryReductionSinking.cpp
        HIRRowWiseMV.cpp
        HIRPrintDiag.cpp
        HIRConditionalLoadStoreMotion.cpp
        HIRNontemporalMarking.cpp
        HIRStencilPattern.cpp
        HIRStoreResultIntoTempArray.cpp
        HIRSumWindowReuse.cpp
        HIRNonZeroSinkingForPerfectLoopnest.cpp
        HIRIdentityMatrixSubstitution.cpp
        HIRArrayScalarizationTestLauncher.cpp
        HIRArrayScalarization.cpp
        HIRIfReversal.cpp
        HIRTempArrayTranspose.cpp
        HIRMinMaxBlobToSelectPass.cpp
        HIRLoopPeeling.cpp
        HIRLowerSmallMemsetMemcpy.cpp
# INTEL_FEATURE_SW_ADVANCED
        HIRInterLoopBlocking.cpp
        HIRCrossLoopArrayContraction.cpp
# end INTEL_FEATURE_SW_ADVANCED

        ADDITIONAL_HEADER_DIRS
        ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
        ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Intel_LoopTransforms

        DEPENDS
        intrinsics_gen
        directives_gen

        LINK_LIBS

        LINK_COMPONENTS
        Analysis
        Core
        Intel_LoopAnalysis
        Support
        TransformUtils
        )

# INTEL_CUSTOMIZATION
# INTEL_FEATURE_SW_DTRANS
is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_SW_DTRANS)
if (p)
  target_link_libraries(LLVMIntel_LoopTransforms PRIVATE LLVMIntel_DTransAnalysis)
endif()
# end INTEL_FEATURE_SW_DTRANS
# end INTEL_CUSTOMIZATION
