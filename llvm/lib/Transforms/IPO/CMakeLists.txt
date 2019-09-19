add_llvm_library(LLVMipo
  AlwaysInliner.cpp
  ArgumentPromotion.cpp
  Attributor.cpp
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
  IPConstantPropagation.cpp
  IPO.cpp
  InferFunctionAttrs.cpp
  Intel_AdvancedFastCall.cpp   # INTEL
  Intel_DopeVectorConstProp.cpp   # INTEL
  Intel_InlineReport.cpp   # INTEL
  Intel_MDInlineReport.cpp   # INTEL
  Intel_InlineReportSetup.cpp # INTEL
  Intel_InlineReportCommon.cpp   # INTEL
  Intel_InlineReportEmitter.cpp # INTEL
  InlineSimple.cpp
  Inliner.cpp
  Intel_CallTreeCloning.cpp # INTEL
  Intel_IPCloning.cpp     # INTEL
  Intel_IPOPrefetch.cpp   # INTEL
  Utils/Intel_IPOUtils.cpp # INTEL
  Intel_InlineLists.cpp   # INTEL
  Intel_OptimizeDynamicCasts.cpp    # INTEL
  Intel_PartialInline.cpp           # INTEL
  Internalize.cpp
  LoopExtractor.cpp
  LowerTypeTests.cpp
  MergeFunctions.cpp
  PartialInlining.cpp
  PassManagerBuilder.cpp
  PruneEH.cpp
  SampleProfile.cpp
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
