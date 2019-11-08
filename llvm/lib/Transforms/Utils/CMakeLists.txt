add_llvm_library(LLVMTransformUtils
  ASanStackFrameLayout.cpp
  AddDiscriminators.cpp
  BasicBlockUtils.cpp
  BreakCriticalEdges.cpp
  BuildLibCalls.cpp
  BypassSlowDivision.cpp
  CallPromotionUtils.cpp
  CanonicalizeAliases.cpp
  CloneFunction.cpp
  CloneModule.cpp
  CodeExtractor.cpp
  CtorUtils.cpp
  Debugify.cpp
  DemoteRegToStack.cpp
  EntryExitInstrumenter.cpp
  EscapeEnumerator.cpp
  Evaluator.cpp
  FlattenCFG.cpp
  FunctionComparator.cpp
  FunctionImportUtils.cpp
  GlobalStatus.cpp
  GuardUtils.cpp
  InlineFunction.cpp
  ImportedFunctionsInliningStatistics.cpp
  InstructionNamer.cpp
  IntegerDivision.cpp
# INTEL_COLLAB
  InferAddressSpacesUtils.cpp
# end INTEL_COLLAB
  Intel_CloneUtils.cpp # INTEL_CUSTOMIZATION
# INTEL_COLLAB
  GeneralUtils.cpp
  IntrinsicUtils.cpp
  ../VPO/Utils/IntrinsicUtils.cpp # TODO: Move to lib/Transforms/Utils
# end INTEL_COLLAB
# INTEL_CUSTOMIZATION
  Intel_RegionSplitter.cpp
  Intel_VecClone.cpp                    #TODO: VEC to COLLAB
#end INTEL_CUSTOMIZATION
  LCSSA.cpp
  LibCallsShrinkWrap.cpp
  Local.cpp
  LoopRotationUtils.cpp
  LoopSimplify.cpp
  LoopUnroll.cpp
  LoopUnrollAndJam.cpp
  LoopUnrollPeel.cpp
  LoopUnrollRuntime.cpp
  LoopUtils.cpp
  LoopVersioning.cpp
  LowerInvoke.cpp
  LowerMemIntrinsics.cpp
  LowerSwitch.cpp
  Mem2Reg.cpp
  MetaRenamer.cpp
  MisExpect.cpp
  ModuleUtils.cpp
  NameAnonGlobals.cpp
  PredicateInfo.cpp
  PromoteMemoryToRegister.cpp
  StripGCRelocates.cpp
  SSAUpdater.cpp
  SSAUpdaterBulk.cpp
  SanitizerStats.cpp
  SimplifyCFG.cpp
  SimplifyIndVar.cpp
  SimplifyLibCalls.cpp
  SizeOpts.cpp
  SplitModule.cpp
  StripNonLineTableDebugInfo.cpp
  SymbolRewriter.cpp
  UnifyFunctionExitNodes.cpp
  Utils.cpp
  ValueMapper.cpp
  VNCoercion.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Utils

  DEPENDS
  intrinsics_gen
# INTEL_COLLAB
  directives_gen
# end INTEL_COLLAB
  )
