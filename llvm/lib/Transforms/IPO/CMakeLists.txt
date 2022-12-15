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
# INTEL_CUSTOMIZATION
set(INTEL_SOURCE_FILES_TO_BUILD)
intel_add_file(INTEL_SOURCE_FILES_TO_BUILD
  COMPLEMENT LLVM_OPTIONAL_SOURCES
# Add custom Intel files below.  Files listed here will be included in
# the INTEL_CUSTOMIZATION build.
  Intel_AdvancedFastCall.cpp
  Intel_AggInliner.cpp
  Intel_ArgNoAliasProp.cpp
  Intel_ArgumentAlignment.cpp
  Intel_AutoCPUClone.cpp
  Intel_CallTreeCloning.cpp
  Intel_DopeVectorConstProp.cpp
  Intel_InlineLists.cpp
  Intel_IPArrayTranspose.cpp
  Intel_IPODeadArgElimination.cpp
  Intel_MathLibrariesDeclaration.cpp
  Intel_OptimizeDynamicCasts.cpp
  Intel_VTableFixup.cpp
  Utils/Intel_IPOUtils.cpp
  )

# INTEL_FEATURE_SW_ADVANCED
intel_add_file(INTEL_SOURCE_FILES_TO_BUILD
  COMPLEMENT LLVM_OPTIONAL_SOURCES
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_SW_ADVANCED
  Intel_DeadArrayOpsElimination.cpp
  Intel_QsortRecognizer.cpp
  Intel_IPCloning.cpp
  Intel_IPOPrefetch.cpp
  Intel_IPPredOpt.cpp
  Intel_PartialInline.cpp
  Intel_TileMVInlMarker.cpp
  )
# end INTEL_FEATURE_SW_ADVANCED

# INTEL_FEATURE_SW_DTRANS
intel_add_file(INTEL_SOURCE_FILES_TO_BUILD
  COMPLEMENT LLVM_OPTIONAL_SOURCES
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_SW_DTRANS
  Intel_DevirtMultiversioning.cpp
  Intel_FoldWPIntrinsic.cpp
  )
# end INTEL_FEATURE_SW_DTRANS
# end INTEL_CUSTOMIZATION

add_llvm_component_library(LLVMipo
  AlwaysInliner.cpp
  Annotation2Metadata.cpp
  ArgumentPromotion.cpp
  Attributor.cpp
  AttributorAttributes.cpp
  BarrierNoopPass.cpp
  BlockExtractor.cpp
  CalledValuePropagation.cpp
  ConstantMerge.cpp
  CrossDSOCFI.cpp
  DeadArgumentElimination.cpp
  ElimAvailExtern.cpp
  ExtractGV.cpp
  ForceFunctionAttrs.cpp
  FunctionAttrs.cpp
  FunctionImport.cpp
  FunctionSpecialization.cpp
  GlobalDCE.cpp
  GlobalOpt.cpp
  GlobalSplit.cpp
  HotColdSplitting.cpp
  IPO.cpp
  IROutliner.cpp
  InferFunctionAttrs.cpp
# INTEL_CUSTOMIZATION
  ${INTEL_SOURCE_FILES_TO_BUILD}
#endif // INTEL_CUSTOMIZATION
  InlineSimple.cpp
  Inliner.cpp
  Internalize.cpp
  LoopExtractor.cpp
  LowerTypeTests.cpp
  MergeFunctions.cpp
  ModuleInliner.cpp
  OpenMPOpt.cpp
  PartialInlining.cpp
  PassManagerBuilder.cpp
  SampleContextTracker.cpp
  SampleProfile.cpp
  SampleProfileProbe.cpp
  SCCP.cpp
  StripDeadPrototypes.cpp
  StripSymbols.cpp
  SyntheticCountsPropagation.cpp
  ThinLTOBitcodeWriter.cpp
  WholeProgramDevirt.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/IPO

  DEPENDS
  intrinsics_gen

# INTEL_COLLAB
  directives_gen
# end INTEL_COLLAB
  omp_gen

  COMPONENT_NAME
  IPO

  LINK_COMPONENTS
  AggressiveInstCombine
  Analysis
  BitReader
  BitWriter
  Core
  FrontendOpenMP
  InstCombine
  IRReader
  Linker
  Object
  ProfileData
  Scalar
  Support
  TransformUtils
  Vectorize
  Instrumentation
# INTEL_CUSTOMIZATION
  Intel_LoopTransforms
  Intel_MapIntrinToIml
  VPOTransforms
  Intel_DPCPPKernelTransforms
# end INTEL_CUSTOMIZATION
  Scalar
  )

# INTEL_CUSTOMIZATION
# INTEL_FEATURE_SW_DTRANS
is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_SW_DTRANS)
if (p)
  target_link_libraries(LLVMipo PRIVATE LLVMIntel_DTrans LLVMIntel_DTransOpt LLVMIntel_DTransAnalysis)
endif()
# end INTEL_FEATURE_SW_DTRANS
# INTEL_FEATURE_CSA
is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_CSA)
if (p)
  target_link_libraries(LLVMipo PRIVATE LLVMIntel_CSAScalarOpt)
endif()
# end INTEL_FEATURE_CSA
# end INTEL_CUSTOMIZATION
