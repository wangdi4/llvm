add_llvm_library(LLVMipo
  AlwaysInliner.cpp
  ArgumentPromotion.cpp
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
  IPConstantPropagation.cpp
  IPO.cpp
  InferFunctionAttrs.cpp
  InlineReport.cpp   # INTEL
  InlineSimple.cpp
  Inliner.cpp
  Intel_CallTreeCloning.cpp # INTEL
  Intel_IPCloning.cpp     # INTEL
  Intel_InlineLists.cpp   # INTEL
  Intel_OptimizeDynamicCasts.cpp    # INTEL
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
  intel_directives_gen
# end INTEL_COLLAB
  )

# INTEL_CUSTOMIZATION
if(INTEL_INCLUDE_DTRANS)
  target_link_libraries(LLVMipo PRIVATE LLVMIntel_DTrans)
endif(INTEL_INCLUDE_DTRANS)
# end INTEL_CUSTOMIZATION
