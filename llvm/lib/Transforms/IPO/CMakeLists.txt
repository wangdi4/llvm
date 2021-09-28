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
  Intel_AdvancedFastCall.cpp   # INTEL
  Intel_AggInliner.cpp # INTEL
  Intel_ArgNoAliasProp.cpp # INTEL
# INTEL_CUSTOMIZATION
# INTEL_FEATURE_SW_ADVANCED
  Intel_DeadArrayOpsElimination.cpp
#end INTEL_FEATURE_SW_ADVANCED
  Intel_DevirtMultiversioning.cpp
  Intel_DopeVectorConstProp.cpp
#if INTEL_FEATURE_SW_ADVANCED
  Intel_QsortRecognizer.cpp
#endif // INTEL_FEATURE_SW_ADVANCED
#endif // INTEL_CUSTOMIZATION
  InlineSimple.cpp
  Inliner.cpp
  Intel_ArgumentAlignment.cpp      # INTEL
  Intel_CallTreeCloning.cpp # INTEL
  Intel_IPODeadArgElimination.cpp #INTEL
  Intel_FoldWPIntrinsic.cpp         # INTEL
  Intel_IPArrayTranspose.cpp     # INTEL
# INTEL_CUSTOMIZATION
# INTEL_FEATURE_SW_ADVANCED
  Intel_IPCloning.cpp
  Intel_IPOPrefetch.cpp
  Intel_IPPredOpt.cpp
#end INTEL_FEATURE_SW_ADVANCED
#end INTEL_CUSTOMIZATION
  Utils/Intel_IPOUtils.cpp # INTEL
  Intel_InlineLists.cpp   # INTEL
  Intel_MathLibrariesDeclaration.cpp #INTEL
  Intel_OptimizeDynamicCasts.cpp    # INTEL
# INTEL_FEATURE_SW_ADVANCED
  Intel_PartialInline.cpp           # INTEL
# end INTEL_FEATURE_SW_ADVANCED
  Intel_TileMVInlMarker.cpp         # INTEL
  Intel_VTableFixup.cpp             # INTEL
  Internalize.cpp
  LoopExtractor.cpp
  LowerTypeTests.cpp
  MergeFunctions.cpp
  OpenMPOpt.cpp
  PartialInlining.cpp
  PassManagerBuilder.cpp
  PruneEH.cpp
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
  Intel_OptReport
  Intel_DPCPPKernelTransforms
# end INTEL_CUSTOMIZATION
  Scalar
  )

# INTEL_CUSTOMIZATION
# INTEL_FEATURE_SW_DTRANS
is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_SW_DTRANS)
if (p)
  target_link_libraries(LLVMipo PRIVATE LLVMIntel_DTrans LLVMIntel_DTransAnalysis)
endif()
# end INTEL_FEATURE_SW_DTRANS
# INTEL_FEATURE_CSA
is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_CSA)
if (p)
  target_link_libraries(LLVMipo PRIVATE LLVMIntel_CSAScalarOpt)
endif()
# end INTEL_FEATURE_CSA
# end INTEL_CUSTOMIZATION
