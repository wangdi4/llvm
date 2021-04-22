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
  Intel_DeadArrayOpsElimination.cpp   # INTEL
  Intel_DevirtMultiversioning.cpp # INTEL
  Intel_DopeVectorConstProp.cpp   # INTEL
  Intel_QsortRecognizer.cpp # INTEL
  InlineSimple.cpp
  Inliner.cpp
  Intel_ArgumentAlignment.cpp      # INTEL
  Intel_CallTreeCloning.cpp # INTEL
  Intel_FoldWPIntrinsic.cpp         # INTEL
  Intel_IPArrayTranspose.cpp     # INTEL
  Intel_IPCloning.cpp     # INTEL
  Intel_IPOPrefetch.cpp   # INTEL
  Utils/Intel_IPOUtils.cpp # INTEL
  Intel_InlineLists.cpp   # INTEL
  Intel_OptimizeDynamicCasts.cpp    # INTEL
  Intel_PartialInline.cpp           # INTEL
  Intel_TileMVInlMarker.cpp         # INTEL
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
  )

# INTEL_CUSTOMIZATION
if(INTEL_INCLUDE_DTRANS)
  target_link_libraries(LLVMipo PRIVATE LLVMIntel_DTrans LLVMIntel_DTransAnalysis)
endif(INTEL_INCLUDE_DTRANS)
# INTEL_FEATURE_CSA
is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_CSA)
if (p)
  target_link_libraries(LLVMipo PRIVATE LLVMIntel_CSAScalarOpt)
endif()
# end INTEL_FEATURE_CSA
# end INTEL_CUSTOMIZATION
